#ifndef DYNM_H
#define DYNM_H

#include <elf.h>
#include <stddef.h>
#include <sys/mman.h>

#define MAX_MODULES 64

#ifndef DT_GNU_HASH
#define DT_GNU_HASH 0x6ffffef5
#endif
#ifndef R_X86_64_COPY
#define R_X86_64_COPY 5
#endif

#define DT_FLAGS        30
#define DT_FLAGS_1      0x6ffffffb
#define DT_VERSYM       0x6ffffff0
#define DT_VERNEED      0x6ffffffe
#define DT_VERNEEDNUM   0x6fffffff
#define DT_RPATH        15
#define DT_RUNPATH      29
#define DT_PLTGOT       3

#define DF_BIND_NOW     0x8
#define DF_1_NOW        0x1
#define DF_1_PIE        0x08000000

#define RTLD_LAZY       0x1
#define RTLD_NOW        0x2
#define RTLD_GLOBAL     0x100
#define RTLD_LOCAL      0x0
#define RTLD_DEFAULT    ((void *) 0)
#define RTLD_NEXT       ((void *) -1L)

#define LD_DEBUG_LIBS     0x1
#define LD_DEBUG_SYMBOLS  0x2
#define LD_DEBUG_RELOC    0x4
#define LD_DEBUG_BINDINGS 0x8

#ifndef Lmid_t
typedef long int Lmid_t;
#endif

typedef struct Module {
    char name[256];
    uintptr_t base;
    size_t map_size;
    Elf64_Dyn *dynamic;
    const char *strtab;
    Elf64_Sym *symtab;
    Elf64_Rela *rela;
    Elf64_Xword relasz;
    Elf64_Xword relaent;
    
    uintptr_t jmprel_raw;
    Elf64_Rela *jmprel;
    Elf64_Xword pltrelsz;
    uintptr_t pltgot;
    
    uint32_t *hash;
    uint32_t *gnu_hash;
    size_t nsyms;
    
    uintptr_t init_func;
    uintptr_t init_array;
    size_t    init_array_sz;
    
    uintptr_t fini_func;
    uintptr_t fini_array;
    size_t    fini_array_sz;
    
    uint16_t *versym;
    Elf64_Verneed *verneed;
    uint16_t verneed_num;
    
    uint32_t flags;
    uint32_t flags1;
    
    const char *rpath;
    const char *runpath;
    
    uintptr_t relro_start;
    size_t relro_size;
    
    int is_preloaded;
    int is_relocated;
    int ref_count;
    int bind_now;
    
    int deps[MAX_MODULES];
    int num_deps;
} Module;

typedef struct MainTargetInfo {
    uintptr_t entry;
    uintptr_t phdr;
    uint16_t phent;
    uint16_t phnum;
    uintptr_t base;
} MainTargetInfo;

void *internal_memset(void *s, int c, size_t n);
void *internal_memmove(void *dest, const void *src, size_t n);

#endif
