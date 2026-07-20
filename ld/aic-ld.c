/*
 * aic-ld.c - Working x86-64 dynamic linker
 *
 * Combines techniques from min-dl (NCKU) and musl libc.
 *
 * Build:
 *   gcc -nostdlib -fno-stack-protector -fPIC -shared  *       -Wl,-e,_start -o aic-ld.so aic-ld.c aic-ld-start.s
 *
 * Test:
 *   ./aic-ld.so /bin/echo hello
 *   LD_LIBRARY_PATH=/usr/lib ./myprogram
 */

#include "syscall.h"

/* ========================================================================
 * SYSCALL WRAPPERS (inline asm, no libc)
 * ======================================================================== */

static inline long __syscall0(long n) {
    long ret;
    __asm__ volatile ("syscall" : "=a"(ret) : "a"(n)
        : "rcx", "r11", "memory");
    return ret;
}

static inline long __syscall1(long n, long a1) {
    long ret;
    __asm__ volatile ("syscall" : "=a"(ret) : "a"(n), "D"(a1)
        : "rcx", "r11", "memory");
    return ret;
}

static inline long __syscall2(long n, long a1, long a2) {
    long ret;
    __asm__ volatile ("syscall" : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2)
        : "rcx", "r11", "memory");
    return ret;
}

static inline long __syscall3(long n, long a1, long a2, long a3) {
    long ret;
    register long r10 __asm__("r10") = a3;
    __asm__ volatile ("syscall" : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "r"(r10)
        : "rcx", "r11", "memory");
    return ret;
}

static inline long __syscall6(long n, long a1, long a2, long a3,
                                long a4, long a5, long a6) {
    long ret;
    register long r10 __asm__("r10") = a3;
    register long r8  __asm__("r8")  = a4;
    register long r9  __asm__("r9")  = a5;
    __asm__ volatile ("syscall" : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "memory");
    return ret;
}

static inline void *sys_mmap(void *addr, size_t len, int prot,
                              int flags, int fd, long off) {
    return (void *)__syscall6(SYS_mmap, (long)addr, (long)len,
                               prot, flags, fd, off);
}

static inline int sys_mprotect(void *addr, size_t len, int prot) {
    return (int)__syscall3(SYS_mprotect, (long)addr, (long)len, prot);
}

static inline int sys_munmap(void *addr, size_t len) {
    return (int)__syscall2(SYS_munmap, (long)addr, (long)len);
}

static inline int sys_open(const char *path, int flags) {
    return (int)__syscall2(SYS_open, (long)path, flags);
}

static inline ssize_t sys_read(int fd, void *buf, size_t count) {
    return (ssize_t)__syscall3(SYS_read, fd, (long)buf, (long)count);
}

static inline int sys_close(int fd) {
    return (int)__syscall1(SYS_close, fd);
}

static inline off_t sys_lseek(int fd, off_t offset, int whence) {
    return (off_t)__syscall3(SYS_lseek, fd, offset, whence);
}

static inline void sys_exit(int code) {
    __syscall1(SYS_exit, code);
    __builtin_unreachable();
}

static inline void sys_write(int fd, const void *buf, size_t len) {
    __syscall3(SYS_write, fd, (long)buf, (long)len);
}

/* ========================================================================
 * STRING UTILITIES (from musl - tiny, no libc dep)
 * ======================================================================== */

static size_t ld_strlen(const char *s) {
    const char *a = s;
    while (*a) a++;
    return a - s;
}

static int ld_strcmp(const char *l, const char *r) {
    for (; *l == *r && *l; l++, r++);
    return *(unsigned char *)l - *(unsigned char *)r;
}

static int ld_strncmp(const char *l, const char *r, size_t n) {
    for (; n && *l == *r && *l; n--, l++, r++);
    return n ? *(unsigned char *)l - *(unsigned char *)r : 0;
}

static void ld_memcpy(void *dst, const void *src, size_t n) {
    char *d = dst;
    const char *s = src;
    while (n--) *d++ = *s++;
}

static void ld_memset(void *dst, int c, size_t n) {
    unsigned char *d = dst;
    while (n--) *d++ = (unsigned char)c;
}

static int ld_isdigit(int c) { return c >= '0' && c <= '9'; }

static int ld_isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r'
        || c == '\f' || c == '\v';
}

/* ========================================================================
 * ERROR REPORTING
 * ======================================================================== */

static void ld_fatal(const char *msg) {
    sys_write(2, "[ldso] ", 7);
    sys_write(2, msg, ld_strlen(msg));
    sys_write(2, "\n", 1);
    sys_exit(127);
}

static void ld_fatal2(const char *msg1, const char *msg2) {
    sys_write(2, "[ldso] ", 7);
    sys_write(2, msg1, ld_strlen(msg1));
    sys_write(2, msg2, ld_strlen(msg2));
    sys_write(2, "\n", 1);
    sys_exit(127);
}

/* ========================================================================
 * DSO STRUCTURE
 * ======================================================================== */

struct dso {
    unsigned char *base;
    unsigned char *map;
    size_t map_len;
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    int phnum;
    size_t phentsize;

    Elf64_Dyn *dyn;
    Elf64_Sym *symtab;
    const char *strtab;
    size_t strsz;
    size_t syment;

    uint32_t *hashtab;
    uint32_t *ghashtab;

    Elf64_Rela *rela;
    size_t rela_cnt;
    Elf64_Rela *rel;
    size_t rel_cnt;
    Elf64_Rela *jmprel;
    size_t jmprel_cnt;
    int pltrel;

    void (**init_array)(void);
    size_t init_array_cnt;
    void (**fini_array)(void);
    size_t fini_array_cnt;
    void (*init)(void);
    void (*fini)(void);

    size_t tls_image;
    size_t tls_len;
    size_t tls_size;
    size_t tls_align;
    size_t tls_offset;
    size_t tls_id;

    char *name;
    char *shortname;
    struct dso *next;
    struct dso *prev;
    struct dso **deps;
    size_t ndeps;
    struct dso *syms_next;

    char relocated;
    char constructed;
    char kernel_mapped;
    char mark;
};

/* Global state */
static struct dso *head, *tail, *syms_tail;
static struct dso ldso;
static struct dso app;
static size_t page_size;
static int runtime;
static int ldd_mode;
static char *env_path;
static char *env_preload;
static size_t tls_cnt;
static size_t tls_offset;
static size_t tls_align = 16;

/* ========================================================================
 * MEMORY HELPERS
 * ======================================================================== */

static inline uintptr_t align_up(uintptr_t val, uintptr_t align) {
    return (val + align - 1) & ~(align - 1);
}

static inline uintptr_t align_down(uintptr_t val, uintptr_t align) {
    return val & ~(align - 1);
}

/* Simple bump allocator */
static char *alloc_buf;
static size_t alloc_used;
static size_t alloc_size;

static void alloc_init(void) {
    alloc_size = 65536;
    alloc_buf = sys_mmap(0, alloc_size, PROT_READ|PROT_WRITE,
                          MAP_ANON|MAP_PRIVATE, -1, 0);
    if (alloc_buf == (void *)-1) ld_fatal("alloc_init: mmap failed");
    alloc_used = 0;
}

static void *alloc(size_t n) {
    n = align_up(n, 16);
    if (alloc_used + n > alloc_size) ld_fatal("alloc: out of memory");
    void *p = alloc_buf + alloc_used;
    alloc_used += n;
    return p;
}

/* ========================================================================
 * ELF HASH FUNCTIONS (from min-dl + musl)
 * ======================================================================== */

static unsigned long elf_hash(const char *name) {
    unsigned long h = 0, g;
    while (*name) {
        h = (h << 4) + (unsigned char)*name++;
        g = h & 0xf0000000;
        if (g) h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

static uint32_t gnu_hash_func(const char *s0) {
    const unsigned char *s = (void *)s0;
    uint32_t h = 5381;
    while (*s) h = (h << 5) + h + *s++;
    return h;
}

/* ========================================================================
 * DYNAMIC SECTION PARSING
 * ======================================================================== */

static size_t get_dynamic_val(Elf64_Dyn *dyn, int tag) {
    for (; dyn->d_tag != DT_NULL; dyn++)
        if (dyn->d_tag == tag) return dyn->d_un.d_val;
    return 0;
}

static void *get_dynamic_ptr(Elf64_Dyn *dyn, int tag, unsigned char *base) {
    for (; dyn->d_tag != DT_NULL; dyn++)
        if (dyn->d_tag == tag) return base + dyn->d_un.d_ptr;
    return 0;
}

static void decode_dyn(struct dso *p) {
    Elf64_Dyn *dyn = p->dyn;
    if (!dyn) return;

    p->symtab    = get_dynamic_ptr(dyn, DT_SYMTAB, p->base);
    p->strtab    = get_dynamic_ptr(dyn, DT_STRTAB, p->base);
    p->strsz     = get_dynamic_val(dyn, DT_STRSZ);
    p->syment    = get_dynamic_val(dyn, DT_SYMENT);
    if (!p->syment) p->syment = sizeof(Elf64_Sym);

    p->hashtab   = get_dynamic_ptr(dyn, DT_HASH, p->base);
    p->ghashtab  = get_dynamic_ptr(dyn, DT_GNU_HASH, p->base);

    size_t rela_ptr = get_dynamic_val(dyn, DT_RELA);
    size_t relasz   = get_dynamic_val(dyn, DT_RELASZ);
    if (rela_ptr) {
        p->rela = (Elf64_Rela *)(p->base + rela_ptr);
        p->rela_cnt = relasz / sizeof(Elf64_Rela);
    }

    size_t rel_ptr = get_dynamic_val(dyn, DT_REL);
    size_t relsz   = get_dynamic_val(dyn, DT_RELSZ);
    if (rel_ptr) {
        p->rel = (Elf64_Rela *)(p->base + rel_ptr);
        p->rel_cnt = relsz / sizeof(Elf64_Rela);
    }

    size_t jmprel_ptr = get_dynamic_val(dyn, DT_JMPREL);
    size_t pltrelsz   = get_dynamic_val(dyn, DT_PLTRELSZ);
    p->pltrel         = get_dynamic_val(dyn, DT_PLTREL);
    if (!p->pltrel) p->pltrel = DT_RELA;
    if (jmprel_ptr) {
        p->jmprel = (Elf64_Rela *)(p->base + jmprel_ptr);
        p->jmprel_cnt = pltrelsz / sizeof(Elf64_Rela);
    }

    size_t init_arr    = get_dynamic_val(dyn, DT_INIT_ARRAY);
    size_t init_arrsz  = get_dynamic_val(dyn, DT_INIT_ARRAYSZ);
    size_t fini_arr    = get_dynamic_val(dyn, DT_FINI_ARRAY);
    size_t fini_arrsz  = get_dynamic_val(dyn, DT_FINI_ARRAYSZ);
    size_t init_addr   = get_dynamic_val(dyn, DT_INIT);
    size_t fini_addr   = get_dynamic_val(dyn, DT_FINI);

    if (init_arr) {
        p->init_array = (void (**)(void))(p->base + init_arr);
        p->init_array_cnt = init_arrsz / sizeof(void *);
    }
    if (fini_arr) {
        p->fini_array = (void (**)(void))(p->base + fini_arr);
        p->fini_array_cnt = fini_arrsz / sizeof(void *);
    }
    if (init_addr) p->init = (void (*)(void))(p->base + init_addr);
    if (fini_addr) p->fini = (void (*)(void))(p->base + fini_addr);

    int i;
    for (i = 0; i < p->phnum; i++) {
        if (p->phdr[i].p_type == PT_TLS) {
            p->tls_image  = (size_t)(p->base + p->phdr[i].p_vaddr);
            p->tls_len    = p->phdr[i].p_filesz;
            p->tls_size   = p->phdr[i].p_memsz;
            p->tls_align  = p->phdr[i].p_align;
            break;
        }
    }
}

/* ========================================================================
 * SYMBOL LOOKUP
 * ======================================================================== */

struct symdef {
    Elf64_Sym *sym;
    struct dso *dso;
};

static Elf64_Sym *lookup_sysv(struct dso *dso, const char *name, unsigned long h) {
    if (!dso->hashtab) return 0;
    uint32_t nbucket = dso->hashtab[0];
    uint32_t nchain  = dso->hashtab[1];
    uint32_t *buckets = &dso->hashtab[2];
    uint32_t *chains  = &dso->hashtab[2 + nbucket];

    uint32_t i;
    for (i = buckets[h % nbucket]; i; i = chains[i]) {
        if (i >= nchain) return 0;
        Elf64_Sym *sym = &dso->symtab[i];
        if (sym->st_name < dso->strsz &&
            ld_strcmp(dso->strtab + sym->st_name, name) == 0) {
            return sym;
        }
    }
    return 0;
}

static Elf64_Sym *lookup_gnu(struct dso *dso, const char *name, uint32_t h) {
    if (!dso->ghashtab) return 0;
    uint32_t nbuckets = dso->ghashtab[0];
    uint32_t symoffset = dso->ghashtab[1];
    uint32_t bloom_size = dso->ghashtab[2];
    uint32_t bloom_shift = dso->ghashtab[3];
    size_t *bloom = (void *)&dso->ghashtab[4];
    uint32_t *buckets = (void *)&bloom[bloom_size];
    uint32_t *chains = &buckets[nbuckets];

    size_t word = bloom[(h / (8*sizeof(size_t))) % bloom_size];
    size_t mask = (size_t)1 << (h % (8*sizeof(size_t)));
    if ((word & mask) == 0) return 0;
    mask = (size_t)1 << ((h >> bloom_shift) % (8*sizeof(size_t)));
    if ((word & mask) == 0) return 0;

    uint32_t i = buckets[h % nbuckets];
    if (i < symoffset) return 0;

    uint32_t *hashval = &chains[i - symoffset];
    for (h |= 1; ; i++) {
        uint32_t h2 = *hashval++;
        if ((h == (h2 | 1)) &&
            ld_strcmp(dso->strtab + dso->symtab[i].st_name, name) == 0) {
            return &dso->symtab[i];
        }
        if (h2 & 1) break;
    }
    return 0;
}

static Elf64_Sym *find_sym_in_dso(struct dso *dso, const char *name) {
    uint32_t gh = gnu_hash_func(name);
    Elf64_Sym *sym = lookup_gnu(dso, name, gh);
    if (!sym && dso->hashtab) {
        unsigned long h = elf_hash(name);
        sym = lookup_sysv(dso, name, h);
    }
    if (!sym) return 0;
    if (sym->st_shndx == SHN_UNDEF) return 0;
    int type = ELF64_ST_TYPE(sym->st_info);
    if (type != STT_NOTYPE && type != STT_OBJECT &&
        type != STT_FUNC && type != STT_COMMON) return 0;
    return sym;
}

static struct symdef find_sym_global(const char *name) {
    struct dso *p;
    struct symdef def = { 0, 0 };
    for (p = syms_tail; p; p = p->syms_next) {
        Elf64_Sym *sym = find_sym_in_dso(p, name);
        if (sym) {
            def.sym = sym;
            def.dso = p;
            return def;
        }
    }
    return def;
}

/* ========================================================================
 * RELOCATIONS
 * ======================================================================== */

static void do_rela(struct dso *dso, Elf64_Rela *rel, size_t cnt, int is_plt) {
    size_t i;
    for (i = 0; i < cnt; i++) {
        Elf64_Rela *r = &rel[i];
        uint64_t *target = (uint64_t *)(dso->base + r->r_offset);
        uint32_t type = ELF64_R_TYPE(r->r_info);
        size_t sym_idx = ELF64_R_SYM(r->r_info);

        switch (type) {
        case R_X86_64_NONE:
            break;

        case R_X86_64_RELATIVE:
            *target = (uint64_t)(dso->base + r->r_addend);
            break;

        case R_X86_64_64: {
            Elf64_Sym *sym = &dso->symtab[sym_idx];
            const char *name = dso->strtab + sym->st_name;
            struct symdef def = find_sym_global(name);
            if (!def.sym) {
                if (ELF64_ST_BIND(sym->st_info) != STB_WEAK)
                    ld_fatal2("undefined symbol: ", name);
                *target = r->r_addend;
            } else {
                *target = (uint64_t)(def.dso->base + def.sym->st_value) + r->r_addend;
            }
            break;
        }

        case R_X86_64_PC32: {
            Elf64_Sym *sym = &dso->symtab[sym_idx];
            const char *name = dso->strtab + sym->st_name;
            struct symdef def = find_sym_global(name);
            if (!def.sym) {
                if (ELF64_ST_BIND(sym->st_info) != STB_WEAK)
                    ld_fatal2("undefined symbol: ", name);
                *(uint32_t *)target = (uint32_t)(r->r_addend - (uint64_t)target);
            } else {
                uint64_t val = (uint64_t)(def.dso->base + def.sym->st_value) + r->r_addend;
                *(uint32_t *)target = (uint32_t)(val - (uint64_t)target);
            }
            break;
        }

        case R_X86_64_GLOB_DAT:
        case R_X86_64_JUMP_SLOT: {
            Elf64_Sym *sym = &dso->symtab[sym_idx];
            const char *name = dso->strtab + sym->st_name;
            struct symdef def = find_sym_global(name);
            if (!def.sym) {
                if (ELF64_ST_BIND(sym->st_info) != STB_WEAK)
                    ld_fatal2("undefined symbol: ", name);
                *target = 0;
            } else {
                *target = (uint64_t)(def.dso->base + def.sym->st_value);
            }
            break;
        }

        case R_X86_64_COPY: {
            Elf64_Sym *sym = &dso->symtab[sym_idx];
            const char *name = dso->strtab + sym->st_name;
            struct symdef def = find_sym_global(name);
            if (!def.sym) ld_fatal2("undefined symbol for COPY: ", name);
            ld_memcpy(target, (void *)(def.dso->base + def.sym->st_value), sym->st_size);
            break;
        }

        default:
            break;
        }
    }
}

static void do_relocs(struct dso *dso) {
    if (dso->relocated) return;
    if (dso->rela_cnt)
        do_rela(dso, dso->rela, dso->rela_cnt, 0);
    if (dso->jmprel_cnt)
        do_rela(dso, dso->jmprel, dso->jmprel_cnt, 1);
    dso->relocated = 1;
}

/* ========================================================================
 * ELF LOADING
 * ======================================================================== */

static void handle_bss(const Elf64_Phdr *ph, unsigned char *base, size_t pagesz) {
    if (ph->p_memsz > ph->p_filesz) {
        uint64_t file_end = (uint64_t)(base + ph->p_vaddr + ph->p_filesz);
        uint64_t mem_end  = (uint64_t)(base + ph->p_vaddr + ph->p_memsz);
        uint64_t file_page_end = align_up(file_end, pagesz);
        uint64_t mem_page_end  = align_up(mem_end, pagesz);

        if (file_page_end > file_end) {
            size_t zero_len = file_page_end - file_end;
            if (zero_len > mem_end - file_end) zero_len = mem_end - file_end;
            ld_memset((void *)file_end, 0, zero_len);
        }

        if (mem_page_end > file_page_end) {
            int prot = 0;
            if (ph->p_flags & PF_R) prot |= PROT_READ;
            if (ph->p_flags & PF_W) prot |= PROT_WRITE;
            if (ph->p_flags & PF_X) prot |= PROT_EXEC;
            void *r = sys_mmap((void *)file_page_end, mem_page_end - file_page_end,
                                prot, MAP_ANON|MAP_PRIVATE|MAP_FIXED, -1, 0);
            if (r == (void *)-1) ld_fatal("handle_bss: mmap failed");
        }
    }
}

static int prot_from_phdr(const Elf64_Phdr *ph) {
    int prot = 0;
    if (ph->p_flags & PF_R) prot |= PROT_READ;
    if (ph->p_flags & PF_W) prot |= PROT_WRITE;
    if (ph->p_flags & PF_X) prot |= PROT_EXEC;
    return prot;
}

static struct dso *load_library_fd(int fd, const char *name, int kernel_mapped) {
    Elf64_Ehdr eh;
    ssize_t n = sys_read(fd, &eh, sizeof(eh));
    if (n != sizeof(eh)) return 0;

    if (eh.e_ident[0] != ELFMAG0 || eh.e_ident[1] != ELFMAG1 ||
        eh.e_ident[2] != ELFMAG2 || eh.e_ident[3] != ELFMAG3 ||
        eh.e_ident[4] != ELFCLASS64 || eh.e_ident[5] != ELFDATA2LSB ||
        eh.e_machine != EM_X86_64) {
        return 0;
    }

    size_t phsize = eh.e_phnum * eh.e_phentsize;
    Elf64_Phdr *ph = alloc(phsize);
    sys_lseek(fd, eh.e_phoff, SEEK_SET);
    n = sys_read(fd, ph, phsize);
    if ((size_t)n != phsize) return 0;

    uint64_t min_vaddr = (uint64_t)-1;
    uint64_t max_vaddr = 0;
    int has_load = 0;
    int i;
    for (i = 0; i < eh.e_phnum; i++) {
        if (ph[i].p_type == PT_LOAD) {
            has_load = 1;
            if (ph[i].p_vaddr < min_vaddr) min_vaddr = ph[i].p_vaddr;
            if (ph[i].p_vaddr + ph[i].p_memsz > max_vaddr)
                max_vaddr = ph[i].p_vaddr + ph[i].p_memsz;
        }
    }
    if (!has_load) return 0;

    min_vaddr = align_down(min_vaddr, page_size);
    max_vaddr = align_up(max_vaddr, page_size);
    size_t map_len = max_vaddr - min_vaddr;

    unsigned char *base;
    if (kernel_mapped) {
        base = 0;
    } else {
        base = sys_mmap((void *)min_vaddr, map_len, PROT_NONE,
                         MAP_PRIVATE|MAP_ANON, -1, 0);
        if (base == (void *)-1) return 0;
    }

    for (i = 0; i < eh.e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD) continue;

        uint64_t seg_start = ph[i].p_vaddr;
        uint64_t seg_mem_end  = seg_start + ph[i].p_memsz;
        uint64_t map_start = align_down(seg_start, page_size);
        uint64_t map_offset = ph[i].p_offset - (seg_start - map_start);
        size_t map_seg_len = align_up(seg_mem_end, page_size) - map_start;

        int prot = prot_from_phdr(&ph[i]);
        void *dest = base + map_start;

        void *r = sys_mmap(dest, map_seg_len, prot,
                            MAP_PRIVATE|MAP_FIXED, fd, map_offset);
        if (r == (void *)-1) {
            sys_munmap(base, map_len);
            return 0;
        }

        handle_bss(&ph[i], base, page_size);
    }

    Elf64_Dyn *dyn = 0;
    for (i = 0; i < eh.e_phnum; i++) {
        if (ph[i].p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn *)(base + ph[i].p_vaddr);
            break;
        }
    }

    struct dso *d = alloc(sizeof(struct dso));
    ld_memset(d, 0, sizeof(*d));

    d->base = base;
    d->map = base;
    d->map_len = map_len;
    d->ehdr = (Elf64_Ehdr *)base;
    d->phdr = ph;
    d->phnum = eh.e_phnum;
    d->phentsize = eh.e_phentsize;
    d->dyn = dyn;
    d->kernel_mapped = kernel_mapped;

    size_t namelen = ld_strlen(name);
    d->name = alloc(namelen + 1);
    ld_memcpy(d->name, name, namelen + 1);

    char *slash = d->name;
    while (*slash) slash++;
    while (slash > d->name && slash[-1] != '/') slash--;
    d->shortname = slash;

    decode_dyn(d);

    if (tail) {
        tail->next = d;
        d->prev = tail;
        tail = d;
    } else {
        head = tail = d;
    }

    return d;
}

static struct dso *load_library(const char *name, struct dso *needed_by) {
    char path[PATH_MAX];
    int fd = -1;
    (void)needed_by;

    if (name[0] == '/' || (name[0] == '.' && name[1] == '/')) {
        fd = sys_open(name, O_RDONLY|O_CLOEXEC);
    } else {
        if (env_path) {
            const char *p = env_path;
            while (*p) {
                while (*p == ':' || ld_isspace(*p)) p++;
                const char *start = p;
                while (*p && *p != ':' && !ld_isspace(*p)) p++;
                size_t dirlen = p - start;
                if (dirlen > 0 && dirlen < PATH_MAX - 2 - ld_strlen(name)) {
                    ld_memcpy(path, start, dirlen);
                    path[dirlen] = '/';
                    ld_memcpy(path + dirlen + 1, name, ld_strlen(name) + 1);
                    fd = sys_open(path, O_RDONLY|O_CLOEXEC);
                    if (fd >= 0) break;
                }
            }
        }
        if (fd < 0) {
            const char *paths[] = { "/lib/", "/usr/lib/", "/usr/local/lib/", 0 };
            int i;
            for (i = 0; paths[i]; i++) {
                size_t plen = ld_strlen(paths[i]);
                ld_memcpy(path, paths[i], plen);
                ld_memcpy(path + plen, name, ld_strlen(name) + 1);
                fd = sys_open(path, O_RDONLY|O_CLOEXEC);
                if (fd >= 0) break;
            }
        }
    }

    if (fd < 0) return 0;

    struct dso *d = load_library_fd(fd, name, 0);
    sys_close(fd);
    return d;
}

/* ========================================================================
 * DEPENDENCY RESOLUTION
 * ======================================================================== */

static void load_deps(struct dso *d) {
    if (d->deps) return;

    size_t ndeps = 0;
    Elf64_Dyn *p;
    for (p = d->dyn; p->d_tag != DT_NULL; p++)
        if (p->d_tag == DT_NEEDED) ndeps++;

    if (ndeps == 0) {
        d->deps = 0;
        d->ndeps = 0;
        return;
    }

    d->deps = alloc((ndeps + 1) * sizeof(struct dso *));
    size_t i = 0;
    for (p = d->dyn; p->d_tag != DT_NULL; p++) {
        if (p->d_tag == DT_NEEDED) {
            const char *libname = d->strtab + p->d_un.d_val;
            struct dso *dep = load_library(libname, d);
            if (!dep) {
                sys_write(2, "[ldso] cannot load ", 19);
                sys_write(2, libname, ld_strlen(libname));
                sys_write(2, "\n", 1);
                sys_exit(127);
            }
            d->deps[i++] = dep;
            load_deps(dep);
        }
    }
    d->deps[i] = 0;
    d->ndeps = i;
}

/* ========================================================================
 * CONSTRUCTOR / DESTRUCTOR ORDERING
 * ======================================================================== */

static struct dso **build_ctor_queue(struct dso *root, size_t *out_cnt) {
    size_t total = 0;
    struct dso *p;
    for (p = head; p; p = p->next) total++;

    struct dso **queue = alloc((total + 1) * sizeof(struct dso *));
    struct dso **stack = alloc(total * sizeof(struct dso *));
    size_t qpos = 0, spos = 0;

    for (p = head; p; p = p->next) p->mark = 0;

    stack[spos++] = root;
    root->mark = 1;

    while (spos > 0) {
        struct dso *cur = stack[--spos];
        queue[qpos++] = cur;
        if (cur->deps) {
            size_t i;
            for (i = 0; cur->deps[i]; i++) {
                if (!cur->deps[i]->mark) {
                    cur->deps[i]->mark = 1;
                    stack[spos++] = cur->deps[i];
                }
            }
        }
    }

    for (p = head; p; p = p->next) p->mark = 0;

    queue[qpos] = 0;
    *out_cnt = qpos;
    return queue;
}

static void run_init(struct dso *d) {
    size_t i;
    if (d->constructed) return;
    d->constructed = 1;

    if (d->init) d->init();
    for (i = 0; i < d->init_array_cnt; i++) {
        if (d->init_array[i]) d->init_array[i]();
    }
}

/* ========================================================================
 * ENVIRONMENT PARSING
 * ======================================================================== */

static char *ld_getenv(const char *name, char **envp) {
    size_t namelen = ld_strlen(name);
    char **e;
    for (e = envp; *e; e++) {
        if (ld_strncmp(*e, name, namelen) == 0 && (*e)[namelen] == '=')
            return *e + namelen + 1;
    }
    return 0;
}

/* ========================================================================
 * SELF-RELOCATION (Bootstrap)
 * ======================================================================== */

static void self_relocate(uintptr_t base, Elf64_Dyn *dynv) {
    uintptr_t rela_ptr = 0, relasz = 0;
    Elf64_Dyn *d;
    for (d = dynv; d->d_tag != DT_NULL; d++) {
        if (d->d_tag == DT_RELA) rela_ptr = d->d_un.d_ptr;
        if (d->d_tag == DT_RELASZ) relasz = d->d_un.d_val;
    }

    if (!rela_ptr || !relasz) return;

    Elf64_Rela *rela = (Elf64_Rela *)(base + rela_ptr);
    size_t count = relasz / sizeof(Elf64_Rela);
    size_t i;

    for (i = 0; i < count; i++) {
        uint32_t type = ELF64_R_TYPE(rela[i].r_info);
        uint64_t *target = (uint64_t *)(base + rela[i].r_offset);

        switch (type) {
        case R_X86_64_RELATIVE:
            *target = base + rela[i].r_addend;
            break;
        case R_X86_64_64:
            *target = base + rela[i].r_addend;
            break;
        case R_X86_64_GLOB_DAT:
        case R_X86_64_JUMP_SLOT:
            *target = base + rela[i].r_addend;
            break;
        }
    }
}

/* ========================================================================
 * MAIN ENTRY POINT
 * ======================================================================== */

void _dl_start_c(uint64_t *sp, Elf64_Dyn *ldso_dynv) {
    uint64_t argc = *sp++;
    char **argv = (char **)sp;
    sp += argc + 1;
    char **envp = (char **)sp;
    while (*sp) sp++;
    sp++;
    uint64_t *auxv = sp;

    uint64_t at_phdr = 0, at_phnum = 0, at_phent = 0;
    uint64_t at_base = 0, at_entry = 0, at_pagesz = 4096;
    uint64_t at_secure = 0;
    int i;

    for (i = 0; auxv[i] != AT_NULL; i += 2) {
        switch (auxv[i]) {
        case AT_PHDR:   at_phdr   = auxv[i+1]; break;
        case AT_PHNUM:  at_phnum  = auxv[i+1]; break;
        case AT_PHENT:  at_phent  = auxv[i+1]; break;
        case AT_BASE:   at_base   = auxv[i+1]; break;
        case AT_ENTRY:  at_entry  = auxv[i+1]; break;
        case AT_PAGESZ: at_pagesz = auxv[i+1]; break;
        case AT_SECURE: at_secure = auxv[i+1]; break;
        }
    }

    page_size = at_pagesz;
    alloc_init();

    /* === SELF-RELOCATION === */
    uintptr_t ldso_base;
    if (at_base) {
        ldso_base = at_base;
    } else {
        Elf64_Ehdr *eh = (Elf64_Ehdr *)(at_phdr - 64);
        ldso_base = (uintptr_t)eh;
    }
    self_relocate(ldso_base, ldso_dynv);

    /* === SETUP LDSO DSO === */
    ld_memset(&ldso, 0, sizeof(ldso));
    ldso.base = (unsigned char *)ldso_base;
    ldso.name = "ld-linux-x86-64.so.2";
    ldso.shortname = "ld-linux-x86-64.so.2";
    ldso.dyn = ldso_dynv;
    ldso.kernel_mapped = 1;

    if (at_phdr) {
        ldso.phdr = (Elf64_Phdr *)at_phdr;
        ldso.phnum = at_phnum;
        ldso.phentsize = at_phent;
    }
    decode_dyn(&ldso);

    head = tail = syms_tail = &ldso;

    /* === PARSE ENVIRONMENT === */
    if (!at_secure) {
        env_path = ld_getenv("LD_LIBRARY_PATH", envp);
        env_preload = ld_getenv("LD_PRELOAD", envp);
    }
    if (ld_strlen(argv[0]) >= 3 &&
        ld_strcmp(argv[0] + ld_strlen(argv[0]) - 3, "ldd") == 0) {
        ldd_mode = 1;
    }

    /* === SETUP MAIN APPLICATION === */
    ld_memset(&app, 0, sizeof(app));

    if (at_phdr && at_phdr != (uint64_t)ldso.phdr) {
        app.phdr = (Elf64_Phdr *)at_phdr;
        app.phnum = at_phnum;
        app.phentsize = at_phent;
        app.kernel_mapped = 1;

        Elf64_Ehdr *app_eh = (Elf64_Ehdr *)(at_phdr - sizeof(Elf64_Ehdr));
        app.base = (unsigned char *)(at_phdr - app_eh->e_phoff);
        app.map = app.base;

        for (i = 0; i < app.phnum; i++) {
            if (app.phdr[i].p_type == PT_DYNAMIC) {
                app.dyn = (Elf64_Dyn *)(app.base + app.phdr[i].p_vaddr);
                break;
            }
        }

        app.name = argv[0];
        app.shortname = argv[0];
        char *slash = app.name;
        while (*slash) slash++;
        while (slash > app.name && slash[-1] != '/') slash--;
        app.shortname = slash;
    } else {
        argv++;
        argc--;
        if (argc < 1) {
            sys_write(2, "Usage: ld.so <program> [args...]\n", 34);
            sys_exit(1);
        }
        int fd = sys_open(argv[0], O_RDONLY);
        if (fd < 0) {
            sys_write(2, "[ldso] cannot open ", 19);
            sys_write(2, argv[0], ld_strlen(argv[0]));
            sys_write(2, "\n", 1);
            sys_exit(127);
        }
        struct dso *d = load_library_fd(fd, argv[0], 0);
        sys_close(fd);
        if (!d) {
            sys_write(2, "[ldso] not a valid ELF: ", 24);
            sys_write(2, argv[0], ld_strlen(argv[0]));
            sys_write(2, "\n", 1);
            sys_exit(127);
        }
        ld_memcpy(&app, d, sizeof(app));
        at_entry = (uint64_t)(app.base + app.ehdr->e_entry);
    }

    decode_dyn(&app);

    tail->next = &app;
    app.prev = tail;
    tail = &app;

    syms_tail->syms_next = &app;
    syms_tail = &app;

    /* === LOAD DEPENDENCIES === */
    load_deps(&app);

    struct dso *p;
    for (p = head; p; p = p->next) {
        if (p != &ldso && p != &app) {
            syms_tail->syms_next = p;
            syms_tail = p;
        }
    }

    /* === HANDLE LD_PRELOAD === */
    if (env_preload) {
        char *s = env_preload;
        while (*s) {
            while (*s == ' ' || *s == ':' || *s == '\t') s++;
            if (!*s) break;
            char *start = s;
            while (*s && *s != ' ' && *s != ':' && *s != '\t') s++;
            char saved = *s;
            *s = 0;
            struct dso *pre = load_library(start, &app);
            *s = saved;
            if (pre) {
                load_deps(pre);
                syms_tail->syms_next = pre;
                syms_tail = pre;
            }
        }
    }

    /* === RELOCATE ALL === */
    for (p = head; p; p = p->next) {
        if (p != &app) do_relocs(p);
    }
    do_relocs(&app);

    /* === RUN CONSTRUCTORS === */
    size_t queue_cnt;
    struct dso **queue = build_ctor_queue(&app, &queue_cnt);
    for (i = (int)queue_cnt; i > 0; i--) {
        run_init(queue[i-1]);
    }

    /* === LDD MODE === */
    if (ldd_mode) {
        for (p = head; p; p = p->next) {
            if (p == &ldso) continue;
            sys_write(1, "\t", 1);
            sys_write(1, p->name, ld_strlen(p->name));
            sys_write(1, " (", 2);
            char hexbuf[32];
            uint64_t addr = (uint64_t)p->base;
            int hpos = 0;
            hexbuf[hpos++] = '0';
            hexbuf[hpos++] = 'x';
            int j;
            for (j = 15; j >= 0; j--) {
                int nibble = (addr >> (j * 4)) & 0xf;
                hexbuf[hpos++] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
            }
            hexbuf[hpos++] = ')';
            hexbuf[hpos++] = '\n';
            sys_write(1, hexbuf, hpos);
        }
        sys_exit(0);
    }

    /* === TRANSFER CONTROL TO PROGRAM === */
    __asm__ volatile ("xor %%rbp, %%rbp" ::: "rbp");

    void (*entry)(void) = (void (*)(void))at_entry;
    __asm__ volatile (
        "xor %%rdx, %%rdx\n\t"
        "jmp *%0"
        :
        : "r"(entry)
        : "rdx", "memory"
    );

    sys_exit(127);
}
