#ifndef DYNM_H
#define DYNM_H

#include <elf.h>
#include <stddef.h>

#define MAX_MODULES 32

#ifndef PROT_READ
#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4
#endif

#ifndef MAP_PRIVATE
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#endif

#ifndef DT_GNU_HASH
#define DT_GNU_HASH 0x6ffffef5
#endif

#ifndef R_X86_64_COPY
#define R_X86_64_COPY 5
#endif

typedef struct Module {
    char name[256];
    uintptr_t base;
    Elf64_Dyn *dynamic;
    const char *strtab;
    Elf64_Sym *symtab;
    Elf64_Rela *rela;
    Elf64_Xword relasz;
    Elf64_Xword relaent;
    Elf64_Rela *jmprel;
    Elf64_Xword pltrelsz;
    uint32_t *hash;
    uint32_t *gnu_hash;
    size_t nsyms;
} Module;

typedef struct MainTargetInfo {
    uintptr_t entry;
    uintptr_t phdr;
    uint16_t phent;
    uint16_t phnum;
    uintptr_t base;
} MainTargetInfo;

#endif
