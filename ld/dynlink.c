#include <elf.h>
#include <internal/pal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "dynm.h"

#define LD_VERSION  "0.1.1"

static Module g_modules[MAX_MODULES];
static int g_num_modules = 0;
static MainTargetInfo g_target_info;
static uintptr_t g_ld_base = 0;

__attribute__((visibility("hidden")))
size_t internal_strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

static int streq(const char *a, const char *b) {
    if (a == b) return 1;
    if (!a || !b) return 0;
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *a == *b;
}

static int streq_icase(const char *a, const char *b) {
    if (a == b) return 1;
    if (!a || !b) return 0;

    while (*a) {
        char ca = (*a >= 'A' && *a <= 'Z') ? (*a + 32) : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? (*b + 32) : *b;

        if (ca != cb) return 0;
        a++;
        b++;
    }
    return *b == '\0';
}

static size_t get_symcount_from_gnu_hash(const uint32_t *gnu_hash) {
    uint32_t nbuckets = gnu_hash[0];
    uint32_t symndx = gnu_hash[1];
    uint32_t maskwords = gnu_hash[2];

    const uint64_t *bloom = (const uint64_t *)&gnu_hash[4];
    const uint32_t *buckets = (const uint32_t *)&bloom[maskwords];
    const uint32_t *chains = &buckets[nbuckets];

    uint32_t max_sym = 0;
    for (uint32_t i = 0; i < nbuckets; i++) {
        if (buckets[i] > max_sym) {
            max_sym = buckets[i];
        }
    }

    if (max_sym < symndx) return symndx;

    const uint32_t *chain = &chains[max_sym - symndx];
    while (1) {
        max_sym++;
        if (*chain & 1) break;
        chain++;
    }
    return max_sym;
}

static void print_help(void) {
    static const char *msg = 
        "Usage: ld.so [OPTION]... EXECUTABLE-FILE [ARGS...]\n"
        "Dynamic ELF loader and linker for AIC.\n\n"
        "Options:\n"
        "  --help, -h     Display this help message and exit\n"
        "  --version, -v  Output version information\n\n";
    pal_write(1, msg, internal_strlen(msg));
}

static void print_version(void) {
    static const char *msg = "aic-ld " LD_VERSION "\n";
    pal_write(1, msg, internal_strlen(msg));
}

void self_relocate(uintptr_t *sp) {
    int argc = (int)sp[0];
    char **argv = (char **)&sp[1];
    char **envp = &argv[argc + 1];

    while (*envp) envp++;
    Elf64_auxv_t *auxv = (Elf64_auxv_t *)(envp + 1);

    Elf64_Addr at_phdr = 0, at_base = 0;
    Elf64_Half phent = 0, phnum = 0;

    for (; auxv->a_type != AT_NULL; auxv++) {
        switch (auxv->a_type) {
            case AT_PHDR:  at_phdr = auxv->a_un.a_val; break;
            case AT_PHENT: phent = auxv->a_un.a_val; break;
            case AT_PHNUM: phnum = auxv->a_un.a_val; break;
            case AT_BASE:  at_base = auxv->a_un.a_val; break;
        }
    }

    Elf64_Addr load_bias = 0;
    Elf64_Phdr *ph = 0;

    if (at_base) {
        load_bias = at_base;
        Elf64_Ehdr *ehdr = (Elf64_Ehdr *)at_base;
        ph = (Elf64_Phdr *)(at_base + ehdr->e_phoff);
        phent = ehdr->e_phentsize;
        phnum = ehdr->e_phnum;
    } else {
        if (!at_phdr || !phent || !phnum) return;
        ph = (Elf64_Phdr *)at_phdr;

        int found_bias = 0;
        for (int i = 0; i < phnum; i++) {
            Elf64_Phdr *p = (Elf64_Phdr *)((uintptr_t)ph + (i * phent));
            if (p->p_type == PT_PHDR) {
                load_bias = at_phdr - p->p_vaddr;
                found_bias = 1;
                break;
            }
        }

        if (!found_bias) {
            for (int i = 0; i < phnum; i++) {
                Elf64_Phdr *p = (Elf64_Phdr *)((uintptr_t)ph + (i * phent));
                if (p->p_type == PT_LOAD && p->p_offset == 0) {
                    if (at_phdr >= 0x40 && *(uint32_t *)(at_phdr - 0x40) == 0x464c457f) {
                        Elf64_Ehdr *ehdr = (Elf64_Ehdr *)(at_phdr - 0x40);
                        load_bias = (at_phdr - ehdr->e_phoff) - p->p_vaddr;
                    } else {
                        load_bias = (at_phdr - 0x40) - p->p_vaddr;
                    }
                    break;
                }
            }
        }
    }

    g_ld_base = load_bias;

    Elf64_Dyn *dyn = 0;
    for (int i = 0; i < phnum; i++) {
        Elf64_Phdr *p = (Elf64_Phdr *)((uintptr_t)ph + (i * phent));
        if (p->p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn *)(load_bias + p->p_vaddr);
            break;
        }
    }

    if (!dyn) return;

    Elf64_Rela *rela = 0;
    Elf64_Xword relasz = 0;
    Elf64_Xword relaent = sizeof(Elf64_Rela);

    for (; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
            case DT_RELA:    rela = (Elf64_Rela *)(load_bias + dyn->d_un.d_ptr); break;
            case DT_RELASZ:  relasz = dyn->d_un.d_val; break;
            case DT_RELAENT: relaent = dyn->d_un.d_val; break;
        }
    }

    if (!rela || !relaent) return;

    Elf64_Xword count = relasz / relaent;
    for (Elf64_Xword i = 0; i < count; i++) {
        if (ELF64_R_TYPE(rela[i].r_info) == R_X86_64_RELATIVE) {
            Elf64_Addr *patch_addr = (Elf64_Addr *)(load_bias + rela[i].r_offset);
            *patch_addr = load_bias + rela[i].r_addend;
        }
    }
}

static int open_so_file(const char *name) {
    if (name[0] == '/' || (name[0] == '.' && name[1] == '/')) {
        return pal_open(name, 0, 0);
    }

    static const char *search_paths[] = {
        "./lib/",
        "./",
        "/lib/x86_64-linux-gnu/",
        "/usr/lib/x86_64-linux-gnu/",
        "/lib64/",
        "/usr/lib64/",
        "/lib/",
        "/usr/lib/",
        NULL
    };

    char fullpath[512];
    for (int i = 0; search_paths[i]; i++) {
        size_t plen = internal_strlen(search_paths[i]);
        size_t nlen = internal_strlen(name);
        if (plen + nlen >= sizeof(fullpath)) continue;

        internal_memmove(fullpath, search_paths[i], plen);
        internal_memmove(fullpath + plen, name, nlen + 1);

        int fd = pal_open(fullpath, 0, 0);
        if (fd >= 0) return fd;
    }
    return -1;
}

static uintptr_t load_module(const char *filename, int is_main) {
    for (int i = 0; i < g_num_modules; i++) {
        if (streq(g_modules[i].name, filename)) {
            return g_modules[i].base;
        }
    }

    if (g_num_modules >= MAX_MODULES) return 0;

    int fd = open_so_file(filename);
    if (fd < 0) return 0;

    Elf64_Ehdr ehdr;
    if (pal_read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
        pal_close(fd);
        return 0;
    }

    if (ehdr.e_ident[0] != 0x7f || ehdr.e_ident[1] != 'E' ||
        ehdr.e_ident[2] != 'L'  || ehdr.e_ident[3] != 'F') {
        pal_close(fd);
        return 0;
    }

    Elf64_Phdr phdr[32];
    pal_lseek(fd, ehdr.e_phoff, 0);
    pal_read(fd, phdr, ehdr.e_phentsize * ehdr.e_phnum);

    uintptr_t min_vaddr = (uintptr_t)-1;
    uintptr_t max_vaddr = 0;

    for (int i = 0; i < ehdr.e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            if (phdr[i].p_vaddr < min_vaddr) min_vaddr = phdr[i].p_vaddr;
            if (phdr[i].p_vaddr + phdr[i].p_memsz > max_vaddr) {
                max_vaddr = phdr[i].p_vaddr + phdr[i].p_memsz;
            }
        }
    }

    uintptr_t page_min = min_vaddr & ~0xFFFU;
    uintptr_t page_max = (max_vaddr + 0xFFFU) & ~0xFFFU;
    size_t total_size = page_max - page_min;

    void *map_addr = NULL;
    uintptr_t base = 0;

    if (ehdr.e_type == ET_DYN) {
        map_addr = pal_mmap(0, total_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if ((intptr_t)map_addr < 0 || !map_addr) {
            pal_close(fd);
            return 0;
        }
        base = (uintptr_t)map_addr - page_min;
    } else {
        base = 0;
        map_addr = pal_mmap((void *)page_min, total_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
	if (map_addr == (void *)-1 || map_addr != (void *)page_min) {
	    pal_close(fd);
	    return 0;
	}
    }

    Module *mod = &g_modules[g_num_modules++];
    internal_memset(mod, 0, sizeof(Module));

    size_t fn_len = internal_strlen(filename);
    if (fn_len >= sizeof(mod->name)) fn_len = sizeof(mod->name) - 1;
    internal_memmove(mod->name, filename, fn_len);
    mod->name[fn_len] = '\0';
    mod->base = base;

    uintptr_t phdr_addr = 0;
    for (int i = 0; i < ehdr.e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            uintptr_t seg_dst = base + phdr[i].p_vaddr;
            pal_lseek(fd, phdr[i].p_offset, 0);
            pal_read(fd, (void *)seg_dst, phdr[i].p_filesz);
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                internal_memset((void *)(seg_dst + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
            }
        } else if (phdr[i].p_type == PT_DYNAMIC) {
            mod->dynamic = (Elf64_Dyn *)(base + phdr[i].p_vaddr);
        } else if (phdr[i].p_type == PT_PHDR) {
            phdr_addr = base + phdr[i].p_vaddr;
        }
    }
    pal_close(fd);

    if (is_main) {
        g_target_info.entry = base + ehdr.e_entry;
        g_target_info.phent = ehdr.e_phentsize;
        g_target_info.phnum = ehdr.e_phnum;
        g_target_info.base = base;
        g_target_info.phdr = phdr_addr ? phdr_addr : (base + ehdr.e_phoff);
    }

    if (mod->dynamic) {
        for (Elf64_Dyn *d = mod->dynamic; d->d_tag != DT_NULL; d++) {
            switch (d->d_tag) {
		case DT_INIT:     mod->init_func = d->d_un.d_ptr; break;
		case DT_INIT_ARRAY: mod->init_array = d->d_un.d_ptr; break;
		case DT_INIT_ARRAYSZ: mod->init_array_sz = d->d_un.d_val;; break;
                case DT_STRTAB:   mod->strtab = (const char *)(base + d->d_un.d_ptr); break;
                case DT_SYMTAB:   mod->symtab = (Elf64_Sym *)(base + d->d_un.d_ptr); break;
                case DT_RELA:     mod->rela = (Elf64_Rela *)(base + d->d_un.d_ptr); break;
                case DT_RELASZ:   mod->relasz = d->d_un.d_val; break;
                case DT_RELAENT:  mod->relaent = d->d_un.d_val; break;
                case DT_JMPREL:   mod->jmprel = (Elf64_Rela *)(base + d->d_un.d_ptr); break;
                case DT_PLTRELSZ: mod->pltrelsz = d->d_un.d_val; break;
                case DT_HASH:     mod->hash = (uint32_t *)(base + d->d_un.d_ptr); break;
                case DT_GNU_HASH: mod->gnu_hash = (uint32_t *)(base + d->d_un.d_ptr); break;
            }
        }

        if (mod->hash) {
            mod->nsyms = mod->hash[1];
        } else if (mod->gnu_hash) {
            mod->nsyms = get_symcount_from_gnu_hash(mod->gnu_hash);
        }

        if (mod->strtab) {
            for (Elf64_Dyn *d = mod->dynamic; d->d_tag != DT_NULL; d++) {
                if (d->d_tag == DT_NEEDED) {
                    const char *so_name = mod->strtab + d->d_un.d_val;
                    if (!load_module(so_name, 0)) {
                        pal_write(2, "ld.so: error: failed to load shared library: ", 45);
                        pal_write(2, so_name, internal_strlen(so_name));
                        pal_write(2, "\n", 1);
                        pal_exit(1);
                    }
                }
            }
        }
    }

    return base + ehdr.e_entry;
}

static uintptr_t lookup_symbol_except(const char *sym_name, Module *skip_mod, size_t *out_size) {
    uintptr_t weak_val = 0;
    size_t weak_size = 0;
    int found_weak = 0;

    for (int m = 0; m < g_num_modules; m++) {
        Module *mod = &g_modules[m];
        if (mod == skip_mod || !mod->symtab || !mod->strtab) continue;

        for (size_t i = 1; i < mod->nsyms; i++) {
            Elf64_Sym *sym = &mod->symtab[i];
            if (sym->st_shndx == SHN_UNDEF) continue;

            unsigned char bind = ELF64_ST_BIND(sym->st_info);
            if (bind == STB_LOCAL) continue;

            const char *name = mod->strtab + sym->st_name;
            if (streq(name, sym_name)) {
                if (bind == STB_GLOBAL || bind == STB_GNU_UNIQUE) {
                    if (out_size) *out_size = sym->st_size;
                    return mod->base + sym->st_value;
                } else if (bind == STB_WEAK && !found_weak) {
                    weak_val = mod->base + sym->st_value;
                    weak_size = sym->st_size;
                    found_weak = 1;
                }
            }
        }
    }

    if (found_weak) {
        if (out_size) *out_size = weak_size;
        return weak_val;
    }

    return 0;
}

static uintptr_t lookup_symbol(const char *sym_name) {
    return lookup_symbol_except(sym_name, NULL, NULL);
}

static void relocate_all(void) {
    for (int m = 0; m < g_num_modules; m++) {
        Module *mod = &g_modules[m];

        if (mod->rela && mod->relaent) {
            Elf64_Xword count = mod->relasz / mod->relaent;
            for (Elf64_Xword i = 0; i < count; i++) {
                Elf64_Rela *r = &mod->rela[i];
                uint32_t type = ELF64_R_TYPE(r->r_info);
                uint32_t sym_idx = ELF64_R_SYM(r->r_info);
                Elf64_Addr *patch = (Elf64_Addr *)(mod->base + r->r_offset);
		Elf64_Sym *sym = (sym_idx != 0 && mod->symtab) ? &mod->symtab[sym_idx] : NULL;
                unsigned char bind = sym ? ELF64_ST_BIND(sym->st_info) : STB_LOCAL;

                switch (type) {
                    case R_X86_64_RELATIVE:
                        *patch = mod->base + r->r_addend;
                        break;

		    case R_X86_64_JUMP_SLOT:
		        if (sym_idx != 0) {
			        const char *sym_name = mod->strtab + mod->symtab[sym_idx].st_name;
			        uintptr_t val = lookup_symbol(sym_name);
			        if (val) {
			            *patch = val;
			        } else {
		        	    Elf64_Sym *sym = (mod->symtab && sym_idx < mod->nsyms) ? &mod->symtab[sym_idx] : NULL;
			            unsigned char bind = sym ? ELF64_ST_BIND(sym->st_info) : STB_LOCAL;
			            if (bind != STB_WEAK) {
			                pal_write(2, "ld.so: undefined symbol: ", 25);
		        	        pal_write(2, sym_name, internal_strlen(sym_name));
			                pal_write(2, "\n", 1);
			                pal_exit(1);
			            }
			        }
                        }
		    break;

		    case R_X86_64_GLOB_DAT:
		        if (sym_idx != 0) {
			        const char *sym_name = mod->strtab + mod->symtab[sym_idx].st_name;
			        uintptr_t val = lookup_symbol(sym_name);
			        if (val) {
			            *patch = val + r->r_addend;
			        } else {
			            Elf64_Sym *sym = (mod->symtab && sym_idx < mod->nsyms) ? &mod->symtab[sym_idx] : NULL;
			            unsigned char bind = sym ? ELF64_ST_BIND(sym->st_info) : STB_LOCAL;
			            if (bind == STB_WEAK) {
			                *patch = 0; /* Weak GOT data pointer safely resolves to NULL */
			            } else {
			                pal_write(2, "ld.so: undefined symbol: ", 25);
			                pal_write(2, sym_name, internal_strlen(sym_name));
			                pal_write(2, "\n", 1);
			                pal_exit(1);
			            }
			        }
			}
		        break;

                    case R_X86_64_64:
			if (sym_idx != 0) {
			        const char *sym_name = mod->strtab + mod->symtab[sym_idx].st_name;
			        uintptr_t val = lookup_symbol(sym_name);
			        if (val) {
			            *patch = val + r->r_addend;
			        } else {
			            Elf64_Sym *sym = (mod->symtab && sym_idx < mod->nsyms) ? &mod->symtab[sym_idx] : NULL;
			            unsigned char bind = sym ? ELF64_ST_BIND(sym->st_info) : STB_LOCAL;
			            if (bind == STB_WEAK) {
			                *patch = 0 + r->r_addend; /* Resolves _ITM_deregisterTMCloneTable to 0 */
			            } else {
			                pal_write(2, "ld.so: undefined symbol: ", 25);
			                pal_write(2, sym_name, internal_strlen(sym_name));
			                pal_write(2, "\n", 1);
			                pal_exit(1);
			            }
			        }
		        } else {
			        *patch = mod->base + r->r_addend;
	                }
                        break;

         	    case R_X86_64_IRELATIVE:
		        uintptr_t *reloc_addr = (uintptr_t *)(mod->base + r->r_offset);
		        uintptr_t resolver_addr = mod->base + r->r_addend;
			typedef uintptr_t (*ifunc_resolver_t)(uint64_t hwcap, const void *cpu_features);
		        ifunc_resolver_t resolver = (ifunc_resolver_t)resolver_addr;
		        *reloc_addr = resolver(0, NULL);
		        break;

                    case R_X86_64_COPY:
                        if (sym_idx != 0) {
                            const char *sym_name = mod->strtab + mod->symtab[sym_idx].st_name;
                            size_t src_size = 0;
                            uintptr_t src_addr = lookup_symbol_except(sym_name, mod, &src_size);
                            if (src_addr) {
                                size_t copy_size = src_size ? src_size : mod->symtab[sym_idx].st_size;
                                internal_memmove(patch, (void *)src_addr, copy_size);
                            }
                        }
                        break;
                }
            }
        }

        if (mod->jmprel) {
            Elf64_Xword count = mod->pltrelsz / sizeof(Elf64_Rela);
            for (Elf64_Xword i = 0; i < count; i++) {
                Elf64_Rela *r = &mod->jmprel[i];
                uint32_t type = ELF64_R_TYPE(r->r_info);
                uint32_t sym_idx = ELF64_R_SYM(r->r_info);
                Elf64_Addr *patch = (Elf64_Addr *)(mod->base + r->r_offset);

                if (type == R_X86_64_JUMP_SLOT || type == R_X86_64_GLOB_DAT) {
                    if (sym_idx != 0) {
                        const char *sym_name = mod->strtab + mod->symtab[sym_idx].st_name;
                        uintptr_t val = lookup_symbol(sym_name);
                        if (!val) {
                            pal_write(2, "ld.so: error: symbol not found: ", 32);
                            pal_write(2, sym_name, internal_strlen(sym_name));
                            pal_write(2, "\n", 1);
                            pal_exit(1);
                        }
                        *patch = val;
                    }
                }
            }
        }
    }
}

static uintptr_t init_main_from_kernel_auxv(Elf64_auxv_t *auxv, const char *exec_name) {
    Elf64_Addr at_phdr = 0, at_entry = 0;
    Elf64_Half phent = 0, phnum = 0;

    for (Elf64_auxv_t *a = auxv; a->a_type != AT_NULL; a++) {
        switch (a->a_type) {
            case AT_PHDR:  at_phdr = a->a_un.a_val; break;
            case AT_PHENT: phent = a->a_un.a_val; break;
            case AT_PHNUM: phnum = a->a_un.a_val; break;
            case AT_ENTRY: at_entry = a->a_un.a_val; break;
        }
    }

    if (!at_phdr || !phent || !phnum || !at_entry) return 0;

    uintptr_t base = 0;
    Elf64_Dyn *dyn = NULL;

    for (int i = 0; i < phnum; i++) {
        Elf64_Phdr *p = (Elf64_Phdr *)(at_phdr + (i * phent));
        if (p->p_type == PT_PHDR) {
            base = at_phdr - p->p_vaddr;
            break;
        }
    }

    for (int i = 0; i < phnum; i++) {
        Elf64_Phdr *p = (Elf64_Phdr *)(at_phdr + (i * phent));
        if (p->p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn *)(base + p->p_vaddr);
            break;
        }
    }

    Module *mod = &g_modules[g_num_modules++];
    internal_memset(mod, 0, sizeof(Module));

    size_t fn_len = internal_strlen(exec_name);
    if (fn_len >= sizeof(mod->name)) fn_len = sizeof(mod->name) - 1;
    internal_memmove(mod->name, exec_name, fn_len);
    mod->name[fn_len] = '\0';
    mod->base = base;
    mod->dynamic = dyn;

    g_target_info.entry = at_entry;
    g_target_info.phent = phent;
    g_target_info.phnum = phnum;
    g_target_info.base = base;
    g_target_info.phdr = at_phdr;

    if (mod->dynamic) {
        for (Elf64_Dyn *d = mod->dynamic; d->d_tag != DT_NULL; d++) {
            switch (d->d_tag) {
		case DT_INIT:          mod->init_func = d->d_un.d_ptr; break;
		case DT_INIT_ARRAY:    mod->init_array = d->d_un.d_ptr; break;
		case DT_INIT_ARRAYSZ:  mod->init_array_sz = d->d_un.d_val; break;
                case DT_STRTAB:   mod->strtab = (const char *)(base + d->d_un.d_ptr); break;
                case DT_SYMTAB:   mod->symtab = (Elf64_Sym *)(base + d->d_un.d_ptr); break;
                case DT_RELA:     mod->rela = (Elf64_Rela *)(base + d->d_un.d_ptr); break;
                case DT_RELASZ:   mod->relasz = d->d_un.d_val; break;
                case DT_RELAENT:  mod->relaent = d->d_un.d_val; break;
                case DT_JMPREL:   mod->jmprel = (Elf64_Rela *)(base + d->d_un.d_ptr); break;
                case DT_PLTRELSZ: mod->pltrelsz = d->d_un.d_val; break;
                case DT_HASH:     mod->hash = (uint32_t *)(base + d->d_un.d_ptr); break;
                case DT_GNU_HASH: mod->gnu_hash = (uint32_t *)(base + d->d_un.d_ptr); break;
            }
        }

        if (mod->hash) {
            mod->nsyms = mod->hash[1];
        } else if (mod->gnu_hash) {
            mod->nsyms = get_symcount_from_gnu_hash(mod->gnu_hash);
        }

        if (mod->strtab) {
            for (Elf64_Dyn *d = mod->dynamic; d->d_tag != DT_NULL; d++) {
                if (d->d_tag == DT_NEEDED) {
                    const char *so_name = mod->strtab + d->d_un.d_val;
                    if (!load_module(so_name, 0)) {
                        pal_write(2, "ld.so: error: failed to load shared library: ", 45);
                        pal_write(2, so_name, internal_strlen(so_name));
                        pal_write(2, "\n", 1);
                        pal_exit(1);
                    }
                }
            }
        }
    }

    return at_entry;
}


typedef void (*init_fn_t)(void);

void run_module_init(Module *mod) {
    if (mod->init_func) {
        init_fn_t init_fn = (init_fn_t)(mod->base + mod->init_func);
        init_fn();
    }

    if (mod->init_array && mod->init_array_sz > 0) {
        init_fn_t *array = (init_fn_t *)(mod->base + mod->init_array);
        size_t count = mod->init_array_sz / sizeof(init_fn_t);

        for (size_t i = 0; i < count; i++) {
            if (array[i] != NULL && (uintptr_t)array[i] != (uintptr_t)-1) {
                array[i]();
            }
        }
    }
}

__attribute__((visibility("hidden")))
void _start_c(uintptr_t *sp) {
    self_relocate(sp);

    int argc = (int)sp[0];
    char **argv = (char **)&sp[1];

    char **envp = &argv[argc + 1];
    while (*envp) envp++;
    Elf64_auxv_t *auxv = (Elf64_auxv_t *)(envp + 1);

    uintptr_t at_base = 0;
    for (Elf64_auxv_t *a = auxv; a->a_type != AT_NULL; a++) {
        if (a->a_type == AT_BASE) {
            at_base = a->a_un.a_val;
            break;
        }
    }

    uintptr_t entry_point = 0;
    int is_interpreter_mode = (at_base != 0);

    if (is_interpreter_mode) {
        entry_point = init_main_from_kernel_auxv(auxv, argv[0]);
        if (!entry_point) {
            pal_write(2, "ld.so: failed to initialize target executable\n", 46);
            pal_exit(1);
        }
    } else {
        if (argc < 2) {
            pal_write(2, "ld.so: missing target executable\nTry '--help' for usage.\n", 57);
            pal_exit(1);
        }

        const char *arg1 = argv[1];
        if (streq_icase(arg1, "--help") || streq_icase(arg1, "-h")) {
            print_help();
            pal_exit(0);
        } else if (streq_icase(arg1, "-v") || streq_icase(arg1, "--version")) {
            print_version();
            pal_exit(0);
        }

        entry_point = load_module(arg1, 1);
        if (!entry_point) {
            pal_write(2, "ld.so: failed to load target executable or shared libraries\n", 60);
            pal_exit(1);
        }
    }

    relocate_all();

    if (!is_interpreter_mode) {
        for (Elf64_auxv_t *a = auxv; a->a_type != AT_NULL; a++) {
            switch (a->a_type) {
                case AT_PHDR:   a->a_un.a_val = g_target_info.phdr; break;
                case AT_PHENT:  a->a_un.a_val = g_target_info.phent; break;
                case AT_PHNUM:  a->a_un.a_val = g_target_info.phnum; break;
                case AT_ENTRY:  a->a_un.a_val = g_target_info.entry; break;
                case AT_BASE:   a->a_un.a_val = g_ld_base; break;
                case AT_EXECFN: a->a_un.a_val = (uintptr_t)argv[1]; break;
            }
        }

        Elf64_auxv_t *auxv_end = auxv;
        while (auxv_end->a_type != AT_NULL) auxv_end++;
        auxv_end++;

        uintptr_t *stack_end = (uintptr_t *)auxv_end;
        size_t total_words = stack_end - &sp[2];

        sp[0] = (uintptr_t)(argc - 1);
        internal_memmove(&sp[1], &sp[2], total_words * sizeof(uintptr_t));
    }

    for (int i = g_num_modules - 1; i >= 0; i--) {
        run_module_init(&g_modules[i]);
    }

    __asm__ __volatile__(
        "mov %[sp], %%rsp\n\t"
        "mov %[entry], %%r12\n\t"
        "xor %%rax, %%rax\n\t"
        "xor %%rbx, %%rbx\n\t"
        "xor %%rcx, %%rcx\n\t"
        "xor %%rdx, %%rdx\n\t"
        "xor %%rsi, %%rsi\n\t"
        "xor %%rdi, %%rdi\n\t"
        "xor %%rbp, %%rbp\n\t"
        "jmp *%%r12\n\t"
        :
        : [sp] "r"(sp), [entry] "r"(entry_point)
        : "r12", "memory"
    );
}

__asm__(
    ".text\n"
    ".global _start\n"
    ".type _start, @function\n"
    "_start:\n"
    "    mov %rsp, %rdi\n"
    "    and $-16, %rsp\n"
    "    call _start_c\n"
    "    hlt\n"
);
