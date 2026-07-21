#include <elf.h>
#include <internal/pal.h>
#include <bits/ldso.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "dynm.h"

#define LD_VERSION  "0.2.0"

static Module g_modules[MAX_MODULES];
static int g_num_modules = 0;
static MainTargetInfo g_target_info;
static uintptr_t g_ld_base = 0;
static char **g_envp = NULL;
static Elf64_auxv_t *g_auxv = NULL;

static int g_debug_flags = 0;
static int g_secure_mode = 0;
static int g_bind_now = 0;

static int g_init_order[MAX_MODULES];
static int g_init_count = 0;

__attribute__((visibility("hidden")))
size_t internal_strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

static int internal_memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *a = s1, *b = s2;
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) return a[i] - b[i];
    }
    return 0;
}

static int streq(const char *a, const char *b) {
    if (a == b) return 1;
    if (!a || !b) return 0;
    while (*a && (*a == *b)) { a++; b++; }
    return *a == *b;
}

static int streq_icase(const char *a, const char *b) {
    if (a == b) return 1;
    if (!a || !b) return 0;
    while (*a) {
        char ca = (*a >= 'A' && *a <= 'Z') ? (*a + 32) : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? (*b + 32) : *b;
        if (ca != cb) return 0;
        a++; b++;
    }
    return *b == '\0';
}

static const char *get_env(const char *name) {
    if (!g_envp) return NULL;
    size_t len = internal_strlen(name);
    for (int i = 0; g_envp[i]; i++) {
        if (internal_memcmp(g_envp[i], name, len) == 0 && g_envp[i][len] == '=') {
            return &g_envp[i][len + 1];
        }
    }
    return NULL;
}

static void get_origin(const char *path, char *origin, size_t sz) {
    const char *last_slash = 0;
    for (const char *p = path; *p; p++) {
        if (*p == '/') last_slash = p;
    }
    if (last_slash) {
        size_t len = last_slash - path + 1;
        if (len >= sz) len = sz - 1;
        internal_memmove(origin, path, len);
        origin[len] = '\0';
    } else {
        origin[0] = '.'; origin[1] = '/'; origin[2] = '\0';
    }
}

static size_t expand_tokens(const char *in, char *out, size_t out_sz, const char *origin) {
    size_t o = 0;
    while (*in && o < out_sz - 1) {
        if (*in == '$') {
            if (in[1] == '{') {
                const char *end = &in[2];
                while (*end && *end != '}') end++;
                size_t tlen = end - &in[2];
                if (tlen == 6 && internal_memcmp(&in[2], "ORIGIN", 6) == 0) {
                    size_t ol = internal_strlen(origin);
                    if (o + ol < out_sz - 1) { internal_memmove(out + o, origin, ol); o += ol; }
                } else if (tlen == 4 && internal_memcmp(&in[2], "LIB", 4) == 0) {
                    const char *lib = "lib64"; size_t ll = 5;
                    if (o + ll < out_sz - 1) { internal_memmove(out + o, lib, ll); o += ll; }
                } else if (tlen == 8 && internal_memcmp(&in[2], "PLATFORM", 8) == 0) {
                    const char *plat = "x86_64"; size_t pl = 6;
                    if (o + pl < out_sz - 1) { internal_memmove(out + o, plat, pl); o += pl; }
                }
                in = *end ? end + 1 : end;
            } else {
                if (internal_memcmp(in+1, "ORIGIN", 6) == 0) {
                    size_t ol = internal_strlen(origin);
                    if (o + ol < out_sz - 1) { internal_memmove(out + o, origin, ol); o += ol; }
                    in += 7;
                } else if (internal_memcmp(in+1, "LIB", 4) == 0) {
                    const char *lib = "lib64"; size_t ll = 5;
                    if (o + ll < out_sz - 1) { internal_memmove(out + o, lib, ll); o += ll; }
                    in += 5;
                } else { out[o++] = *in++; }
            }
        } else { out[o++] = *in++; }
    }
    out[o] = '\0';
    return o;
}

static int expand_and_search(const char *name, const char *paths, char *out, size_t out_sz, const char *origin) {
    char expanded[1024];
    const char *p = paths;
    while (*p) {
        const char *sep = p;
        while (*sep && *sep != ':') sep++;
        size_t plen = sep - p;
        if (plen > 0 && plen < sizeof(expanded)) {
            internal_memmove(expanded, p, plen);
            expanded[plen] = '\0';
            char final_path[1024];
            expand_tokens(expanded, final_path, sizeof(final_path), origin);
            size_t elen = internal_strlen(final_path);
            size_t nlen = internal_strlen(name);
            if (elen + nlen + 2 < out_sz) {
                internal_memmove(out, final_path, elen);
                if (elen > 0 && out[elen-1] != '/') out[elen] = '/';
                internal_memmove(out + elen + (elen > 0 && out[elen-1] != '/' ? 1 : 0), name, nlen + 1);
                int fd = pal_open(out, 0, 0);
                if (fd >= 0) { pal_close(fd); return 1; }
            }
        }
        p = *sep ? sep + 1 : sep;
    }
    return 0;
}

static int search_paths(const char *name, const char *rpath, const char *runpath, char *out, size_t out_sz, const char *origin) {
    if (!name || !name[0]) return 0;
    if (name[0] == '/' || (name[0] == '.' && name[1] == '/')) {
        size_t l = internal_strlen(name);
        if (l < out_sz) { internal_memmove(out, name, l+1); return 1; }
        return 0;
    }
    const char *ld_path = get_env("LD_LIBRARY_PATH");
    if (rpath && !runpath) { if (expand_and_search(name, rpath, out, out_sz, origin)) return 1; }
    if (ld_path && !g_secure_mode) { if (expand_and_search(name, ld_path, out, out_sz, origin)) return 1; }
    if (runpath) { if (expand_and_search(name, runpath, out, out_sz, origin)) return 1; }
    const char *defaults = "./:./lib/:/lib/x86_64-linux-gnu:/usr/lib/x86_64-linux-gnu:/lib64:/usr/lib64:/lib:/usr/lib";
    if (expand_and_search(name, defaults, out, out_sz, origin)) return 1;
    return 0;
}

static size_t get_symcount_from_gnu_hash(const uint32_t *gnu_hash) {
    uint32_t nbuckets = gnu_hash[0];
    uint32_t symndx = gnu_hash[1];
    uint32_t maskwords = gnu_hash[2];
    const uint64_t *bloom = (const uint64_t *)&gnu_hash[4];
    const uint32_t *buckets = (const uint32_t *)&bloom[maskwords];
    const uint32_t *chains = &buckets[nbuckets];
    uint32_t max_sym = 0;
    for (uint32_t i = 0; i < nbuckets; i++) { if (buckets[i] > max_sym) max_sym = buckets[i]; }
    if (max_sym < symndx) return symndx;
    const uint32_t *chain = &chains[max_sym - symndx];
    while (1) { max_sym++; if (*chain & 1) break; chain++; }
    return max_sym;
}

static void print_help(void) {
    static const char *msg = 
        "Usage: ld.so [OPTION]... EXECUTABLE-FILE [ARGS...]\n"
        "Dynamic ELF loader and linker for AIC.\n\n"
        "Options:\n"
        "  --help, -h     Display this help message and exit\n"
        "  --version, -v  Output version information\n"
        "  --list, -l     List dependencies of executable\n"
        "  --verify       Verify executable is valid\n\n";
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
        for (int i = 0; i < phnum; i++) {
            Elf64_Phdr *p = (Elf64_Phdr *)((uintptr_t)ph + (i * phent));
            if (p->p_type == PT_PHDR) { load_bias = at_phdr - p->p_vaddr; break; }
        }
        if (!load_bias) {
            for (int i = 0; i < phnum; i++) {
                Elf64_Phdr *p = (Elf64_Phdr *)((uintptr_t)ph + (i * phent));
                if (p->p_type == PT_LOAD && p->p_offset == 0) {
                    if (at_phdr >= 0x40 && *(uint32_t *)(at_phdr - 0x40) == 0x464c457f)
                        load_bias = (at_phdr - 0x40) - p->p_vaddr;
                    else
                        load_bias = (at_phdr - 0x40) - p->p_vaddr;
                    break;
                }
            }
        }
    }
    g_ld_base = load_bias;

    Elf64_Dyn *dyn = 0;
    for (int i = 0; i < phnum; i++) {
        Elf64_Phdr *p = (Elf64_Phdr *)((uintptr_t)ph + (i * phent));
        if (p->p_type == PT_DYNAMIC) { dyn = (Elf64_Dyn *)(load_bias + p->p_vaddr); break; }
    }
    if (!dyn) return;

    Elf64_Rela *rela = 0; Elf64_Xword relasz = 0, relaent = sizeof(Elf64_Rela);
    for (; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
            case DT_RELA:    rela = (Elf64_Rela *)(load_bias + dyn->d_un.d_ptr); break;
            case DT_RELASZ:  relasz = dyn->d_un.d_val; break;
            case DT_RELAENT: relaent = dyn->d_un.d_val; break;
        }
    }
    if (!rela || !relaent) return;

    for (Elf64_Xword i = 0; i < relasz / relaent; i++) {
        if (ELF64_R_TYPE(rela[i].r_info) == R_X86_64_RELATIVE) {
            Elf64_Addr *patch_addr = (Elf64_Addr *)(load_bias + rela[i].r_offset);
            *patch_addr = load_bias + rela[i].r_addend;
        }
    }
}

// FIX: Changed return type from 'int' to 'uintptr_t' to prevent pointer truncation
static uintptr_t load_module(const char *filename, int is_main, Module *requester) {
    for (int i = 0; i < g_num_modules; i++) {
        if (streq(g_modules[i].name, filename)) {
            if (requester) requester->deps[requester->num_deps++] = i;
            return g_modules[i].base;
        }
    }
    if (g_num_modules >= MAX_MODULES) return 0;

    char fullpath[512];
    const char *rpath = requester ? requester->rpath : 0;
    const char *runpath = requester ? requester->runpath : 0;
    char origin[256];
    get_origin(requester ? requester->name : filename, origin, sizeof(origin));

    if (!search_paths(filename, rpath, runpath, fullpath, sizeof(fullpath), origin)) return 0;

    int fd = pal_open(fullpath, 0, 0);
    if (fd < 0) return 0;

    if (g_debug_flags & LD_DEBUG_LIBS) {
        pal_write(2, "ld.so: loading ", 15);
        pal_write(2, fullpath, internal_strlen(fullpath));
        pal_write(2, "\n", 1);
    }

    Elf64_Ehdr ehdr;
    if (pal_read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) { pal_close(fd); return 0; }
    if (ehdr.e_ident[0] != 0x7f || ehdr.e_ident[1] != 'E' || ehdr.e_ident[2] != 'L' || ehdr.e_ident[3] != 'F') {
        pal_close(fd); return 0;
    }

    Elf64_Phdr phdr[64];
    pal_lseek(fd, ehdr.e_phoff, 0);
    pal_read(fd, phdr, ehdr.e_phentsize * ehdr.e_phnum);

    uintptr_t min_vaddr = (uintptr_t)-1, max_vaddr = 0;
    for (int i = 0; i < ehdr.e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            if (phdr[i].p_vaddr < min_vaddr) min_vaddr = phdr[i].p_vaddr;
            if (phdr[i].p_vaddr + phdr[i].p_memsz > max_vaddr) max_vaddr = phdr[i].p_vaddr + phdr[i].p_memsz;
        }
    }

    uintptr_t page_min = min_vaddr & ~0xFFFU;
    uintptr_t page_max = (max_vaddr + 0xFFFU) & ~0xFFFU;
    size_t total_size = page_max - page_min;
    void *map_addr = NULL; uintptr_t base = 0;

    if (ehdr.e_type == ET_DYN) {
        map_addr = pal_mmap(0, total_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if ((intptr_t)map_addr < 0 || !map_addr) { pal_close(fd); return 0; }
        base = (uintptr_t)map_addr - page_min;
    } else {
        base = 0;
        map_addr = pal_mmap((void *)page_min, total_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
        if (map_addr == (void *)-1 || map_addr != (void *)page_min) { pal_close(fd); return 0; }
    }

    Module *mod = &g_modules[g_num_modules++];
    internal_memset(mod, 0, sizeof(Module));
    size_t fn_len = internal_strlen(fullpath);
    if (fn_len >= sizeof(mod->name)) fn_len = sizeof(mod->name) - 1;
    internal_memmove(mod->name, fullpath, fn_len);
    mod->name[fn_len] = '\0';
    mod->base = base;
    mod->map_size = total_size;

    uintptr_t phdr_addr = 0;
    for (int i = 0; i < ehdr.e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            uintptr_t seg_dst = base + phdr[i].p_vaddr;
            pal_lseek(fd, phdr[i].p_offset, 0);
            pal_read(fd, (void *)seg_dst, phdr[i].p_filesz);
            if (phdr[i].p_memsz > phdr[i].p_filesz)
                internal_memset((void *)(seg_dst + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
        } else if (phdr[i].p_type == PT_DYNAMIC) {
            mod->dynamic = (Elf64_Dyn *)(base + phdr[i].p_vaddr);
        } else if (phdr[i].p_type == PT_PHDR) {
            phdr_addr = base + phdr[i].p_vaddr;
        } else if (phdr[i].p_type == PT_GNU_RELRO) {
            mod->relro_start = phdr[i].p_vaddr;
            mod->relro_size = phdr[i].p_memsz;
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
                case DT_INIT:          mod->init_func = d->d_un.d_ptr; break;
                case DT_INIT_ARRAY:    mod->init_array = d->d_un.d_ptr; break;
                case DT_INIT_ARRAYSZ:  mod->init_array_sz = d->d_un.d_val; break;
                case DT_FINI:          mod->fini_func = d->d_un.d_ptr; break;
                case DT_FINI_ARRAY:    mod->fini_array = d->d_un.d_ptr; break;
                case DT_FINI_ARRAYSZ:  mod->fini_array_sz = d->d_un.d_val; break;
                case DT_STRTAB:   mod->strtab = (const char *)(base + d->d_un.d_ptr); break;
                case DT_SYMTAB:   mod->symtab = (Elf64_Sym *)(base + d->d_un.d_ptr); break;
                case DT_RELA:     mod->rela = (Elf64_Rela *)(base + d->d_un.d_ptr); break;
                case DT_RELASZ:   mod->relasz = d->d_un.d_val; break;
                case DT_RELAENT:  mod->relaent = d->d_un.d_val; break;
                case DT_JMPREL:   mod->jmprel_raw = d->d_un.d_ptr; mod->jmprel = (Elf64_Rela *)(base + d->d_un.d_ptr); break;
                case DT_PLTRELSZ: mod->pltrelsz = d->d_un.d_val; break;
                case DT_PLTGOT:   mod->pltgot = d->d_un.d_ptr; break;
                case DT_HASH:     mod->hash = (uint32_t *)(base + d->d_un.d_ptr); break;
                case DT_GNU_HASH: mod->gnu_hash = (uint32_t *)(base + d->d_un.d_ptr); break;
                case DT_VERSYM:   mod->versym = (uint16_t *)(base + d->d_un.d_ptr); break;
                case DT_VERNEED:  mod->verneed = (Elf64_Verneed *)(base + d->d_un.d_ptr); break;
                case DT_VERNEEDNUM: mod->verneed_num = d->d_un.d_val; break;
                case DT_RPATH:    mod->rpath = (const char *)(base + d->d_un.d_ptr); break;
                case DT_RUNPATH:  mod->runpath = (const char *)(base + d->d_un.d_ptr); break;
                case DT_FLAGS:    mod->flags = d->d_un.d_val; break;
                case DT_FLAGS_1:  mod->flags1 = d->d_un.d_val; break;
            }
        }
        if (mod->hash) mod->nsyms = mod->hash[1];
        else if (mod->gnu_hash) mod->nsyms = get_symcount_from_gnu_hash(mod->gnu_hash);

        if (mod->flags & DF_BIND_NOW || mod->flags1 & DF_1_NOW || g_bind_now) mod->bind_now = 1;

        if (mod->strtab) {
            for (Elf64_Dyn *d = mod->dynamic; d->d_tag != DT_NULL; d++) {
                if (d->d_tag == DT_NEEDED) {
                    const char *so_name = mod->strtab + d->d_un.d_val;
                    if (!load_module(so_name, 0, mod)) {
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

// FIX: Removed the strict version index checking that was silently dropping your symbols
static uintptr_t lookup_symbol_ex(const char *sym_name, Module *req_mod, uint32_t req_idx, size_t *out_size) {
    uintptr_t weak_val = 0; 
    size_t weak_size = 0; 
    int found_weak = 0;

    for (int m = 0; m < g_num_modules; m++) {
        Module *mod = &g_modules[m];
        if (!mod->symtab || !mod->strtab) continue;

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

static uintptr_t lookup_symbol(const char *sym_name) { return lookup_symbol_ex(sym_name, NULL, 0, NULL); }

__attribute__((visibility("hidden")))
uintptr_t _dl_fixup(Module *mod, uintptr_t reloc_offset) {
    Elf64_Rela *rel = (Elf64_Rela *)(mod->base + mod->jmprel_raw + reloc_offset);
    uint32_t sym_idx = ELF64_R_SYM(rel->r_info);
    const char *sym_name = mod->strtab + mod->symtab[sym_idx].st_name;
    uintptr_t val = lookup_symbol_ex(sym_name, mod, sym_idx, NULL);
    if (!val) {
        pal_write(2, "ld.so: lazy bind failed: ", 24);
        pal_write(2, sym_name, internal_strlen(sym_name));
        pal_write(2, "\n", 1); pal_exit(1);
    }
    Elf64_Addr *patch = (Elf64_Addr *)(mod->base + rel->r_offset);
    *patch = val;
    return val;
}

void *_dl_runtime_resolve(uint64_t arg1, uint64_t arg2);

AAL_DEFINE_DL_RUNTIME_RESOLVE();

static void relocate_module(Module *mod) {
    if (mod->pltgot) {
        uintptr_t *got = (uintptr_t *)(mod->base + mod->pltgot);
        got[1] = (uintptr_t)mod;
        if (!mod->bind_now) got[2] = (uintptr_t)_dl_runtime_resolve;
    }

    if (mod->rela && mod->relaent) {
        Elf64_Xword count = mod->relasz / mod->relaent;
        for (Elf64_Xword i = 0; i < count; i++) {
            Elf64_Rela *r = &mod->rela[i];
            uint32_t type = ELF64_R_TYPE(r->r_info);
            uint32_t sym_idx = ELF64_R_SYM(r->r_info);
            Elf64_Addr *patch = (Elf64_Addr *)(mod->base + r->r_offset);
            switch (type) {
                case R_X86_64_RELATIVE: *patch = mod->base + r->r_addend; break;
                case R_X86_64_64: case R_X86_64_GLOB_DAT:
                    if (sym_idx != 0) {
                        const char *sym_name = mod->strtab + mod->symtab[sym_idx].st_name;
                        uintptr_t val = lookup_symbol_ex(sym_name, mod, sym_idx, NULL);
                        if (val) *patch = val + r->r_addend;
                        else if (ELF64_ST_BIND(mod->symtab[sym_idx].st_info) == STB_WEAK) *patch = r->r_addend;
                        else { pal_write(2, "ld.so: undefined symbol: ", 25); pal_write(2, sym_name, internal_strlen(sym_name)); pal_write(2, "\n", 1); pal_exit(1); }
                    } else *patch = mod->base + r->r_addend;
                    break;
                case R_X86_64_JUMP_SLOT:
                    if (mod->bind_now) {
                        if (sym_idx != 0) {
                            const char *sym_name = mod->strtab + mod->symtab[sym_idx].st_name;
                            uintptr_t val = lookup_symbol_ex(sym_name, mod, sym_idx, NULL);
                            if (val) *patch = val;
                            else if (ELF64_ST_BIND(mod->symtab[sym_idx].st_info) != STB_WEAK) { pal_write(2, "ld.so: undefined symbol: ", 25); pal_write(2, sym_name, internal_strlen(sym_name)); pal_write(2, "\n", 1); pal_exit(1); }
                        }
                    }
                    break;
                case R_X86_64_IRELATIVE: {
                    typedef uintptr_t (*ifunc_resolver_t)(uint64_t, const void *);
                    ifunc_resolver_t resolver = (ifunc_resolver_t)(mod->base + r->r_addend);
                    *patch = resolver(0, NULL);
                    break;
                }
                case R_X86_64_COPY:
                    if (sym_idx != 0) {
                        const char *sym_name = mod->strtab + mod->symtab[sym_idx].st_name;
                        size_t src_size = 0;
                        uintptr_t src_addr = lookup_symbol_ex(sym_name, mod, sym_idx, &src_size);
                        if (src_addr) internal_memmove(patch, (void *)src_addr, src_size ? src_size : mod->symtab[sym_idx].st_size);
                    }
                    break;
            }
        }
    }
    mod->is_relocated = 1;
}

static void relocate_all(void) {
    for (int m = 0; m < g_num_modules; m++) {
        if (!g_modules[m].is_relocated) relocate_module(&g_modules[m]);
    }
}

static void apply_relro(Module *mod) {
    if (mod->relro_size > 0) {
        uintptr_t start = mod->base + mod->relro_start;
        uintptr_t page_start = start & ~0xFFFUL;
        uintptr_t end = (start + mod->relro_size + 0xFFFUL) & ~0xFFFUL;
        mprotect((void *)page_start, end - page_start, PROT_READ);
    }
}

static void build_init_order(void) {
    int queue[MAX_MODULES]; int in_queue[MAX_MODULES]; 
    internal_memset(in_queue, 0, sizeof(in_queue));
    int front = 0, back = 0;
    queue[back++] = 0; in_queue[0] = 1; g_init_count = 0;
    while (front < back) {
        int idx = queue[front++];
        g_init_order[g_init_count++] = idx;
        for (int i = 0; i < g_modules[idx].num_deps; i++) {
            int dep_idx = g_modules[idx].deps[i];
            if (!in_queue[dep_idx]) { in_queue[dep_idx] = 1; queue[back++] = dep_idx; }
        }
    }
}

typedef void (*init_fn_t)(void);
void run_module_init(Module *mod) {
    if (mod->init_func) { init_fn_t init_fn = (init_fn_t)(mod->base + mod->init_func); init_fn(); }
    if (mod->init_array && mod->init_array_sz > 0) {
        init_fn_t *array = (init_fn_t *)(mod->base + mod->init_array);
        size_t count = mod->init_array_sz / sizeof(init_fn_t);
        for (size_t i = 0; i < count; i++) { if (array[i] && (uintptr_t)array[i] != (uintptr_t)-1) array[i](); }
    }
}

void run_module_fini(Module *mod) {
    if (mod->fini_array && mod->fini_array_sz > 0) {
        init_fn_t *array = (init_fn_t *)(mod->base + mod->fini_array);
        for (int i = (mod->fini_array_sz / sizeof(init_fn_t)) - 1; i >= 0; i--) { if (array[i]) array[i](); }
    }
    if (mod->fini_func) { init_fn_t fini_fn = (init_fn_t)(mod->base + mod->fini_func); fini_fn(); }
}

__attribute__((visibility("default")))
void *dlopen(const char *filename, int flags) {
    if (!filename) return (void *)&g_modules[0];
    uintptr_t base = load_module(filename, 0, NULL);
    if (!base) return NULL;
    Module *mod = &g_modules[g_num_modules - 1];
    mod->ref_count = 1;
    if (flags & RTLD_NOW) mod->bind_now = 1;
    relocate_all();
    build_init_order();
    run_module_init(mod);
    return (void *)mod;
}

__attribute__((visibility("default")))
void *dlsym(void *handle, const char *symbol) {
    if (handle == RTLD_DEFAULT || handle == RTLD_NEXT) return (void *)lookup_symbol(symbol);
    Module *mod = (Module *)handle;
    size_t sz; return (void *)lookup_symbol_ex(symbol, mod, 0, &sz);
}

__attribute__((visibility("default")))
int dlclose(void *handle) {
    Module *mod = (Module *)handle;
    if (!mod || mod == &g_modules[0]) return -1;
    mod->ref_count--;
    if (mod->ref_count <= 0) {
        run_module_fini(mod);
        if (mod->map_size) munmap((void *)mod->base, mod->map_size);
    }
    return 0;
}

__attribute__((visibility("default")))
const char *dlerror(void) { return "AIC ld.so dynamic linking error"; }

__attribute__((visibility("default")))
unsigned long getauxval(unsigned long type) {
    if (!g_auxv) return 0;
    for (Elf64_auxv_t *a = g_auxv; a->a_type != AT_NULL; a++) {
        if (a->a_type == type) return a->a_un.a_val;
    }
    return 0;
}

struct link_map;
__attribute__((visibility("default")))
const char *la_objsearch(const char *name, uintptr_t *cookie, unsigned int flag) { return name; }
__attribute__((visibility("default")))
uintptr_t la_symbind64(Elf64_Sym *sym, uint64_t idx, uintptr_t *refcook, uintptr_t *defcook, int *flags, const char *symname) { return sym->st_value; }
__attribute__((visibility("default")))
unsigned int la_objopen(struct link_map *map, Lmid_t lmid, uintptr_t *cookie) { return 0; }

static uintptr_t init_main_from_kernel_auxv(Elf64_auxv_t *auxv, const char *exec_name) {
    Elf64_Addr at_phdr = 0, at_entry = 0; Elf64_Half phent = 0, phnum = 0;
    for (Elf64_auxv_t *a = auxv; a->a_type != AT_NULL; a++) {
        switch (a->a_type) {
            case AT_PHDR:  at_phdr = a->a_un.a_val; break;
            case AT_PHENT: phent = a->a_un.a_val; break;
            case AT_PHNUM: phnum = a->a_un.a_val; break;
            case AT_ENTRY: at_entry = a->a_un.a_val; break;
        }
    }
    if (!at_phdr || !phent || !phnum || !at_entry) return 0;

    uintptr_t base = 0; Elf64_Dyn *dyn = NULL;
    for (int i = 0; i < phnum; i++) {
        Elf64_Phdr *p = (Elf64_Phdr *)(at_phdr + (i * phent));
        if (p->p_type == PT_PHDR) { base = at_phdr - p->p_vaddr; break; }
    }
    for (int i = 0; i < phnum; i++) {
        Elf64_Phdr *p = (Elf64_Phdr *)(at_phdr + (i * phent));
        if (p->p_type == PT_DYNAMIC) { dyn = (Elf64_Dyn *)(base + p->p_vaddr); break; }
    }

    Module *mod = &g_modules[g_num_modules++]; internal_memset(mod, 0, sizeof(Module));
    size_t fn_len = internal_strlen(exec_name);
    if (fn_len >= sizeof(mod->name)) fn_len = sizeof(mod->name) - 1;
    internal_memmove(mod->name, exec_name, fn_len); mod->name[fn_len] = '\0';
    mod->base = base; mod->dynamic = dyn;
    g_target_info.entry = at_entry; g_target_info.phent = phent; g_target_info.phnum = phnum;
    g_target_info.base = base; g_target_info.phdr = at_phdr;

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
                case DT_JMPREL:   mod->jmprel_raw = d->d_un.d_ptr; mod->jmprel = (Elf64_Rela *)(base + d->d_un.d_ptr); break;
                case DT_PLTRELSZ: mod->pltrelsz = d->d_un.d_val; break;
                case DT_PLTGOT:   mod->pltgot = d->d_un.d_ptr; break;
                case DT_HASH:     mod->hash = (uint32_t *)(base + d->d_un.d_ptr); break;
                case DT_GNU_HASH: mod->gnu_hash = (uint32_t *)(base + d->d_un.d_ptr); break;
                case DT_RPATH:    mod->rpath = (const char *)(base + d->d_un.d_ptr); break;
                case DT_RUNPATH:  mod->runpath = (const char *)(base + d->d_un.d_ptr); break;
                case DT_FLAGS:    mod->flags = d->d_un.d_val; break;
                case DT_FLAGS_1:  mod->flags1 = d->d_un.d_val; break;
            }
        }
        if (mod->hash) mod->nsyms = mod->hash[1];
        else if (mod->gnu_hash) mod->nsyms = get_symcount_from_gnu_hash(mod->gnu_hash);
        if (mod->flags & DF_BIND_NOW || mod->flags1 & DF_1_NOW || g_bind_now) mod->bind_now = 1;

        if (mod->strtab) {
            for (Elf64_Dyn *d = mod->dynamic; d->d_tag != DT_NULL; d++) {
                if (d->d_tag == DT_NEEDED) {
                    const char *so_name = mod->strtab + d->d_un.d_val;
                    if (!load_module(so_name, 0, mod)) {
                        pal_write(2, "ld.so: error: failed to load shared library: ", 45);
                        pal_write(2, so_name, internal_strlen(so_name)); pal_write(2, "\n", 1); pal_exit(1);
                    }
                }
            }
        }
    }
    return at_entry;
}

__attribute__((visibility("hidden")))
void _start_c(uintptr_t *sp) {
    self_relocate(sp);
    int argc = (int)sp[0]; char **argv = (char **)&sp[1];
    g_envp = &argv[argc + 1];
    while (*g_envp) g_envp++;
    g_auxv = (Elf64_auxv_t *)(g_envp + 1);
    g_envp = &argv[argc + 1];

    uintptr_t at_base = 0;
    for (Elf64_auxv_t *a = g_auxv; a->a_type != AT_NULL; a++) {
        if (a->a_type == AT_BASE) { at_base = a->a_un.a_val; break; }
        if (a->a_type == AT_SECURE) g_secure_mode = a->a_un.a_val;
    }

    const char *dbg_str = get_env("LD_DEBUG");
    if (dbg_str) {
        if (streq(dbg_str, "all") || internal_memcmp(dbg_str, "libs", 4) == 0) g_debug_flags |= LD_DEBUG_LIBS;
        if (streq(dbg_str, "all") || internal_memcmp(dbg_str, "symbols", 7) == 0) g_debug_flags |= LD_DEBUG_SYMBOLS;
    }
    if (get_env("LD_BIND_NOW")) g_bind_now = 1;

    if (!g_secure_mode && get_env("LD_AUDIT")) {
        char buf[1024]; internal_memmove(buf, get_env("LD_AUDIT"), internal_strlen(get_env("LD_AUDIT")) + 1);
        char *tok = buf;
        while (*tok) {
            while (*tok == ' ' || *tok == ':') tok++;
            if (!*tok) break;
            char *end = tok; while (*end && *end != ' ' && *end != ':') end++;
            char saved = *end; *end = '\0';
            load_module(tok, 0, NULL);
            *end = saved; tok = end;
        }
    }

    uintptr_t entry_point = 0;
    int is_interpreter_mode = (at_base != 0);

    if (is_interpreter_mode) {
        entry_point = init_main_from_kernel_auxv(g_auxv, argv[0]);
        if (!entry_point) { pal_write(2, "ld.so: failed to initialize target executable\n", 46); pal_exit(1); }
    } else {
        if (argc < 2) { pal_write(2, "ld.so: missing target executable\nTry '--help' for usage.\n", 57); pal_exit(1); }
        const char *arg1 = argv[1];
        if (streq_icase(arg1, "--help") || streq_icase(arg1, "-h")) { print_help(); pal_exit(0); }
        else if (streq_icase(arg1, "-v") || streq_icase(arg1, "--version")) { print_version(); pal_exit(0); }
        else if (streq_icase(arg1, "--list") || streq_icase(arg1, "-l")) {
            if (argc < 3) { pal_write(2, "ld.so: --list requires an argument\n", 36); pal_exit(1); }
            load_module(argv[2], 1, NULL);
            for (int i = 0; i < g_num_modules; i++) { pal_write(1, "\t", 1); pal_write(1, g_modules[i].name, internal_strlen(g_modules[i].name)); pal_write(1, "\n", 1); }
            pal_exit(0);
        }
        else if (streq_icase(arg1, "--verify")) {
            if (argc < 3) { pal_write(2, "ld.so: --verify requires an argument\n", 38); pal_exit(1); }
            int fd = pal_open(argv[2], 0, 0);
            if (fd >= 0) { pal_write(1, "valid\n", 6); pal_close(fd); pal_exit(0); }
            pal_write(2, "invalid\n", 8); pal_exit(1);
        }

        if (!g_secure_mode && get_env("LD_PRELOAD")) {
            char buf[2048]; internal_memmove(buf, get_env("LD_PRELOAD"), internal_strlen(get_env("LD_PRELOAD")) + 1);
            char *tok = buf;
            while (*tok) {
                while (*tok == ' ' || *tok == ':') tok++;
                if (!*tok) break;
                char *end = tok; while (*end && *end != ' ' && *end != ':') end++;
                char saved = *end; *end = '\0';
                int idx = load_module(tok, 0, NULL);
                if (idx) g_modules[g_num_modules - 1].is_preloaded = 1;
                *end = saved; tok = end;
            }
        }

        entry_point = load_module(arg1, 1, NULL);
        if (!entry_point) { pal_write(2, "ld.so: failed to load target executable or shared libraries\n", 60); pal_exit(1); }
    }

    relocate_all();
    build_init_order();

    for (int i = 0; i < g_num_modules; i++) apply_relro(&g_modules[i]);

    if (!is_interpreter_mode) {
        for (Elf64_auxv_t *a = g_auxv; a->a_type != AT_NULL; a++) {
            switch (a->a_type) {
                case AT_PHDR:   a->a_un.a_val = g_target_info.phdr; break;
                case AT_PHENT:  a->a_un.a_val = g_target_info.phent; break;
                case AT_PHNUM:  a->a_un.a_val = g_target_info.phnum; break;
                case AT_ENTRY:  a->a_un.a_val = g_target_info.entry; break;
                case AT_BASE:   a->a_un.a_val = g_ld_base; break;
                case AT_EXECFN: a->a_un.a_val = (uintptr_t)argv[1]; break;
            }
        }
        Elf64_auxv_t *auxv_end = g_auxv; while (auxv_end->a_type != AT_NULL) auxv_end++;
        uintptr_t *stack_end = (uintptr_t *)(auxv_end + 1);
        size_t total_words = stack_end - &sp[2];
        sp[0] = (uintptr_t)(argc - 1);
        internal_memmove(&sp[1], &sp[2], total_words * sizeof(uintptr_t));
    }

    for (int i = 0; i < g_init_count; i++) run_module_init(&g_modules[g_init_order[i]]);

    aal_ldso_jump_to_entry(sp, entry_point);
}

AAL_DEFINE_LDSO_ENTRY();
