#define _GNU_SOURCE
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define PAGE_SIZE 4096
#define PAGE_ALIGN_DOWN(x) ((x) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(x)   (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define MAX_LIBS 32

typedef struct LoadedLib {
    char name[256];
    uintptr_t base;
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    Elf64_Dyn *dynamic;
    Elf64_Sym *symtab;
    const char *strtab;
    Elf64_Rela *rela;
    size_t rela_size;
    Elf64_Rela *jmprel;
    size_t jmprel_size;
    size_t sym_count;
} LoadedLib;

static LoadedLib g_libs[MAX_LIBS];
static size_t g_lib_count = 0;

/* Helper to convert file offsets or relative VAs to runtime memory pointers */
static inline void *resolve_ptr(uintptr_t base, uintptr_t addr) {
    if (addr < base) {
        return (void *)(base + addr);
    }
    return (void *)addr;
}

/* Parse DT_GNU_HASH to determine total dynamic symbol count */
static size_t parse_gnu_hash_symcount(uintptr_t base, uintptr_t gnu_hash_ptr) {
    Elf64_Word *gnuhash = (Elf64_Word *)resolve_ptr(base, gnu_hash_ptr);
    uint32_t nbuckets = gnuhash[0];
    uint32_t symoffset = gnuhash[1];
    uint32_t bloom_size = gnuhash[2];
    
    /* Bloom filter precedes buckets. Words can be 32-bit or 64-bit depending on architecture */
    uint32_t *buckets = (uint32_t *)&gnuhash[4 + (sizeof(void *) / 4) * bloom_size];
    uint32_t *chains = &buckets[nbuckets];

    uint32_t max_sym = 0;
    for (uint32_t b = 0; b < nbuckets; b++) {
        if (buckets[b] > max_sym) {
            max_sym = buckets[b];
        }
    }

    if (max_sym >= symoffset) {
        while (!(chains[max_sym - symoffset] & 1)) {
            max_sym++;
        }
        return max_sym + 1;
    }
    return symoffset;
}

/* Load an ELF binary into memory, map segments, and record dynamic headers */
static LoadedLib *load_elf(const char *path, uintptr_t preferred_base) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return NULL;
    }

    struct stat st;
    fstat(fd, &st);

    void *file_mem = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_mem == MAP_FAILED) {
        perror("mmap file");
        close(fd);
        return NULL;
    }

    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)file_mem;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0 || ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        fprintf(stderr, "Invalid ELF file: %s\n", path);
        munmap(file_mem, st.st_size);
        close(fd);
        return NULL;
    }

    Elf64_Phdr *ph = (Elf64_Phdr *)((uintptr_t)file_mem + ehdr->e_phoff);
    
    /* Calculate virtual layout size */
    uintptr_t min_vaddr = (uintptr_t)-1;
    uintptr_t max_vaddr = 0;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (ph[i].p_type == PT_LOAD) {
            if (ph[i].p_vaddr < min_vaddr) min_vaddr = ph[i].p_vaddr;
            if (ph[i].p_vaddr + ph[i].p_memsz > max_vaddr) max_vaddr = ph[i].p_vaddr + ph[i].p_memsz;
        }
    }

    uintptr_t base = 0;
    if (ehdr->e_type == ET_DYN) {
        base = preferred_base;
    }

    /* Map PT_LOAD segments */
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (ph[i].p_type == PT_LOAD) {
            uintptr_t vaddr = base + ph[i].p_vaddr;
            uintptr_t map_addr = PAGE_ALIGN_DOWN(vaddr);
            size_t page_off = vaddr - map_addr;
            size_t map_len = PAGE_ALIGN_UP(ph[i].p_memsz + page_off);

            void *res = mmap((void *)map_addr, map_len, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
            if (res == MAP_FAILED) {
                perror("mmap segment");
                munmap(file_mem, st.st_size);
                close(fd);
                return NULL;
            }

            if (ph[i].p_filesz > 0) {
                memcpy((void *)vaddr, (char *)file_mem + ph[i].p_offset, ph[i].p_filesz);
            }
            if (ph[i].p_memsz > ph[i].p_filesz) {
                memset((void *)(vaddr + ph[i].p_filesz), 0, ph[i].p_memsz - ph[i].p_filesz);
            }
        }
    }

    LoadedLib *lib = &g_libs[g_lib_count++];
    memset(lib, 0, sizeof(LoadedLib));
    strncpy(lib->name, path, sizeof(lib->name) - 1);
    lib->base = base;
    lib->ehdr = (Elf64_Ehdr *)(base + (ehdr->e_type == ET_DYN ? 0 : 0));

    /* Locate dynamic segment */
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (ph[i].p_type == PT_DYNAMIC) {
            lib->dynamic = (Elf64_Dyn *)resolve_ptr(base, ph[i].p_vaddr);
            break;
        }
    }

    /* Parse dynamic table */
    if (lib->dynamic) {
        uintptr_t hash_ptr = 0;
        uintptr_t gnu_hash_ptr = 0;

        for (Elf64_Dyn *dyn = lib->dynamic; dyn->d_tag != DT_NULL; dyn++) {
            switch (dyn->d_tag) {
                case DT_SYMTAB:
                    lib->symtab = (Elf64_Sym *)resolve_ptr(base, dyn->d_un.d_ptr);
                    break;
                case DT_STRTAB:
                    lib->strtab = (const char *)resolve_ptr(base, dyn->d_un.d_ptr);
                    break;
                case DT_RELA:
                    lib->rela = (Elf64_Rela *)resolve_ptr(base, dyn->d_un.d_ptr);
                    break;
                case DT_RELASZ:
                    lib->rela_size = dyn->d_un.d_val;
                    break;
                case DT_JMPREL:
                    lib->jmprel = (Elf64_Rela *)resolve_ptr(base, dyn->d_un.d_ptr);
                    break;
                case DT_PLTRELSZ:
                    lib->jmprel_size = dyn->d_un.d_val;
                    break;
                case DT_HASH:
                    hash_ptr = dyn->d_un.d_ptr;
                    break;
                case DT_GNU_HASH:
                    gnu_hash_ptr = dyn->d_un.d_ptr;
                    break;
            }
        }

        if (gnu_hash_ptr) {
            lib->sym_count = parse_gnu_hash_symcount(base, gnu_hash_ptr);
        } else if (hash_ptr) {
            Elf64_Word *hash = (Elf64_Word *)resolve_ptr(base, hash_ptr);
            lib->sym_count = hash[1];
        }
    }

    /* Keep underlying file segment data accessible or clean up file descriptor */
    munmap(file_mem, st.st_size);
    close(fd);

    return lib;
}

/* Find symbol in loaded libraries */
static uintptr_t lookup_symbol(const char *name, LoadedLib **found_in) {
    for (size_t i = 0; i < g_lib_count; i++) {
        LoadedLib *lib = &g_libs[i];
        if (!lib->symtab || !lib->strtab) continue;

        size_t count = lib->sym_count ? lib->sym_count : 1000;
        for (size_t s = 0; s < count; s++) {
            Elf64_Sym *sym = &lib->symtab[s];
            if (sym->st_shndx != SHN_UNDEF && sym->st_name != 0) {
                const char *sym_name = &lib->strtab[sym->st_name];
                if (strcmp(sym_name, name) == 0) {
                    if (found_in) *found_in = lib;
                    return lib->base + sym->st_value;
                }
            }
        }
    }
    return 0;
}

/* Load DT_NEEDED shared library dependencies recursively */
static void load_dependencies(LoadedLib *lib) {
    if (!lib->dynamic) return;

    for (Elf64_Dyn *dyn = lib->dynamic; dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_NEEDED) {
            const char *lib_name = &lib->strtab[dyn->d_un.d_val];
            
            int already_loaded = 0;
            for (size_t i = 0; i < g_lib_count; i++) {
                if (strstr(g_libs[i].name, lib_name)) {
                    already_loaded = 1;
                    break;
                }
            }

            if (!already_loaded) {
                /* Basic search paths */
                char path[512];
                snprintf(path, sizeof(path), "./%s", lib_name);
                if (access(path, F_OK) != 0) {
                    snprintf(path, sizeof(path), "/lib/x86_64-linux-gnu/%s", lib_name);
                }
                if (access(path, F_OK) != 0) {
                    snprintf(path, sizeof(path), "/usr/lib/%s", lib_name);
                }

                LoadedLib *dep = load_elf(path, 0x7f0000000000 + (g_lib_count * 0x10000000));
                if (dep) {
                    load_dependencies(dep);
                }
            }
        }
    }
}

/* Apply R_X86_64 RELA relocations */
static void apply_relocs(LoadedLib *lib, Elf64_Rela *rela, size_t size) {
    if (!rela || !size) return;

    size_t count = size / sizeof(Elf64_Rela);
    for (size_t i = 0; i < count; i++) {
        Elf64_Rela *r = &rela[i];
        uint32_t type = ELF64_R_TYPE(r->r_info);
        uint32_t sym_idx = ELF64_R_SYM(r->r_info);

        uintptr_t *target = (uintptr_t *)resolve_ptr(lib->base, r->r_offset);
        uintptr_t sym_val = 0;

        if (sym_idx != 0 && lib->symtab && lib->strtab) {
            Elf64_Sym *sym = &lib->symtab[sym_idx];
            const char *sym_name = &lib->strtab[sym->st_name];
            sym_val = lookup_symbol(sym_name, NULL);
        }

        switch (type) {
            case R_X86_64_RELATIVE:
                *target = lib->base + r->r_addend;
                break;
            case R_X86_64_GLOB_DAT:
            case R_X86_64_JUMP_SLOT:
                if (sym_val) {
                    *target = sym_val + r->r_addend;
                }
                break;
            case R_X86_64_64:
                if (sym_val) {
                    *target = sym_val + r->r_addend;
                }
                break;
            default:
                break;
        }
    }
}

/* Restore exact memory page permissions (PROT_READ, PROT_WRITE, PROT_EXEC) */
static void protect_segments(LoadedLib *lib, const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return;

    Elf64_Ehdr ehdr;
    read(fd, &ehdr, sizeof(ehdr));

    Elf64_Phdr ph[ehdr.e_phnum];
    lseek(fd, ehdr.e_phoff, SEEK_SET);
    read(fd, ph, sizeof(ph));
    close(fd);

    for (int i = 0; i < ehdr.e_phnum; i++) {
        if (ph[i].p_type == PT_LOAD) {
            uintptr_t vaddr = lib->base + ph[i].p_vaddr;
            uintptr_t map_addr = PAGE_ALIGN_DOWN(vaddr);
            size_t page_off = vaddr - map_addr;
            size_t map_len = PAGE_ALIGN_UP(ph[i].p_memsz + page_off);

            int prot = 0;
            if (ph[i].p_flags & PF_R) prot |= PROT_READ;
            if (ph[i].p_flags & PF_W) prot |= PROT_WRITE;
            if (ph[i].p_flags & PF_X) prot |= PROT_EXEC;

            mprotect((void *)map_addr, map_len, prot);
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <elf_binary>\n", argv[0]);
        return 1;
    }

    const char *target_path = argv[1];

    /* Load main ELF binary */
    LoadedLib *main_lib = load_elf(target_path, 0x400000);
    if (!main_lib) {
        fprintf(stderr, "Failed to load main binary: %s\n", target_path);
        return 1;
    }

    /* Recursively load dependencies */
    load_dependencies(main_lib);

    /* Apply relocations across all loaded objects */
    for (size_t i = 0; i < g_lib_count; i++) {
        apply_relocs(&g_libs[i], g_libs[i].rela, g_libs[i].rela_size);
        apply_relocs(&g_libs[i], g_libs[i].jmprel, g_libs[i].jmprel_size);
    }

    /* Apply memory protections */
    protect_segments(main_lib, target_path);

    /* Get entry point address */
    uintptr_t entry = main_lib->base + main_lib->ehdr->e_entry;

    /* Jump to target entry point */
    typedef void (*entry_func_t)(void);
    entry_func_t start = (entry_func_t)entry;
    start();

    return 0;
}
