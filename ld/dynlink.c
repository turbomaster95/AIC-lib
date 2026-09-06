/*
 * ld.c - small self-contained x86-64 ELF dynamic linker.
 *
 * Intended for a custom libc / custom userspace.  It intentionally has
 * NO libc dependency and NO TLS dependency.  It uses Linux syscalls
 * directly and performs eager symbol binding.
 *
 * Supported:
 *   ELF64 ET_DYN/ET_EXEC
 *   PT_LOAD / PT_TLS is rejected (this linker has no TLS implementation)
 *   DT_NEEDED, DT_SONAME, DT_RPATH, DT_RUNPATH
 *   DT_SYMTAB/DT_STRTAB
 *   DT_HASH and DT_GNU_HASH
 *   DT_RELA + DT_RELACOUNT
 *   DT_JMPREL / DT_PLTREL(=RELA)
 *   DT_RELR / DT_RELRSZ / DT_RELRENT
 *   DT_INIT/DT_FINI
 *   DT_INIT_ARRAY/DT_FINI_ARRAY
 *   DT_GNU_RELRO
 *   STT_GNU_IFUNC + R_X86_64_IRELATIVE
 *   R_X86_64_{64,PC32,GOT32,PLT32,GLOB_DAT,JUMP_SLOT,RELATIVE,
 *              GOTPCREL,32,32S,16,PC16,8,PC8,IRELATIVE,GOTPCRELX,
 *              REX_GOTPCRELX,SIZE32,SIZE64,GOTOFF64,GOTPC32}
 *
 * Deliberately not implemented:
 *   ELF symbol versioning
 *   TLS relocations / TLS module setup
 *   audit/filter/trace features
 *   ld.so.cache
 *   lazy PLT binding
 *
 * Build with the accompanying Makefile.
 */

#include <stdint.h>
#include <stddef.h>

/* ------------------------------ ELF ------------------------------ */

#define EI_NIDENT 16
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define ELFCLASS64 2
#define ELFDATA2LSB 1
#define EV_CURRENT 1
#define ELFOSABI_SYSV 0

#define ET_EXEC 2
#define ET_DYN  3
#define EM_X86_64 62

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_TLS 7
#define PT_GNU_RELRO 0x6474e552
#define PT_PHDR 6

#define PF_X 1
#define PF_W 2
#define PF_R 4

#define SHN_UNDEF 0
#define SHN_ABS 0xfff1

#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2
#define STB_GNU_UNIQUE 10

#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4
#define STT_COMMON 5
#define STT_TLS 6
#define STT_GNU_IFUNC 10

#define ELF64_ST_BIND(x) ((uint8_t)((x) >> 4))
#define ELF64_ST_TYPE(x) ((uint8_t)((x) & 0xf))
#define ELF64_ST_VISIBILITY(x) ((uint8_t)((x) & 0x3))
#define STV_DEFAULT 0
#define STV_INTERNAL 1
#define STV_HIDDEN 2

#define DT_NULL 0
#define DT_NEEDED 1
#define DT_PLTRELSZ 2
#define DT_PLTGOT 3
#define DT_HASH 4
#define DT_STRTAB 5
#define DT_SYMTAB 6
#define DT_RELA 7
#define DT_RELASZ 8
#define DT_RELAENT 9
#define DT_STRSZ 10
#define DT_SYMENT 11
#define DT_INIT 12
#define DT_FINI 13
#define DT_SONAME 14
#define DT_RPATH 15
#define DT_SYMBOLIC 16
#define DT_REL 17
#define DT_RELSZ 18
#define DT_RELENT 19
#define DT_PLTREL 20
#define DT_DEBUG 21
#define DT_TEXTREL 22
#define DT_JMPREL 23
#define DT_BIND_NOW 24
#define DT_INIT_ARRAY 25
#define DT_FINI_ARRAY 26
#define DT_INIT_ARRAYSZ 27
#define DT_FINI_ARRAYSZ 28
#define DT_RUNPATH 29
#define DT_FLAGS 30
#define DT_GNU_HASH 0x6ffffef5
#define DT_VERSYM 0x6ffffff0
#define DT_RELACOUNT 0x6ffffff9
#define DT_GNU_PRELINKED 0x6ffffdf5
#define DT_GNU_LIBLIST 0x6ffffef9
#define DT_GNU_CONFLICT 0x6ffffef8
#define DT_FLAGS_1 0x6ffffffb
#define DT_VERDEF 0x6ffffffc
#define DT_VERDEFNUM 0x6ffffffd
#define DT_VERNEED 0x6ffffffe
#define DT_VERNEEDNUM 0x6fffffff
#define DT_GNU_RELRO 0x6ffffdf8
#define DT_RELR 36
#define DT_RELRSZ 35
#define DT_RELRENT 37

#define DF_SYMBOLIC 0x2

/* x86-64 relocations */
#define R_X86_64_NONE 0
#define R_X86_64_64 1
#define R_X86_64_PC32 2
#define R_X86_64_GOT32 3
#define R_X86_64_PLT32 4
#define R_X86_64_COPY 5
#define R_X86_64_GLOB_DAT 6
#define R_X86_64_JUMP_SLOT 7
#define R_X86_64_RELATIVE 8
#define R_X86_64_GOTPCREL 9
#define R_X86_64_32 10
#define R_X86_64_32S 11
#define R_X86_64_16 12
#define R_X86_64_PC16 13
#define R_X86_64_8 14
#define R_X86_64_PC8 15
#define R_X86_64_DTPMOD64 16
#define R_X86_64_DTPOFF64 17
#define R_X86_64_TPOFF64 18
#define R_X86_64_TLSGD 19
#define R_X86_64_TLSLD 20
#define R_X86_64_DTPOFF32 21
#define R_X86_64_GOTTPOFF 22
#define R_X86_64_TPOFF32 23
#define R_X86_64_PC64 24
#define R_X86_64_GOTOFF64 25
#define R_X86_64_GOTPC32 26
#define R_X86_64_GOT64 27
#define R_X86_64_GOTPCREL64 28
#define R_X86_64_GOTPC64 29
#define R_X86_64_GOTPLT64 30
#define R_X86_64_PLTOFF64 31
#define R_X86_64_SIZE32 32
#define R_X86_64_SIZE64 33
#define R_X86_64_GOTPC32_TLSDESC 34
#define R_X86_64_TLSDESC_CALL 35
#define R_X86_64_TLSDESC 36
#define R_X86_64_GOTPCRELX 41
#define R_X86_64_REX_GOTPCRELX 42
#define R_X86_64_IRELATIVE 37

/* Linux */
#define AT_NULL 0
#define AT_IGNORE 1
#define AT_EXECFD 2
#define AT_PHDR 3
#define AT_PHENT 4
#define AT_PHNUM 5
#define AT_PAGESZ 6
#define AT_BASE 7
#define AT_FLAGS 8
#define AT_ENTRY 9
#define AT_UID 11
#define AT_EUID 12
#define AT_GID 13
#define AT_EGID 14
#define AT_SECURE 23
#define AT_EXECFN 31

#define AT_FDCWD (-100)
#define SYS_write 1
#define SYS_close 3
#define SYS_mmap 9
#define SYS_mprotect 10
#define SYS_munmap 11
#define SYS_pread64 17
#define SYS_exit 60
#define SYS_exit_group 231
#define SYS_openat 257
#define SYS_readlink 89

#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#define MAP_SHARED 1
#define MAP_PRIVATE 2
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20
#define MAP_FIXED_NOREPLACE 0x100000

#define O_RDONLY 0
#define O_CLOEXEC 02000000

#define PAGE_FALLBACK 4096UL
#define MAX_OBJECTS 512
#define MAX_PHDRS 128
#define MAX_PATH 4096
#define MAX_SEARCH 8192
#define MAX_NEEDED 256

/* ---------------------------- ABI structs ---------------------------- */

typedef struct {
    uint8_t e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

typedef struct {
    int64_t d_tag;
    union { uint64_t d_val; uint64_t d_ptr; } d_un;
} Elf64_Dyn;

typedef struct {
    uint32_t st_name;
    uint8_t st_info;
    uint8_t st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint64_t st_size;
} Elf64_Sym;

typedef struct {
    uint64_t r_offset;
    uint64_t r_info;
    int64_t r_addend;
} Elf64_Rela;

#define ELF64_R_SYM(i) ((uint32_t)((i) >> 32))
#define ELF64_R_TYPE(i) ((uint32_t)(i))
#define ELF64_R_INFO(s,t) ((((uint64_t)(s)) << 32) | ((uint64_t)(t) & 0xffffffffULL))

/* ------------------------------ helpers ------------------------------ */

static inline long sys0(long n) {
    long r;
    __asm__ volatile ("syscall" : "=a"(r) : "a"(n) : "rcx", "r11", "memory");
    return r;
}
static inline long sys1(long n, long a1) {
    long r;
    __asm__ volatile ("syscall" : "=a"(r) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
    return r;
}
static inline long sys2(long n, long a1, long a2) {
    long r;
    __asm__ volatile ("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2) : "rcx", "r11", "memory");
    return r;
}
static inline long sys3(long n, long a1, long a2, long a3) {
    long r;
    __asm__ volatile ("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2), "d"(a3) : "rcx", "r11", "memory");
    return r;
}
static inline long sys4(long n, long a1, long a2, long a3, long a4) {
    long r;
    register long r10 __asm__("r10") = a4;
    __asm__ volatile ("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10) : "rcx", "r11", "memory");
    return r;
}
static inline long sys5(long n, long a1, long a2, long a3, long a4, long a5) {
    long r;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    __asm__ volatile ("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
    return r;
}
static inline long sys6(long n, long a1, long a2, long a3, long a4, long a5, long a6) {
    long r;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    register long r9 __asm__("r9") = a6;
    __asm__ volatile ("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
    return r;
}

static void kexit(int code) {
    sys1(SYS_exit_group, code);
    for (;;) __asm__ volatile ("hlt");
}

static size_t slen(const char *s) {
    size_t n = 0;
    if (!s) return 0;
    while (s[n]) ++n;
    return n;
}
static int scmp(const char *a, const char *b) {
    size_t i = 0;
    if (a == b) return 0;
    if (!a || !b) return a ? 1 : (b ? -1 : 0);
    while (a[i] || b[i]) {
        unsigned char x = (unsigned char)a[i];
        unsigned char y = (unsigned char)b[i];
        if (x != y) return x < y ? -1 : 1;
        ++i;
    }
    return 0;
}
static int sncmp(const char *a, const char *b, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) {
        unsigned char x = (unsigned char)a[i], y = (unsigned char)b[i];
        if (x != y) return x < y ? -1 : 1;
        if (!x || !y) return 0;
    }
    return 0;
}
static void *mcpy(void *dst, const void *src, size_t n) {
    size_t i;
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    for (i = 0; i < n; ++i) d[i] = s[i];
    return dst;
}
static void *mset(void *dst, int c, size_t n) {
    size_t i;
    unsigned char *d = (unsigned char *)dst;
    for (i = 0; i < n; ++i) d[i] = (unsigned char)c;
    return dst;
}
static uintptr_t align_down(uintptr_t x, uintptr_t a) { return x & ~(a - 1); }
static uintptr_t align_up(uintptr_t x, uintptr_t a) { return (x + a - 1) & ~(a - 1); }
static int fits_s32(int64_t x) { return x >= -2147483648LL && x <= 2147483647LL; }

/* ------------------------------ diagnostics ------------------------------ */

static void putstr(const char *s) {
    if (!s) return;
    sys3(SYS_write, 2, (long)s, (long)slen(s));
}
static void puthex(uint64_t x) {
    char b[19];
    static const char h[] = "0123456789abcdef";
    int i;
    b[0] = '0'; b[1] = 'x';
    for (i = 0; i < 16; ++i) b[2 + i] = h[(x >> ((15 - i) * 4)) & 0xf];
    sys3(SYS_write, 2, (long)b, 18);
}
static void fatal(const char *msg) {
    putstr("ld.so: fatal: ");
    putstr(msg);
    putstr("\n");
    kexit(127);
}
static void putdec(uint64_t x) {
    char b[32]; size_t n = 0;
    if (x == 0) { char c = '0'; sys3(SYS_write, 2, (long)&c, 1); return; }
    while (x) { b[n++] = (char)('0' + (x % 10)); x /= 10; }
    while (n) { char c = b[--n]; sys3(SYS_write, 2, (long)&c, 1); }
}
static const char *base_name(const char *p) {
    const char *q = p;
    if (!p) return "";
    while (*p) { if (*p++ == '/') q = p; }
    return q;
}
static void fatal_obj(const char *msg, const char *obj) {
    putstr("ld.so: fatal: ");
    putstr(msg);
    putstr(": ");
    putstr(obj ? obj : "<unknown>");
    putstr("\n");
    kexit(127);
}

/* ------------------------------ state ------------------------------ */

typedef struct Object Object;

typedef struct {
    uintptr_t addr;
    size_t len;
    int flags;
} Segment;

struct Object {
    int used;
    int mapped_by_us;
    int initialized;
    int relocated;
    int init_running;
    int fini_ran;
    int is_main;
    int is_rtld;
    int needed_scanned;
    uint16_t type;
    uintptr_t base;
    uintptr_t entry;
    int fd;
    char path[MAX_PATH];
    char soname[MAX_PATH];

    Elf64_Ehdr eh;
    Elf64_Phdr phdrs[MAX_PHDRS];

    uintptr_t dyn_addr;
    size_t dyn_count;

    const char *strtab;
    size_t strsz;
    const Elf64_Sym *symtab;
    size_t syment;

    const uint32_t *sysv_hash;
    const uint32_t *gnu_hash;

    const Elf64_Rela *rela;
    size_t relasz;
    size_t relaent;
    size_t relacount;

    uintptr_t jmprel;
    size_t pltrelsz;
    int pltrel_is_rela;

    const uintptr_t *relr;
    size_t relrsz;
    size_t relrent;

    uintptr_t init;
    uintptr_t fini;
    uintptr_t init_array;
    size_t init_arraysz;
    uintptr_t fini_array;
    size_t fini_arraysz;

    uintptr_t relro;
    size_t relrosz;
    uintptr_t got;

    uintptr_t rpath;
    uintptr_t runpath;
    uintptr_t flags;
    uintptr_t flags1;
    uintptr_t symbolic;
};

static Object g_objects[MAX_OBJECTS];
static size_t g_object_count;
static uintptr_t g_page = PAGE_FALLBACK;
static char **g_envp;
static const char *g_execfn;
static uintptr_t g_loader_base;
static uintptr_t g_at_secure;
static const char *g_library_path;
static const char *g_preload_path;
static int g_cli_mode;
static int g_list_mode;
static int g_verify_mode;
static uintptr_t g_main_entry;
static uintptr_t g_initial_sp;
static uintptr_t g_exec_phdr;
static uintptr_t g_exec_phnum;
static uintptr_t g_exec_phent;

/* -------------------------- temporary arena -------------------------- */

static unsigned char *g_arena;
static size_t g_arena_size;
static size_t g_arena_used;

static void arena_init(void) {
    if (g_arena) return;
    size_t sz = 4 * 1024 * 1024;
    long p = sys6(SYS_mmap, 0, sz, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p < 0) fatal("cannot allocate linker arena");
    g_arena = (unsigned char *)(uintptr_t)p;
    g_arena_size = sz;
    g_arena_used = 0;
}
static void *arena_alloc(size_t n, size_t align) {
    size_t p;
    arena_init();
    p = (g_arena_used + align - 1) & ~(align - 1);
    if (p > g_arena_size || n > g_arena_size - p) fatal("linker arena exhausted");
    g_arena_used = p + n;
    return g_arena + p;
}

/* ------------------------------ object table ------------------------------ */

static Object *object_new(void) {
    size_t i;
    for (i = 0; i < MAX_OBJECTS; ++i) {
        if (!g_objects[i].used) {
            Object *o = &g_objects[i];
            mset(o, 0, sizeof(*o));
            o->used = 1;
            o->fd = -1;
            if (i >= g_object_count) g_object_count = i + 1;
            return o;
        }
    }
    fatal("too many loaded objects");
    return 0;
}

/* ------------------------------ file access ------------------------------ */

static long xopen(const char *path) {
    return sys4(SYS_openat, AT_FDCWD, (long)path, O_RDONLY | O_CLOEXEC, 0);
}
static long xpread(int fd, void *buf, size_t n, uint64_t off) {
    return sys4(SYS_pread64, fd, (long)buf, n, (long)off);
}
static int read_full(int fd, void *buf, size_t n, uint64_t off) {
    size_t done = 0;
    while (done < n) {
        long r = xpread(fd, (unsigned char *)buf + done, n - done, off + done);
        if (r <= 0) return -1;
        done += (size_t)r;
    }
    return 0;
}

static int copy_path(char *dst, const char *src) {
    size_t n = slen(src);
    if (n >= MAX_PATH) return -1;
    mcpy(dst, src, n + 1);
    return 0;
}
static int path_dir(char *dst, const char *path) {
    size_t n = slen(path);
    size_t i;
    if (!n || n >= MAX_PATH) return -1;
    for (i = n; i > 0; --i) {
        if (path[i - 1] == '/') {
            if (i == 1) {
                dst[0] = '/'; dst[1] = 0; return 0;
            }
            if (i >= MAX_PATH) return -1;
            mcpy(dst, path, i - 1);
            dst[i - 1] = 0;
            return 0;
        }
    }
    dst[0] = '.'; dst[1] = 0;
    return 0;
}

static int build_join(char *out, const char *dir, const char *name) {
    size_t a = slen(dir), b = slen(name);
    if (a + 1 + b + 1 >= MAX_PATH) return -1;
    mcpy(out, dir, a);
    if (a && out[a - 1] != '/') out[a++] = '/';
    mcpy(out + a, name, b);
    out[a + b] = 0;
    return 0;
}

static int build_origin_path(char *out, const char *path, const char *entry) {
    char dir[MAX_PATH];
    const char *p = entry;
    if (path_dir(dir, path) != 0) return -1;
    if (sncmp(p, "$ORIGIN", 7) == 0) {
        size_t d = slen(dir);
        size_t rest = slen(p + 7);
        if (d + rest + 1 >= MAX_PATH) return -1;
        mcpy(out, dir, d);
        mcpy(out + d, p + 7, rest + 1);
        return 0;
    }
    if (sncmp(p, "${ORIGIN}", 9) == 0) {
        size_t d = slen(dir);
        size_t rest = slen(p + 9);
        if (d + rest + 1 >= MAX_PATH) return -1;
        mcpy(out, dir, d);
        mcpy(out + d, p + 9, rest + 1);
        return 0;
    }
    if (slen(p) >= MAX_PATH) return -1;
    mcpy(out, p, slen(p) + 1);
    return 0;
}

static int try_open_file(const char *p, int *fd_out) {
    long fd = xopen(p);
    if (fd < 0) return -1;
    *fd_out = (int)fd;
    return 0;
}

static int path_has_slash(const char *s) {
    size_t i;
    for (i = 0; s[i]; ++i) if (s[i] == '/') return 1;
    return 0;
}

static const char *getenv_raw(const char *name) {
    size_t n = slen(name);
    size_t i;
    if (!g_envp) return 0;
    for (i = 0; g_envp[i]; ++i) {
        const char *e = g_envp[i];
        if (!sncmp(e, name, n) && e[n] == '=') return e + n + 1;
    }
    return 0;
}

static int try_path_list(const char *list, const char *objpath, const char *name, int *fd_out, char *opened_path) {
    char elem[MAX_PATH];
    char candidate[MAX_PATH];
    const char *p = list;
    while (p && *p) {
        size_t n = 0;
        while (p[n] && p[n] != ':') ++n;
        if (n == 0) {
            elem[0] = '.'; elem[1] = 0;
        } else if (n < MAX_PATH) {
            mcpy(elem, p, n); elem[n] = 0;
        } else {
            return -1;
        }
        if (build_origin_path(candidate, objpath, elem) == 0) {
            if (build_join(candidate, candidate, name) == 0 && try_open_file(candidate, fd_out) == 0) {
                copy_path(opened_path, candidate); return 0;
            }
        }
        p += n;
        if (*p == ':') ++p;
        if (!*p) break;
    }
    return -1;
}

static int find_library_file(Object *from, const char *name, int *fd_out, char *opened_path) {
    char cand[MAX_PATH];
    const char *ldlp = (!g_at_secure) ? (g_library_path ? g_library_path : getenv_raw("LD_LIBRARY_PATH")) : 0;
    char main_dir[MAX_PATH];

    if (path_has_slash(name)) {
        if (try_open_file(name, fd_out) == 0) {
            copy_path(opened_path, name);
            return 0;
        }
        return -1;
    }

    /* DT_RPATH applies first when no RUNPATH is present. */
    if (from && from->rpath && !from->runpath) {
        if (try_path_list((const char *)(from->strtab + from->rpath), from->path, name, fd_out, opened_path) == 0)
            return 0;
    }

    if (ldlp && *ldlp) {
        if (try_path_list(ldlp, from ? from->path : g_execfn, name, fd_out, opened_path) == 0)
            return 0;
    }

    if (from && from->runpath) {
        if (try_path_list((const char *)(from->strtab + from->runpath), from->path, name, fd_out, opened_path) == 0)
            return 0;
    }

    /* Main-program directory is useful for custom libcs and is deliberately
       explicit here; this loader is not trying to emulate glibc's secure-mode
       policy. */
    if (from && from->is_main) {
        if (path_dir(main_dir, from->path) == 0) {
            if (build_join(cand, main_dir, name) == 0 && try_open_file(cand, fd_out) == 0) {
                copy_path(opened_path, cand); return 0;
            }
        }
    }

    if (build_join(cand, "/lib64", name) == 0 && try_open_file(cand, fd_out) == 0) {
        copy_path(opened_path, cand); return 0;
    }
    if (build_join(cand, "/usr/lib64", name) == 0 && try_open_file(cand, fd_out) == 0) {
        copy_path(opened_path, cand); return 0;
    }
    if (build_join(cand, "/lib", name) == 0 && try_open_file(cand, fd_out) == 0) {
        copy_path(opened_path, cand); return 0;
    }
    if (build_join(cand, "/usr/lib", name) == 0 && try_open_file(cand, fd_out) == 0) {
        copy_path(opened_path, cand); return 0;
    }
    return -1;
}

/* ------------------------------ ELF load ------------------------------ */

static int validate_ehdr(const Elf64_Ehdr *e) {
    if (e->e_ident[0] != ELFMAG0 || e->e_ident[1] != ELFMAG1 ||
        e->e_ident[2] != ELFMAG2 || e->e_ident[3] != ELFMAG3)
        return -1;
    if (e->e_ident[4] != ELFCLASS64 || e->e_ident[5] != ELFDATA2LSB ||
        e->e_version != EV_CURRENT || e->e_machine != EM_X86_64)
        return -1;
    if (e->e_type != ET_DYN && e->e_type != ET_EXEC) return -1;
    if (e->e_phentsize != sizeof(Elf64_Phdr) || !e->e_phnum || e->e_phnum > MAX_PHDRS) return -1;
    return 0;
}

static int seg_prot(uint32_t f) {
    int p = 0;
    if (f & PF_R) p |= PROT_READ;
    if (f & PF_W) p |= PROT_WRITE;
    if (f & PF_X) p |= PROT_EXEC;
    return p;
}

static int load_object_file(Object *o, int fd, const char *path, int map_now) {
    Elf64_Ehdr eh;
    Elf64_Phdr ph[MAX_PHDRS];
    uintptr_t minv = ~(uintptr_t)0;
    uintptr_t maxv = 0;
    size_t i;
    long span_addr;
    uintptr_t span, base;

    if (read_full(fd, &eh, sizeof(eh), 0) < 0 || validate_ehdr(&eh) < 0)
        return -1;
    if ((uint64_t)eh.e_phnum * sizeof(Elf64_Phdr) > sizeof(ph)) return -1;
    if (read_full(fd, ph, (size_t)eh.e_phnum * sizeof(Elf64_Phdr), eh.e_phoff) < 0)
        return -1;

    for (i = 0; i < eh.e_phnum; ++i) {
        if (ph[i].p_type == PT_TLS) {
            fatal_obj("PT_TLS encountered; TLS is intentionally unsupported", path);
        }
        if (ph[i].p_type != PT_LOAD) continue;
        uintptr_t lo = align_down((uintptr_t)ph[i].p_vaddr, g_page);
        uintptr_t hi = align_up((uintptr_t)(ph[i].p_vaddr + ph[i].p_memsz), g_page);
        if (lo < minv) minv = lo;
        if (hi > maxv) maxv = hi;
        if (ph[i].p_filesz > ph[i].p_memsz) return -1;
        if ((ph[i].p_offset & (g_page - 1)) != (ph[i].p_vaddr & (g_page - 1))) return -1;
    }
    if (minv == ~(uintptr_t)0 || maxv <= minv) return -1;
    span = maxv - minv;

    if (!map_now) {
        o->type = eh.e_type;
        o->eh = eh;
        mcpy(o->phdrs, ph, (size_t)eh.e_phnum * sizeof(Elf64_Phdr));
        return 0;
    }

    if (eh.e_type == ET_EXEC) {
        span_addr = sys6(SYS_mmap, (long)minv, span, PROT_NONE,
                          MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
        if (span_addr < 0) return -1;
        base = 0;
    } else {
        span_addr = sys6(SYS_mmap, 0, span, PROT_NONE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (span_addr < 0) return -1;
        base = (uintptr_t)span_addr - minv;
    }

    for (i = 0; i < eh.e_phnum; ++i) {
        const Elf64_Phdr *p = &ph[i];
        uintptr_t seg_lo, file_lo, file_hi, mem_hi;
        size_t file_map_len, mem_map_len;
        uintptr_t where;
        int prot;
        if (p->p_type != PT_LOAD) continue;
        seg_lo = align_down((uintptr_t)p->p_vaddr, g_page);
        file_lo = align_down((uintptr_t)p->p_offset, g_page);
        file_hi = align_up((uintptr_t)(p->p_offset + p->p_filesz), g_page);
        mem_hi = align_up((uintptr_t)(p->p_vaddr + p->p_memsz), g_page);
        where = base + seg_lo;
        prot = seg_prot(p->p_flags);

        file_map_len = file_hi > file_lo ? (size_t)(file_hi - file_lo) : 0;
        if (file_map_len) {
            long m = sys6(SYS_mmap, (long)where, file_map_len, prot | PROT_WRITE,
                          MAP_PRIVATE | MAP_FIXED, fd, (long)file_lo);
            if (m < 0) return -1;
        }

        /* Tail of last file page is specified as zero; clear only the part
           belonging to the in-memory object. */
        if (p->p_filesz < p->p_memsz) {
            uintptr_t bss_start = base + p->p_vaddr + p->p_filesz;
            uintptr_t file_page_end = align_up(bss_start, g_page);
            uintptr_t zero_end = base + p->p_vaddr + p->p_memsz;
            if (file_page_end > bss_start) {
                mset((void *)bss_start, 0, (size_t)(file_page_end - bss_start));
            }
            if (mem_hi > file_hi) {
                mem_map_len = (size_t)(mem_hi - file_hi);
                if (sys6(SYS_mmap, (long)(base + file_hi), mem_map_len, prot | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0) < 0)
                    return -1;
            }
            (void)zero_end;
        }
    }

    o->type = eh.e_type;
    o->eh = eh;
    o->base = base;
    o->entry = base + eh.e_entry;
    mcpy(o->phdrs, ph, (size_t)eh.e_phnum * sizeof(Elf64_Phdr));
    return 0;
}

static Object *find_loaded_name(const char *name) {
    size_t i;
    for (i = 0; i < g_object_count; ++i) {
        Object *o = &g_objects[i];
        if (!o->used) continue;
        if (o->soname[0] && scmp(o->soname, name) == 0) return o;
        if (scmp(o->path, name) == 0) return o;
    }
    return 0;
}

static Object *find_loaded_soname_or_basename(const char *name) {
    size_t i;
    for (i = 0; i < g_object_count; ++i) {
        Object *o = &g_objects[i];
        const char *p;
        if (!o->used) continue;
        if (o->soname[0] && scmp(o->soname, name) == 0) return o;
        p = o->path;
        while (*p) ++p;
        while (p > o->path && p[-1] != '/') --p;
        if (scmp(p, name) == 0) return o;
    }
    return 0;
}

static void parse_dynamic(Object *o) {
    size_t i;
    if (!o->dyn_addr) return;
    for (i = 0; i < o->dyn_count; ++i) {
        Elf64_Dyn *d = &((Elf64_Dyn *)o->dyn_addr)[i];
        switch (d->d_tag) {
            case DT_NULL: return;
            case DT_STRTAB: o->strtab = (const char *)(o->base + d->d_un.d_ptr); break;
            case DT_PLTGOT: o->got = o->base + d->d_un.d_ptr; break;
            case DT_STRSZ: o->strsz = (size_t)d->d_un.d_val; break;
            case DT_SYMTAB: o->symtab = (const Elf64_Sym *)(o->base + d->d_un.d_ptr); break;
            case DT_SYMENT: o->syment = (size_t)d->d_un.d_val; break;
            case DT_HASH: o->sysv_hash = (const uint32_t *)(o->base + d->d_un.d_ptr); break;
            case DT_GNU_HASH: o->gnu_hash = (const uint32_t *)(o->base + d->d_un.d_ptr); break;
            case DT_RELA: o->rela = (const Elf64_Rela *)(o->base + d->d_un.d_ptr); break;
            case DT_RELASZ: o->relasz = (size_t)d->d_un.d_val; break;
            case DT_RELAENT: o->relaent = (size_t)d->d_un.d_val; break;
            case DT_RELACOUNT: o->relacount = (size_t)d->d_un.d_val; break;
            case DT_JMPREL: o->jmprel = o->base + d->d_un.d_ptr; break;
            case DT_PLTRELSZ: o->pltrelsz = (size_t)d->d_un.d_val; break;
            case DT_PLTREL: o->pltrel_is_rela = (d->d_un.d_val == DT_RELA); break;
            case DT_INIT: o->init = o->base + d->d_un.d_ptr; break;
            case DT_FINI: o->fini = o->base + d->d_un.d_ptr; break;
            case DT_INIT_ARRAY: o->init_array = o->base + d->d_un.d_ptr; break;
            case DT_INIT_ARRAYSZ: o->init_arraysz = (size_t)d->d_un.d_val; break;
            case DT_FINI_ARRAY: o->fini_array = o->base + d->d_un.d_ptr; break;
            case DT_FINI_ARRAYSZ: o->fini_arraysz = (size_t)d->d_un.d_val; break;
            case DT_SONAME:
                if (o->strtab) copy_path(o->soname, o->strtab + d->d_un.d_val);
                break;
            case DT_RPATH: o->rpath = (uintptr_t)d->d_un.d_val; break;
            case DT_RUNPATH: o->runpath = (uintptr_t)d->d_un.d_val; break;
            case DT_FLAGS: o->flags = (uintptr_t)d->d_un.d_val; break;
            case DT_FLAGS_1: o->flags1 = (uintptr_t)d->d_un.d_val; break;
            case DT_SYMBOLIC: o->symbolic = 1; break;
            case DT_RELRSZ: o->relrsz = (size_t)d->d_un.d_val; break;
            case DT_RELRENT: o->relrent = (size_t)d->d_un.d_val; break;
            case DT_RELR: o->relr = (const uintptr_t *)(o->base + d->d_un.d_ptr); break;
            default: break;
        }
    }
}

static void set_protections(Object *o, int add_write) {
    size_t i;
    for (i = 0; i < o->eh.e_phnum; ++i) {
        Elf64_Phdr *p = &o->phdrs[i];
        if (p->p_type != PT_LOAD || !p->p_memsz) continue;
        uintptr_t lo = align_down(o->base + p->p_vaddr, g_page);
        uintptr_t hi = align_up(o->base + p->p_vaddr + p->p_memsz, g_page);
        int prot = seg_prot(p->p_flags);
        if (add_write) prot |= PROT_WRITE;
        if (sys3(SYS_mprotect, (long)lo, (long)(hi - lo), prot) < 0)
            fatal_obj("mprotect failed", o->path);
    }
}

static void apply_relro(Object *o) {
    if (o->relro && o->relrosz) {
        if (sys3(SYS_mprotect, (long)o->relro, (long)o->relrosz, PROT_READ) < 0)
            fatal_obj("DT_GNU_RELRO mprotect failed", o->path);
    }
}

/* ------------------------------ hash ------------------------------ */

static uint32_t elf_hash(const char *name) {
    uint32_t h = 0, g;
    while (*name) {
        h = (h << 4) + (unsigned char)*name++;
        g = h & 0xf0000000U;
        if (g) h ^= g >> 24;
        h &= ~g;
    }
    return h;
}
static uint32_t gnu_hash(const char *name) {
    uint32_t h = 5381;
    unsigned char c;
    while ((c = (unsigned char)*name++)) h = (h << 5) + h + c;
    return h;
}

static size_t sysv_symcount(Object *o) {
    if (!o->sysv_hash) return 0;
    return o->sysv_hash[1];
}

static size_t gnu_symcount(Object *o) {
    if (!o->gnu_hash) return 0;
    const uint32_t *p = o->gnu_hash;
    uint32_t nb = p[0], symoff = p[1], bloom = p[2];
    (void)nb;
    (void)bloom;
    const uintptr_t *bloom_words = (const uintptr_t *)(p + 4);
    const uint32_t *buckets = (const uint32_t *)(bloom_words + bloom);
    const uint32_t *chains = buckets + nb;
    size_t maxidx = symoff;
    uint32_t i;
    for (i = 0; i < nb; ++i) {
        uint32_t b = buckets[i];
        if (b < symoff) continue;
        uint32_t idx = b;
        for (;;) {
            if ((size_t)idx > maxidx) maxidx = idx;
            uint32_t c = chains[idx - symoff];
            if (c & 1) break;
            ++idx;
        }
    }
    return maxidx + 1;
}

static int sym_usable(const Elf64_Sym *s, int allow_weak) {
    uint8_t bind = ELF64_ST_BIND(s->st_info);
    if (s->st_shndx == SHN_UNDEF) return 0;
    if (bind == STB_LOCAL) return 0;
    if (!allow_weak && bind == STB_WEAK) return 0;
    return 1;
}

static const Elf64_Sym *lookup_one(Object *o, const char *name) {
    if (!o || !o->symtab || !o->strtab) return 0;

    if (o->gnu_hash) {
        const uint32_t *p = o->gnu_hash;
        uint32_t nbuckets = p[0], symoffset = p[1], bloom_size = p[2], bloom_shift = p[3];
        const uintptr_t *bloom = (const uintptr_t *)(p + 4);
        const uint32_t *buckets = (const uint32_t *)(bloom + bloom_size);
        const uint32_t *chains = buckets + nbuckets;
        const size_t bits = sizeof(uintptr_t) * 8;
        uint32_t h = gnu_hash(name);
        uintptr_t word = bloom[(h / bits) % bloom_size];
        uintptr_t mask = ((uintptr_t)1 << (h % bits)) |
                         ((uintptr_t)1 << ((h >> bloom_shift) % bits));
        if ((word & mask) != mask) return 0;
        uint32_t idx = buckets[h % nbuckets];
        if (idx < symoffset) return 0;
        for (;;) {
            uint32_t c = chains[idx - symoffset];
            if ((c | 1U) == (h | 1U)) {
                const Elf64_Sym *s = &o->symtab[idx];
                if (sym_usable(s, 1) && scmp(o->strtab + s->st_name, name) == 0)
                    return s;
            }
            if (c & 1U) break;
            ++idx;
        }
        return 0;
    }

    if (o->sysv_hash) {
        uint32_t h = elf_hash(name);
        uint32_t nb = o->sysv_hash[0];
        const uint32_t *buckets = o->sysv_hash + 2;
        const uint32_t *chains = buckets + nb;
        uint32_t idx = buckets[h % nb];
        while (idx) {
            const Elf64_Sym *s = &o->symtab[idx];
            if (sym_usable(s, 1) && scmp(o->strtab + s->st_name, name) == 0)
                return s;
            idx = chains[idx];
        }
    } else {
        size_t n = gnu_symcount(o);
        size_t i;
        for (i = 0; i < n; ++i) {
            const Elf64_Sym *s = &o->symtab[i];
            if (sym_usable(s, 1) && scmp(o->strtab + s->st_name, name) == 0) return s;
        }
    }
    return 0;
}

static uintptr_t symbol_addr(Object *o, const Elf64_Sym *s) {
    uintptr_t a;
    if (!s) return 0;
    if (s->st_shndx == SHN_ABS) a = (uintptr_t)s->st_value;
    else a = o->base + (uintptr_t)s->st_value;
    if (ELF64_ST_TYPE(s->st_info) == STT_GNU_IFUNC) {
        uintptr_t (*resolver)(void) = (uintptr_t (*)(void))a;
        a = resolver();
    }
    return a;
}

static const Elf64_Sym *lookup_global(const char *name, Object *requester, Object **defobj_out, int skip_requester) {
    size_t i;
    const Elf64_Sym *weak = 0;
    Object *weakobj = 0;

    if (requester && (requester->symbolic || (requester->flags & DF_SYMBOLIC))) {
        const Elf64_Sym *s = lookup_one(requester, name);
        if (s) {
            if (defobj_out) *defobj_out = requester;
            return s;
        }
    }

    if (requester && !skip_requester) {
        const Elf64_Sym *s = lookup_one(requester, name);
        if (s && ELF64_ST_VISIBILITY(s->st_other) != STV_HIDDEN &&
            ELF64_ST_VISIBILITY(s->st_other) != STV_INTERNAL) {
            uint8_t bind = ELF64_ST_BIND(s->st_info);
            if (bind == STB_WEAK) { weak = s; weakobj = requester; }
            else { if (defobj_out) *defobj_out = requester; return s; }
        }
    }

    for (i = 0; i < g_object_count; ++i) {
        Object *o = &g_objects[i];
        const Elf64_Sym *s;
        uint8_t vis, bind;
        if (!o->used || o == requester) continue;
        s = lookup_one(o, name);
        if (!s) continue;
        vis = ELF64_ST_VISIBILITY(s->st_other);
        if (vis == STV_HIDDEN || vis == STV_INTERNAL) continue;
        bind = ELF64_ST_BIND(s->st_info);
        if (bind == STB_WEAK) { if (!weak) { weak = s; weakobj = o; } }
        else { if (defobj_out) *defobj_out = o; return s; }
    }
    if (weak) { if (defobj_out) *defobj_out = weakobj; return weak; }
    if (defobj_out) *defobj_out = 0;
    return 0;
}

/* ------------------------------ object metadata ------------------------------ */

static void discover_dynamic(Object *o) {
    size_t i;
    for (i = 0; i < o->eh.e_phnum; ++i) {
        const Elf64_Phdr *p = &o->phdrs[i];
        if (p->p_type == PT_DYNAMIC) {
            o->dyn_addr = o->base + p->p_vaddr;
            o->dyn_count = p->p_memsz / sizeof(Elf64_Dyn);
        } else if (p->p_type == PT_GNU_RELRO) {
            o->relro = o->base + align_down((uintptr_t)p->p_vaddr, g_page);
            o->relrosz = align_up((uintptr_t)p->p_memsz, g_page);
        }
    }
    parse_dynamic(o);

    if (!o->syment) o->syment = sizeof(Elf64_Sym);
    if (o->rela && o->relaent == 0) o->relaent = sizeof(Elf64_Rela);
    if (o->relr && o->relrent == 0) o->relrent = sizeof(uintptr_t);

    if (o->strtab && o->strsz >= 1 && o->strtab[o->strsz - 1] != 0)
        fatal_obj("malformed string table", o->path);
    if (o->symtab && o->syment != sizeof(Elf64_Sym))
        fatal_obj("unsupported DT_SYMENT", o->path);
    if (o->rela && o->relaent != sizeof(Elf64_Rela))
        fatal_obj("unsupported DT_RELAENT", o->path);
    if (o->relr && o->relrent != sizeof(uintptr_t))
        fatal_obj("unsupported DT_RELRENT", o->path);
}

static Object *load_by_fd(Object *from, int fd, const char *path, int is_main) {
    size_t i;
    Object *o = object_new();
    if (load_object_file(o, fd, path, 1) < 0) fatal_obj("cannot load ELF object", path);
    o->fd = fd;
    o->is_main = is_main;
    copy_path(o->path, path);
    discover_dynamic(o);
    if (o->soname[0] == 0) {
        const char *p = o->path;
        while (*p) ++p;
        while (p > o->path && p[-1] != '/') --p;
        copy_path(o->soname, p);
    }
    set_protections(o, 1);
    (void)i;
    return o;
}

static Object *load_main_from_stack(void) {
    Object *o = object_new();
    size_t i;
    if (!g_exec_phdr || !g_exec_phnum) fatal("missing AT_PHDR/AT_PHNUM");
    if (g_exec_phnum > MAX_PHDRS) fatal("too many program headers");

    mset(o, 0, sizeof(*o));
    o->used = 1;
    o->fd = -1;
    o->is_main = 1;
    o->type = ET_DYN;
    o->eh.e_type = ET_DYN;
    o->eh.e_phnum = (uint16_t)g_exec_phnum;
    o->eh.e_phentsize = (uint16_t)g_exec_phent;
    o->base = 0;
    if (g_execfn) copy_path(o->path, g_execfn);
    else { o->path[0] = '<'; o->path[1] = 0; }

    mcpy(o->phdrs, (const void *)g_exec_phdr, g_exec_phnum * sizeof(Elf64_Phdr));

    /* Find the load bias.  AT_PHDR points to the runtime PHDR table. */
    for (i = 0; i < g_exec_phnum; ++i) {
        Elf64_Phdr *p = &o->phdrs[i];
        if (p->p_type == PT_PHDR) {
            o->base = g_exec_phdr - p->p_vaddr;
            break;
        }
    }
    if (!o->base) {
        for (i = 0; i < g_exec_phnum; ++i) {
            Elf64_Phdr *p = &o->phdrs[i];
            if (p->p_type == PT_LOAD && g_exec_phdr >= o->base + p->p_vaddr &&
                g_exec_phdr < o->base + p->p_vaddr + p->p_memsz) {
                o->base = g_exec_phdr - p->p_vaddr;
                break;
            }
        }
    }
    if (o->base) {
        const Elf64_Ehdr *ehp = (const Elf64_Ehdr *)(o->base);
        if (validate_ehdr(ehp) == 0) {
            o->eh = *ehp;
            o->type = ehp->e_type;
        }
    } else {
        /* ET_EXEC has a zero load bias. */
        const Elf64_Ehdr *ehp = (const Elf64_Ehdr *)(uintptr_t)(g_exec_phdr - o->phdrs[0].p_vaddr);
        (void)ehp;
    }
    for (i = 0; i < g_exec_phnum; ++i) {
        if (o->phdrs[i].p_type == PT_LOAD && o->phdrs[i].p_vaddr == 0) {
            const Elf64_Ehdr *ehp = (const Elf64_Ehdr *)(uintptr_t)(o->base + o->phdrs[i].p_vaddr);
            if (validate_ehdr(ehp) == 0) { o->eh = *ehp; o->type = ehp->e_type; }
            break;
        }
    }
    o->entry = g_main_entry;
    discover_dynamic(o);
    /* Main program mappings are already active, but may be read-only now.
       Temporarily making its segments writable lets us apply relocations. */
    set_protections(o, 1);
    return o;
}

static Object *find_or_load_dependency(Object *from, const char *name);

static void load_needed_recursive(Object *o) {
    if (!o || o->needed_scanned || !o->strtab) return;
    o->needed_scanned = 1;
    size_t i;
    Elf64_Dyn *d = (Elf64_Dyn *)o->dyn_addr;
    for (i = 0; i < o->dyn_count; ++i) {
        if (d[i].d_tag == DT_NULL) break;
        if (d[i].d_tag != DT_NEEDED) continue;
        const char *name = o->strtab + d[i].d_un.d_val;
        Object *dep = find_or_load_dependency(o, name);
        if (!dep) fatal_obj("cannot find DT_NEEDED library", name);
        load_needed_recursive(dep);
    }
}

static Object *find_or_load_dependency(Object *from, const char *name) {
    Object *o = find_loaded_soname_or_basename(name);
    if (o) return o;

    char path[MAX_PATH];
    int fd;
    if (find_library_file(from, name, &fd, path) < 0) return 0;
    o = load_by_fd(from, fd, path, 0);
    return o;
}

/* ------------------------------ relocation ------------------------------ */

static uintptr_t got_slot_for_sym(Object *o, uint32_t symidx) {
    size_t i, n;
    const Elf64_Rela *r;
    if (o->rela && o->relasz) {
        size_t n = o->relasz / o->relaent;
        for (i = 0; i < n; ++i) {
            r = &o->rela[i];
            if ((ELF64_R_TYPE(r->r_info) == R_X86_64_GLOB_DAT ||
                 ELF64_R_TYPE(r->r_info) == R_X86_64_JUMP_SLOT) &&
                ELF64_R_SYM(r->r_info) == symidx)
                return o->base + r->r_offset;
        }
    }
    if (o->jmprel && o->pltrelsz && o->pltrel_is_rela) {
        n = o->pltrelsz / sizeof(Elf64_Rela);
        for (i = 0; i < n; ++i) {
            r = &((const Elf64_Rela *)o->jmprel)[i];
            if (ELF64_R_TYPE(r->r_info) == R_X86_64_JUMP_SLOT && ELF64_R_SYM(r->r_info) == symidx)
                return o->base + r->r_offset;
        }
    }
    return 0;
}

static uintptr_t resolve_reloc_symbol(Object *o, uint32_t symidx, int *is_weak) {
    const Elf64_Sym *s;
    Object *def = 0;
    const char *name;
    if (!o->symtab) fatal_obj("relocation needs DT_SYMTAB", o->path);
    s = &o->symtab[symidx];
    if (s->st_shndx != SHN_UNDEF) {
        uint8_t vis = ELF64_ST_VISIBILITY(s->st_other);
        if (vis == STV_HIDDEN || vis == STV_INTERNAL || ELF64_ST_BIND(s->st_info) == STB_LOCAL) {
            if (is_weak) *is_weak = (ELF64_ST_BIND(s->st_info) == STB_WEAK);
            return symbol_addr(o, s);
        }
    }
    name = o->strtab + s->st_name;
    if (!*name) {
        if (is_weak) *is_weak = 1;
        return 0;
    }
    if (is_weak) *is_weak = (ELF64_ST_BIND(s->st_info) == STB_WEAK);
    const Elf64_Sym *found = lookup_global(name, o, &def, 0);
    if (!found) {
        if (ELF64_ST_BIND(s->st_info) == STB_WEAK) return 0;
        putstr("ld.so: unresolved symbol: "); putstr(name); putstr(" in "); putstr(o->path); putstr("\n");
        kexit(127);
    }
    return symbol_addr(def, found);
}

static void apply_relr(Object *o) {
    if (!o->relr || !o->relrsz) return;
    size_t n = o->relrsz / o->relrent;
    size_t i;
    uintptr_t where = 0;
    for (i = 0; i < n; ++i) {
        uintptr_t e = o->relr[i];
        if ((e & 1) == 0) {
            where = o->base + e;
            *(uintptr_t *)where += o->base;
            where += sizeof(uintptr_t);
        } else {
            uintptr_t bits = e >> 1;
            size_t bit;
            for (bit = 0; bit < sizeof(uintptr_t) * 8 - 1; ++bit) {
                if (bits & ((uintptr_t)1 << bit))
                    *(uintptr_t *)(where + bit * sizeof(uintptr_t)) += o->base;
            }
            where += (sizeof(uintptr_t) * 8 - 1) * sizeof(uintptr_t);
        }
    }
}

static void apply_one_rela(Object *o, const Elf64_Rela *r, int from_plt) {
    uint32_t type = ELF64_R_TYPE(r->r_info);
    uint32_t si = ELF64_R_SYM(r->r_info);
    uintptr_t P = o->base + r->r_offset;
    uintptr_t S = 0;
    int weak = 0;
    const Elf64_Sym *sym = 0;
    (void)from_plt;
    if (si && o->symtab) sym = &o->symtab[si];

    switch (type) {
        case R_X86_64_NONE:
            return;
        case R_X86_64_RELATIVE:
            *(uintptr_t *)P = o->base + (uintptr_t)r->r_addend;
            return;
        case R_X86_64_IRELATIVE: {
            uintptr_t resolver_addr = o->base + (uintptr_t)r->r_addend;
            uintptr_t (*resolver)(void) = (uintptr_t (*)(void))resolver_addr;
            *(uintptr_t *)P = resolver();
            return;
        }
        case R_X86_64_COPY: {
            const char *name;
            Object *copy_def = 0;
            const Elf64_Sym *copy_sym;
            size_t n;
            if (!sym) fatal_obj("COPY without symbol", o->path);
            name = o->strtab + sym->st_name;
            copy_sym = lookup_global(name, o, &copy_def, 1);
            if (!copy_sym || !copy_def) fatal_obj("COPY source not found", o->path);
            n = (size_t)sym->st_size;
            if ((size_t)copy_sym->st_size < n) n = (size_t)copy_sym->st_size;
            mcpy((void *)P, (const void *)symbol_addr(copy_def, copy_sym), n);
            return;
        }
        case R_X86_64_64:
        case R_X86_64_PC32:
        case R_X86_64_GOT32:
        case R_X86_64_PLT32:
        case R_X86_64_GLOB_DAT:
        case R_X86_64_JUMP_SLOT:
        case R_X86_64_GOTPCREL:
        case R_X86_64_32:
        case R_X86_64_32S:
        case R_X86_64_16:
        case R_X86_64_PC16:
        case R_X86_64_8:
        case R_X86_64_PC8:
        case R_X86_64_PC64:
        case R_X86_64_GOTOFF64:
        case R_X86_64_GOTPC32:
        case R_X86_64_GOT64:
        case R_X86_64_GOTPCREL64:
        case R_X86_64_GOTPC64:
        case R_X86_64_GOTPLT64:
        case R_X86_64_PLTOFF64:
        case R_X86_64_SIZE32:
        case R_X86_64_SIZE64:
        case R_X86_64_GOTPCRELX:
        case R_X86_64_REX_GOTPCRELX:
            break;
        case R_X86_64_DTPMOD64:
        case R_X86_64_DTPOFF64:
        case R_X86_64_TPOFF64:
        case R_X86_64_TLSGD:
        case R_X86_64_TLSLD:
        case R_X86_64_DTPOFF32:
        case R_X86_64_GOTTPOFF:
        case R_X86_64_TPOFF32:
        case R_X86_64_GOTPC32_TLSDESC:
        case R_X86_64_TLSDESC_CALL:
        case R_X86_64_TLSDESC:
            fatal_obj("TLS relocation encountered, but this ld.so has no TLS", o->path);
            return;
        default:
            putstr("ld.so: unsupported relocation type "); puthex(type);
            putstr(" in "); putstr(o->path); putstr("\n");
            kexit(127);
    }

    if (!si || !sym) fatal_obj("relocation requires symbol", o->path);
    S = resolve_reloc_symbol(o, si, &weak);

    switch (type) {
        case R_X86_64_64:
            *(uint64_t *)P = (uint64_t)(S + (uintptr_t)r->r_addend);
            break;
        case R_X86_64_PC32:
        case R_X86_64_PLT32: {
            int64_t v = (int64_t)(S + (uintptr_t)r->r_addend - P);
            if (!fits_s32(v)) fatal_obj("PC32 relocation overflow", o->path);
            *(uint32_t *)P = (uint32_t)(int32_t)v;
            break;
        }
        case R_X86_64_GOT32: {
            uintptr_t G = got_slot_for_sym(o, si);
            if (!G) G = S;
            *(uint32_t *)P = (uint32_t)(G + (uintptr_t)r->r_addend);
            break;
        }
        case R_X86_64_GLOB_DAT:
        case R_X86_64_JUMP_SLOT:
            *(uint64_t *)P = (uint64_t)S;
            break;
        case R_X86_64_GOTPCREL:
        case R_X86_64_GOTPCRELX:
        case R_X86_64_REX_GOTPCRELX: {
            uintptr_t G = got_slot_for_sym(o, si);
            if (!G) {
                /* Local non-preemptible references can be resolved directly. */
                G = S;
            }
            int64_t v = (int64_t)(G + (uintptr_t)r->r_addend - P);
            if (!fits_s32(v)) fatal_obj("GOTPCREL relocation overflow", o->path);
            *(uint32_t *)P = (uint32_t)(int32_t)v;
            break;
        }
        case R_X86_64_32: {
            uint64_t v = (uint64_t)(S + (uintptr_t)r->r_addend);
            if (v > 0xffffffffULL) fatal_obj("32-bit relocation overflow", o->path);
            *(uint32_t *)P = (uint32_t)v;
            break;
        }
        case R_X86_64_32S: {
            int64_t v = (int64_t)(S + (uintptr_t)r->r_addend);
            if (!fits_s32(v)) fatal_obj("32S relocation overflow", o->path);
            *(int32_t *)P = (int32_t)v;
            break;
        }
        case R_X86_64_16:
            *(uint16_t *)P = (uint16_t)(S + (uintptr_t)r->r_addend);
            break;
        case R_X86_64_PC16:
            *(uint16_t *)P = (uint16_t)(S + (uintptr_t)r->r_addend - P);
            break;
        case R_X86_64_8:
            *(uint8_t *)P = (uint8_t)(S + (uintptr_t)r->r_addend);
            break;
        case R_X86_64_PC8:
            *(uint8_t *)P = (uint8_t)(S + (uintptr_t)r->r_addend - P);
            break;
        case R_X86_64_GOTOFF64:
            *(uint64_t *)P = (uint64_t)(S + (uintptr_t)r->r_addend - o->base);
            break;
        case R_X86_64_GOTPC32: {
            uintptr_t got = o->base + o->base; /* overwritten below if DT_PLTGOT exists */
            /* GOTPC32's intended G is the GOT base. */
            got = o->base + (o->symbolic ? 0 : 0);
            if (o->base && o->jmprel) {
                /* Find DT_PLTGOT by rescanning dynamic section. */
                size_t i;
                Elf64_Dyn *d = (Elf64_Dyn *)o->dyn_addr;
                for (i = 0; i < o->dyn_count; ++i) {
                    if (d[i].d_tag == DT_PLTGOT) { got = o->base + d[i].d_un.d_ptr; break; }
                    if (d[i].d_tag == DT_NULL) break;
                }
            }
            int64_t v = (int64_t)(got + (uintptr_t)r->r_addend - P);
            if (!fits_s32(v)) fatal_obj("GOTPC32 relocation overflow", o->path);
            *(uint32_t *)P = (uint32_t)(int32_t)v;
            break;
        }
        case R_X86_64_SIZE32: {
            uint64_t v = (uint64_t)(S + (uintptr_t)r->r_addend + (sym ? sym->st_size : 0));
            if (v > 0xffffffffULL) fatal_obj("SIZE32 overflow", o->path);
            *(uint32_t *)P = (uint32_t)v;
            break;
        }
        case R_X86_64_SIZE64:
            *(uint64_t *)P = (uint64_t)(S + (uintptr_t)r->r_addend + (sym ? sym->st_size : 0));
            break;
        default:
            fatal_obj("internal relocation dispatch error", o->path);
    }
}

static void relocate_object(Object *o) {
    size_t i;
    if (o->relocated) return;

    /* RELATIVE relocations can be applied before normal symbol resolution. */
    if (o->rela && o->relasz) {
        size_t n = o->relasz / o->relaent;
        size_t first = o->relacount;
        if (first > n) first = n;
        for (i = 0; i < first; ++i) {
            if (ELF64_R_TYPE(o->rela[i].r_info) != R_X86_64_RELATIVE)
                fatal_obj("DT_RELACOUNT prefix is not RELATIVE", o->path);
            apply_one_rela(o, &o->rela[i], 0);
        }
        for (i = first; i < n; ++i) apply_one_rela(o, &o->rela[i], 0);
    }

    if (o->jmprel && o->pltrelsz) {
        if (!o->pltrel_is_rela) fatal_obj("DT_JMPREL is REL; only RELA is supported", o->path);
        size_t n = o->pltrelsz / sizeof(Elf64_Rela);
        const Elf64_Rela *r = (const Elf64_Rela *)o->jmprel;
        for (i = 0; i < n; ++i) apply_one_rela(o, &r[i], 1);
    }

    apply_relr(o);
    o->relocated = 1;
}

/* ------------------------------ constructors/destructors ------------------------------ */

static void call0(uintptr_t fn) {
    if (!fn) return;
    ((void (*)(void))fn)();
}

static void init_object_recursive(Object *o) {
    size_t i;
    Elf64_Dyn *d;
    if (!o || o->initialized) return;
    if (o->init_running) fatal_obj("dependency cycle during constructors", o->path);
    o->init_running = 1;

    d = (Elf64_Dyn *)o->dyn_addr;
    if (d) {
        for (i = 0; i < o->dyn_count; ++i) {
            if (d[i].d_tag == DT_NULL) break;
            if (d[i].d_tag != DT_NEEDED) continue;
            const char *name = o->strtab + d[i].d_un.d_val;
            Object *dep = find_loaded_soname_or_basename(name);
            if (dep) init_object_recursive(dep);
        }
    }

    call0(o->init);
    if (o->init_array && o->init_arraysz) {
        uintptr_t *a = (uintptr_t *)o->init_array;
        size_t n = o->init_arraysz / sizeof(uintptr_t);
        for (i = 0; i < n; ++i) call0(a[i]);
    }
    o->initialized = 1;
    o->init_running = 0;
}

static void run_fini_reverse(void) {
    size_t oi;
    for (oi = g_object_count; oi-- > 0;) {
        Object *o = &g_objects[oi];
        size_t i;
        if (!o->used || !o->initialized || o->fini_ran) continue;
        if (o->fini_array && o->fini_arraysz) {
            uintptr_t *a = (uintptr_t *)o->fini_array;
            size_t n = o->fini_arraysz / sizeof(uintptr_t);
            for (i = n; i-- > 0;) call0(a[i]);
        }
        call0(o->fini);
        o->fini_ran = 1;
    }
}

/* ------------------------------ return / main transfer ------------------------------ */

__attribute__((noreturn, naked, used))
static void jump_to_entry(uintptr_t entry, uintptr_t sp) {
    (void)entry; (void)sp;
    __asm__ volatile (
        "mov %rsi, %rsp\n"
        "xor %rbp, %rbp\n"
        "jmp *%rdi\n"
    );
    __builtin_unreachable();
}

__attribute__((noreturn, naked, used))
void _start(void) {
    __asm__ volatile (
        "mov %rsp, %rdi\n"
        "and $-16, %rsp\n"
        "call rtld_start\n"
        "ud2\n"
    );
}

/* ------------------------------ startup / CLI ------------------------------ */

static void parse_initial_stack(uintptr_t sp) {
    uintptr_t *p = (uintptr_t *)sp;
    uintptr_t argc = p[0];
    char **argv = (char **)&p[1];
    char **env;
    uintptr_t *aux;
    size_t i;

    env = &argv[argc + 1];
    g_envp = env;
    while (*env) ++env;
    aux = (uintptr_t *)(env + 1);
    for (i = 0; ; i += 2) {
        uintptr_t tag = aux[i];
        uintptr_t val = aux[i + 1];
        if (tag == AT_NULL) break;
        switch (tag) {
            case AT_PHDR: g_exec_phdr = val; break;
            case AT_PHENT: g_exec_phent = val; break;
            case AT_PHNUM: g_exec_phnum = val; break;
            case AT_PAGESZ: if (val) g_page = val; break;
            case AT_ENTRY: g_main_entry = val; break;
            case AT_EXECFN: g_execfn = (const char *)val; break;
            case AT_BASE: g_loader_base = val; break;
            case AT_SECURE: g_at_secure = val; break;
            default: break;
        }
    }
    g_initial_sp = sp;
}

static void sanitize_main_path(Object *main) {
    if (main->path[0] == 0 || main->path[0] == '<') {
        if (g_execfn) copy_path(main->path, g_execfn);
    }
}

static void finish_protections(void) {
    size_t i;
    for (i = 0; i < g_object_count; ++i) {
        Object *o = &g_objects[i];
        if (!o->used) continue;
        set_protections(o, 0);
        apply_relro(o);
    }
}


static void usage(void) {
    putstr("Usage: ld.so [options] pathname [args...]\n\n");
    putstr("Options:\n");
    putstr("  --list, --ldd              List dynamic dependencies and resolved paths\n");
    putstr("  --verify                   Verify that pathname is a supported ELF\n");
    putstr("  --library-path PATH        Override LD_LIBRARY_PATH\n");
    putstr("  --preload LIST             Preload colon-separated DSOs\n");
    putstr("  --help                     Show this help\n");
    putstr("  --version                  Show linker version\n");
}

static void version(void) {
    putstr("ld64 x86-64 dynamic linker\n");
    putstr("version 0.2.0\n");
    putstr("freestanding / no TLS / eager binding\n");
}

static int stack_argc(uintptr_t sp) { return (int)((uintptr_t *)sp)[0]; }
static char **stack_argv(uintptr_t sp) { return (char **)(uintptr_t *)((uintptr_t *)sp + 1); }

static void set_exec_auxv(Object *main) {
    uintptr_t *aux = (uintptr_t *)(g_envp + 1);
    size_t i;
    uintptr_t phdr_rt = 0;

    for (i = 0; i < main->eh.e_phnum; ++i) {
        if (main->phdrs[i].p_type == PT_PHDR) {
            phdr_rt = main->base + main->phdrs[i].p_vaddr;
            break;
        }
    }
    if (!phdr_rt) {
        /* The PHDR table is normally inside the first PT_LOAD. */
        for (i = 0; i < main->eh.e_phnum; ++i) {
            const Elf64_Phdr *ph = &main->phdrs[i];
            if (ph->p_type != PT_LOAD) continue;
            if (main->eh.e_phoff >= ph->p_offset &&
                main->eh.e_phoff + (uint64_t)main->eh.e_phnum * sizeof(Elf64_Phdr) <=
                    ph->p_offset + ph->p_filesz) {
                phdr_rt = main->base + ph->p_vaddr + (main->eh.e_phoff - ph->p_offset);
                break;
            }
        }
    }

    for (i = 0; ; i += 2) {
        if (aux[i] == AT_NULL) break;
        switch (aux[i]) {
            case AT_PHDR: aux[i + 1] = phdr_rt; break;
            case AT_PHENT: aux[i + 1] = sizeof(Elf64_Phdr); break;
            case AT_PHNUM: aux[i + 1] = main->eh.e_phnum; break;
            case AT_ENTRY: aux[i + 1] = main->entry; break;
            case AT_BASE: aux[i + 1] = g_loader_base; break;
            case AT_EXECFN: aux[i + 1] = (uintptr_t)((char **)((uintptr_t *)g_initial_sp + 1))[0]; break;
            default: break;
        }
    }
}

static int cli_parse(int argc, char **argv, int *target_index, int *list_mode, int *verify_mode) {
    int i = 1;
    *target_index = -1;
    *list_mode = 0;
    *verify_mode = 0;
    while (i < argc) {
        const char *a = argv[i];
        if (scmp(a, "--") == 0) { ++i; break; }
        if (scmp(a, "--help") == 0 || scmp(a, "-h") == 0) { usage(); kexit(0); }
        if (scmp(a, "--version") == 0) { version(); kexit(0); }
        if (scmp(a, "--list") == 0 || scmp(a, "--ldd") == 0) { *list_mode = 1; ++i; continue; }
        if (scmp(a, "--verify") == 0) { *verify_mode = 1; ++i; continue; }
        if (scmp(a, "--library-path") == 0 || scmp(a, "-L") == 0) {
            if (i + 1 >= argc) fatal("--library-path requires an argument");
            g_library_path = argv[++i]; ++i; continue;
        }
        if (sncmp(a, "--library-path=", 15) == 0) { g_library_path = a + 15; ++i; continue; }
        if (scmp(a, "--preload") == 0) {
            if (i + 1 >= argc) fatal("--preload requires an argument");
            g_preload_path = argv[++i]; ++i; continue;
        }
        if (sncmp(a, "--preload=", 10) == 0) { g_preload_path = a + 10; ++i; continue; }
        if (scmp(a, "--inhibit-cache") == 0 || scmp(a, "--inhibit-rpath") == 0) { ++i; continue; }
        if (scmp(a, "-v") == 0 || scmp(a, "--verbose") == 0) { *list_mode = 1; ++i; continue; }
        if (a[0] == '-' && a[1] != 0) {
            putstr("ld.so: unknown option: "); putstr(a); putstr("\n"); usage(); kexit(2);
        }
        *target_index = i;
        return 0;
    }
    if (i < argc) *target_index = i;
    return 0;
}

static Object *load_cli_main(const char *path) {
    int fd;
    Object *main;
    char opened[MAX_PATH];
    if (try_open_file(path, &fd) < 0) fatal_obj("cannot open program", path);
    main = load_by_fd(0, fd, path, 1);
    copy_path(main->path, path);
    (void)opened;
    return main;
}

static void preload_one(Object *main, const char *name) {
    char token[MAX_PATH], path[MAX_PATH];
    size_t n = 0;
    int fd;
    const char *p = name;
    while (*p) {
        n = 0;
        while (p[n] && p[n] != ':') ++n;
        if (n >= MAX_PATH) fatal("preload name too long");
        mcpy(token, p, n); token[n] = 0;
        if (token[0]) {
            Object *already = find_loaded_soname_or_basename(token);
            if (!already) {
                if (path_has_slash(token)) {
                    if (try_open_file(token, &fd) < 0) fatal_obj("cannot preload", token);
                    copy_path(path, token);
                } else {
                    if (find_library_file(main, token, &fd, path) < 0) fatal_obj("cannot preload", token);
                }
                load_by_fd(main, fd, path, 0);
            }
        }
        p += n;
        if (*p == ':') ++p;
    }
}

static void print_object_line(Object *o) {
    putstr("\t");
    if (o->soname[0]) putstr(o->soname); else putstr(o->path);
    putstr(" => "); putstr(o->path);
    putstr(" ("); puthex(o->base); putstr(")\n");
}

static void print_dependencies(Object *main) {
    size_t i;
    putstr("\t"); putstr(main->path); putstr(" (main)\n");
    for (i = 0; i < g_object_count; ++i) {
        Object *o = &g_objects[i];
        if (!o->used || o == main) continue;
        print_object_line(o);
    }
}

static void run_loader_normal(uintptr_t sp, int argc, char **argv) {
    Object *main;
    size_t i;
    int target_index = -1;
    int from_ldd = (scmp(base_name(argv[0]), "ldd") == 0);

    g_cli_mode = 1;
    cli_parse(argc, argv, &target_index, &g_list_mode, &g_verify_mode);
    if (target_index < 0) { usage(); kexit(2); }

    main = load_cli_main(argv[target_index]);
    if (g_verify_mode) { putstr("Verified: "); putstr(main->path); putstr("\n"); kexit(0); }

    if (g_preload_path && !g_at_secure) preload_one(main, g_preload_path);
    if (!g_at_secure) {
        const char *envpre = getenv_raw("LD_PRELOAD");
        if (envpre && *envpre) preload_one(main, envpre);
    }

    load_needed_recursive(main);
    for (i = 0; i < g_object_count; ++i) if (g_objects[i].used) relocate_object(&g_objects[i]);

    if (g_list_mode || from_ldd) {
        finish_protections();
        print_dependencies(main);
        kexit(0);
    }

    finish_protections();
    init_object_recursive(main);

    /* Replace argv in-place: ld.so ./prog a b -> ./prog a b. */
    {
        int new_argc = argc - target_index;
        uintptr_t *p = (uintptr_t *)sp;
        char **av = (char **)(p + 1);
        int j;
        for (j = 0; j < new_argc; ++j) av[j] = argv[target_index + j];
        av[new_argc] = 0;
        p[0] = (uintptr_t)new_argc;
    }
    set_exec_auxv(main);
    jump_to_entry(main->entry, sp);
}

void rtld_start(uintptr_t sp) {
    int argc;
    char **argv;
    size_t i;
    Object *main;

    parse_initial_stack(sp);
    argc = stack_argc(sp);
    argv = stack_argv(sp);

    /* When invoked as PT_INTERP, Linux provides a nonzero AT_BASE for the
       interpreter. When executed directly, AT_BASE is zero and argv belongs
       to ld.so itself. */
    if (g_loader_base != 0) {
        arena_init();
        if (!g_main_entry) fatal("missing AT_ENTRY");
        main = load_main_from_stack();
        sanitize_main_path(main);
        if (!g_at_secure) {
            const char *pre = getenv_raw("LD_PRELOAD");
            if (pre && *pre) preload_one(main, pre);
        }
        load_needed_recursive(main);
        for (i = 0; i < g_object_count; ++i) if (g_objects[i].used) relocate_object(&g_objects[i]);
        finish_protections();
        init_object_recursive(main);
        jump_to_entry(main->entry, g_initial_sp);
    }

    run_loader_normal(sp, argc, argv);
}
