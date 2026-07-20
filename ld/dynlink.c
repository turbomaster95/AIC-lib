#include <elf.h>
#include <internal/pal.h>
#include <stdint.h>
#include <string.h>

static int streq(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *a == *b;
}

static void print_help(void) {
    static const char *msg = 
        "Usage: ld.so [OPTION]... EXECUTABLE-FILE [ARGS...]\n"
        "Dynamic ELF loader and linker for AIC.\n\n"
        "Options:\n"
        "  --help     Display this help message and exit\n"
        "  -v         Output version information\n\n";
    pal_write(1, msg, strlen(msg));
}

static void print_version(void) {
    static const char *msg = "aic-ld 0.1.0\n";
    pal_write(1, msg, strlen(msg));
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
        // ld.so was loaded as an interpreter for another program.
        // at_base is ld.so's load bias. Parse our own ELF header.
        load_bias = at_base;
        Elf64_Ehdr *ehdr = (Elf64_Ehdr *)at_base;
        ph = (Elf64_Phdr *)(at_base + ehdr->e_phoff);
        phent = ehdr->e_phentsize;
        phnum = ehdr->e_phnum;
    } else {
        // ld.so was executed directly.
        if (!at_phdr || !phent || !phnum) return;
        ph = (Elf64_Phdr *)at_phdr;

        // try to find PT_PHDR
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
                        // Standard ELF header size offset (0x40)
                        load_bias = (at_phdr - 0x40) - p->p_vaddr;
                    }
                    break;
                }
            }
        }
    }

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

__attribute__((visibility("hidden")))
void _start_c(uintptr_t *sp) {
    self_relocate(sp);

    int argc = (int)sp[0];
    char **argv = (char **)&sp[1];

    if (argc < 2) {
        pal_write(2, "ld.so: missing target executable\nTry '--help' for usage.\n", 57);
        pal_exit(1);
    }

    const char *arg1 = argv[1];
    if (streq(arg1, "--help")) {
        print_help();
        pal_exit(0);
    }

    if (streq(arg1, "-v")) {
        print_version();
        pal_exit(0);
    }

    pal_exit(0);
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
