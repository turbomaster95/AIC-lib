# =============================================================================
# AIC (Ain't it C) Standard Library - Makefile
# =============================================================================
# A freestanding C standard library implementation for Linux
# Supports: x86_64, aarch64, i386/x86
# =============================================================================

CCACHE_BIN := $(firstword $(wildcard /usr/bin/ccache /bin/ccache $(shell which ccache 2>/dev/null)))

ifeq ($(CCACHE_BIN),)
  CCACHE :=
else
  CCACHE := $(CCACHE_BIN)
endif

ARCH   ?= $(shell uname -m)
PLATF  ?= linux
CC      = clang
HOSTCC  = gcc
AR      = ar
LD      = ld.lld
OBJCOPY = objcopy
IS_ANDROID = $(shell uname -o | grep -qi "android" && echo yes || echo no)

ifeq ($(strip $(CCACHE)),)
  CC  := $(CC)
else
  CC  := $(CCACHE) $(CC)
endif

# Define PIE flags only if needed
ifeq ($(IS_ANDROID),yes)
    PIE_LDFLAGS = -pie
    CC = clang
    # Note: For Android, we also usually need to specify the 
    # page size for the ELF header if it's modern Android.
    PIE_LDFLAGS += -z max-page-size=4096
else
    PIE_LDFLAGS = 
endif

AIC_ROOT    = $(shell pwd)
PREFIX      ?= /usr/local
SYSROOT     ?= $(AIC_ROOT)/sysroot
INCLUDES    = -I$(AIC_ROOT)/include -I$(AIC_ROOT)/src -I$(AIC_ROOT)/src/platforms/$(PLATF)/arch/$(ARCH) -I$(AIC_ROOT)/src/platforms/$(PLATF)/include

# Build flags
ifeq ($(filter $(ARCH),x86 i386),$(ARCH))
    ARCFLAGS += -m32
else
    ARCFLAGS += -m64
endif

CFLAGS      = $(ARCFLAGS) $(INCLUDES) -MMD -MP -nostdinc -nostdlib -ffreestanding -Wall -O2 -fno-stack-protector -fPIC -w
CFLAGS_DBG  = $(ARCFLAGS) $(INCLUDES) -MMD -MP -nostdinc -nostdlib -ffreestanding -Wall -g -fno-stack-protector -fPIC -Wall -Wextra -O0
ASFLAGS     = $(ARCFLAGS) $(INCLUDES) -nostdlib -Wall

# Output paths
BUILD_DIR   = build
LIB_DIR     = $(BUILD_DIR)/lib
OBJ_DIR     = $(BUILD_DIR)/objs
SYSROOT_DIR = $(SYSROOT)

# Library names
LIB_STATIC  = $(LIB_DIR)/libaic.a
LIB_SHARED  = $(LIB_DIR)/libaic.so
LIBC_A      = $(LIB_DIR)/libc.a
LIBC_SO     = $(LIB_DIR)/libc.so

LIBM_A      = $(LIB_DIR)/libm.a
LIBM_SO     = $(LIB_DIR)/libm.so

LDSO        = $(LIB_DIR)/ld.so
LDSO_SRC    = ld/dynlink.c src/platforms/$(PLATF)/pal.c

RAW_SRCS    = $(shell find src -name "*.c" ! -path "*/platforms/*/*")
LIBM_SRCS   = $(filter src/math/%, $(RAW_SRCS))
SRCS        = $(filter-out src/math/%, $(RAW_SRCS))

ARCH_SRCS   = $(shell find -L src/platforms/$(PLATF)/arch/$(ARCH) -name "*.c" 2>/dev/null)
ARCH_SRCS  += src/platforms/$(PLATF)/pal.c
ARCH_ASMS   = $(shell find -L src/platforms/$(PLATF)/arch/$(ARCH) \( -name "*.S" -o -name "*.s" \) ! -name "crt1.s" 2>/dev/null)
ALL_SRCS    = $(SRCS) $(LIBM_SRCS) $(ARCH_SRCS)
STARTUP     = src/platforms/$(PLATF)/arch/$(ARCH)/crt1.s
TEST_SRCS   = $(wildcard tests/*.c)

OBJS        = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)
LIBM_OBJS   = $(LIBM_SRCS:src/%.c=$(OBJ_DIR)/%.o)
ARCH_OBJS   = $(ARCH_SRCS:src/%.c=$(OBJ_DIR)/%.o)
ARCH_ASM_OBJS = $(ARCH_ASMS:src/%.S=$(OBJ_DIR)/%.o)
ALL_OBJS    = $(OBJS) $(ARCH_OBJS) $(ARCH_ASM_OBJS)
STARTUP_OBJ = $(OBJ_DIR)/platforms/$(PLATF)/arch/$(ARCH)/crt1.o
TEST_BINS   = $(TEST_SRCS:tests/%.c=bin/%)
DEPS        = $(ALL_OBJS:.o=.d) $(LIBM_OBJS:.o=.d) $(STARTUP_OBJ:.o=.d)

ifeq ($(ARCH),x86_64)
    # This tells GCC to act as a cross-compiler for x86_64
    TARGET_FLAGS = -target x86_64-linux-gnu -nostdlib

    CFLAGS   += $(TARGET_FLAGS)
    ASFLAGS  += $(TARGET_FLAGS)
    LDFLAGS  += $(TARGET_FLAGS)
    ARCFLAGS += $(TARGET_FLAGS)
endif

# --- Override ---
ifeq ($(filter $(ARCH),x86 i386),$(ARCH))
    # 1. Check for Android specifically within the x86 branch
    ifeq ($(IS_ANDROID),yes)
        PLAT_FLAGS := -target i686-linux-gnu -fPIC
    endif
endif
OCFLAGS =
CFLAGS += $(OCFLAGS) $(PLAT_FLAGS)
ARCFLAGS += $(PLAT_FLAGS)
ASFLAGS += $(PLAT_FLAGS)

.PHONY: all
all: dirs $(LIB_STATIC) $(LIB_SHARED) $(LIBC_A) $(LIBC_SO) $(LIBM_A) $(LIBM_SO) $(LDSO) $(TEST_BINS)
	@echo "[AIC] Build complete."

.PHONY: dirs
dirs:
	@mkdir -p $(LIB_DIR) $(OBJ_DIR) $(SYSROOT_DIR)/include $(SYSROOT_DIR)/lib $(SYSROOT_DIR)/usr/include $(SYSROOT_DIR)/usr/lib

$(LIB_STATIC): $(ALL_OBJS) $(STARTUP_OBJ)
	@mkdir -p $(dir $@)
	@echo "[AR] $@"
	@$(AR) rcs $@ $(ALL_OBJS)

$(LIB_SHARED): $(ALL_OBJS) $(STARTUP_OBJ)
	@mkdir -p $(dir $@)
	@echo "[LD] $@ (shared)"
	@$(CC) -nostdlib -shared $(ARCFLAGS) -o $@ $(ALL_OBJS)

$(LIBM_A): $(LIBM_OBJS)
	@mkdir -p $(dir $@)
	@echo "[AR] $@"
	@$(AR) rcs $@ $(LIBM_OBJS)

$(LIBM_SO): $(LIBM_OBJS)
	@mkdir -p $(dir $@)
	@echo "[LD] $@ (shared)"
	@$(CC) -nostdlib -shared $(ARCFLAGS) -o $@ $(LIBM_OBJS)

$(LIBC_A): $(LIB_STATIC)
	@echo "[CP] $@"
	@cp $< $@

$(LIBC_SO): $(LIB_SHARED)
	@echo "[CP] $@"
	@cp $< $@

$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	@$(CC) -fPIC $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/platforms/$(PLATF)/arch/$(ARCH)/%.o: src/platforms/$(PLATF)/arch/$(ARCH)/%.c
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	@$(CC) -fPIC $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/platforms/$(PLATF)/arch/$(ARCH)/crt1.o: src/platforms/$(PLATF)/arch/$(ARCH)/crt1.s
	@mkdir -p $(dir $@)
	@echo "[AS] $<"
	@$(CC) -fPIC $(ASFLAGS) -c $< -o $@

$(OBJ_DIR)/platforms/$(PLATF)/arch/$(ARCH)/%.o: src/platforms/$(PLATF)/arch/$(ARCH)/%.S
	@mkdir -p $(dir $@)
	@echo "[AS] $<"
	@$(CC) -fPIC $(ASFLAGS)  -c $< -o $@

$(OBJ_DIR)/platforms/$(PLATF)/arch/$(ARCH)/%.o: src/platforms/$(PLATF)/arch/$(ARCH)/%.s
	@mkdir -p $(dir $@)
	@echo "[AS] $<"
	@$(CC) -fPIC $(ASFLAGS)  -c $< -o $@

build/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	@echo "[TEST-CC] $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

bin/%: build/tests/%.o $(STARTUP_OBJ) $(LIB_STATIC)
	@mkdir -p bin
	@echo "[LD] $@"
	@$(LD) -static $(PIE_LDFLAGS) --no-dynamic-linker $(STARTUP_OBJ) $< $(LIB_STATIC) -o $@

$(LDSO): $(LDSO_SRC)
	@mkdir -p $(dir $@)
	@echo "[CCLD] $@"
	@echo $(CC) -fPIC -O2 -nostdlib -e _start -shared $(CFLAGS) \
		-fno-plt -fno-stack-protector -mno-red-zone \
		-fvisibility=hidden -Wl,-Bsymbolic \
		$(INCLUDES) \
		$^ -o $@
	@$(CC) -fPIC -O2 -nostdlib -e _start -shared $(CFLAGS) \
		-fno-plt -fno-stack-protector -mno-red-zone \
		-fvisibility=hidden -Wl,-Bsymbolic \
		$(INCLUDES) \
		$^ -o $@

.PHONY: install install-sysroot sysroot
install: install-sysroot
	@echo "[AIC] Installation complete to $(PREFIX)"

sysroot: install-sysroot
	@echo "[AIC] Sysroot generated at $(SYSROOT_DIR)"

install-sysroot: all
	@echo "[INSTALL] Creating sysroot at $(SYSROOT_DIR)..."
	@mkdir -p $(SYSROOT_DIR)/include
	@mkdir -p $(SYSROOT_DIR)/lib
	@mkdir -p $(SYSROOT_DIR)/usr/include
	@mkdir -p $(SYSROOT_DIR)/usr/lib
	@mkdir -p $(SYSROOT_DIR)/bin
	@echo "[INSTALL] Headers..."
	@cp -r include/* $(SYSROOT_DIR)/include/
	@cp -r include/* $(SYSROOT_DIR)/usr/include/
	@echo "[INSTALL] Libraries..."
	@cp $(LIB_STATIC) $(SYSROOT_DIR)/lib/
	@cp $(LIB_SHARED) $(SYSROOT_DIR)/lib/
	@cp $(LIBC_A) $(SYSROOT_DIR)/lib/
	@cp $(LIBC_SO) $(SYSROOT_DIR)/lib/
	@cp $(LIBM_A) $(SYSROOT_DIR)/lib/
	@cp $(LIBM_SO) $(SYSROOT_DIR)/lib/
	@cp $(LDSO) $(SYSROOT_DIR)/lib/
	@cp $(LIB_STATIC) $(SYSROOT_DIR)/usr/lib/
	@cp $(LIB_SHARED) $(SYSROOT_DIR)/usr/lib/
	@cp $(LIBC_A) $(SYSROOT_DIR)/usr/lib/
	@cp $(LIBC_SO) $(SYSROOT_DIR)/usr/lib/
	@cp $(LIBM_A) $(SYSROOT_DIR)/usr/lib/
	@cp $(LIBM_SO) $(SYSROOT_DIR)/usr/lib/
	@echo "[INSTALL] Startup files..."
	@cp $(STARTUP_OBJ) $(SYSROOT_DIR)/lib/crt1.o
	@cp $(STARTUP_OBJ) $(SYSROOT_DIR)/usr/lib/crt1.o
	@echo "[INSTALL] Toolchain scripts..."
	@cp scripts/aic-gcc $(SYSROOT_DIR)/bin/
	@cp scripts/aic.spec $(SYSROOT_DIR)/lib/
	@echo "[AIC] Sysroot ready at: $(SYSROOT_DIR)"

.PHONY: install-system
install-system: all
	@echo "[INSTALL] Installing to $(PREFIX)..."
	@mkdir -p $(PREFIX)/include
	@mkdir -p $(PREFIX)/lib
	@cp -r include/* $(PREFIX)/include/
	@cp $(LIB_STATIC) $(PREFIX)/lib/libaic.a
	@cp $(LIB_SHARED) $(PREFIX)/lib/libaic.so
	@cp $(LIB_STATIC) $(PREFIX)/lib/libc.a
	@cp $(LIB_SHARED) $(PREFIX)/lib/libc.so
	@cp $(LIBM_A) $(PREFIX)/lib/libm.a
	@cp $(LIBM_SO) $(PREFIX)/lib/libm.so
	@cp $(LDSO) $(PREFIX)/lib/ld.so
	@echo "[AIC] Installed to $(PREFIX)"

.PHONY: package
package: all install-sysroot
	@echo "[PACKAGE] Creating AIC distribution package..."
	@mkdir -p $(BUILD_DIR)/package/aic-$(ARCH)-$(PLATF)
	@cp -r $(SYSROOT_DIR)/* $(BUILD_DIR)/package/aic-$(ARCH)-$(PLATF)/
	@cp scripts/aic-cc $(BUILD_DIR)/package/aic-$(ARCH)-$(PLATF)/bin/ 2>/dev/null || true
	@echo "[TAR] Creating aic-$(ARCH)-$(PLATF).tar.gz..."
	@cd $(BUILD_DIR)/package && tar -czf ../aic-$(ARCH)-$(PLATF).tar.gz aic-$(ARCH)-$(PLATF)
	@rm -rf $(BUILD_DIR)/package
	@echo "[AIC] Package created: $(BUILD_DIR)/aic-$(ARCH)-$(PLATF).tar.gz"

T ?= test_hello

.PHONY: run
run: all
	@echo "[AIC-CC] Compiling tests/$(T).c..."
	@mkdir -p bin build/tests
	@$(CC) \
		-nostdinc -nostdlib \
		-I$(AIC_ROOT)/include \
		-c tests/$(T).c -o build/tests/$(T).o
	@$(LD) -static $(PIE_LDFLAGS) --no-dynamic-linker $(STARTUP_OBJ) build/tests/$(T).o $(LIB_STATIC) -o bin/$(T)
	@echo "[AIC] Running bin/$(T):"
	@./bin/$(T)

.PHONY: debug
debug: CFLAGS = $(CFLAGS_DBG)
debug: clean all
	@echo "[AIC] Debug build complete."

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR) bin
	@rm -rf $(SYSROOT_DIR)
	@rm -rf $(BUILD_DIR)
	@echo "[AIC] Fully cleaned."

.PHONY: help
help:
	@echo "============================================================================="
	@echo "  AIC (Ain't it C) Standard Library - Build System"
	@echo "============================================================================="
	@echo ""
	@echo "  Usage: make [target] [options]"
	@echo ""
	@echo "  Build Targets:"
	@echo "    all           Build everything (default)"
	@echo "    debug         Build with debug symbols"
	@echo "    clean         Remove build artifacts"
	@echo ""
	@echo "  Installation Targets:"
	@echo "    sysroot       Generate sysroot at ./sysroot (quick)"
	@echo "    install       Install to sysroot (default: ./sysroot)"
	@echo "    install-system Install system-wide to PREFIX (default: /usr/local)"
	@echo "    package       Create distributable tarball"
	@echo ""
	@echo "  Testing Targets:"
	@echo "    run           Run test suite (or make run T=<testname>)"
	@echo ""
	@echo "  Options:"
	@echo "    ARCH=<arch>   Target architecture (auto-detected)"
	@echo "    PLATF=<platform> Target platform to build for (default linux)"
	@echo "    PREFIX=<path> Installation prefix for install-system"
	@echo "    SYSROOT=<dir> Sysroot directory for install"
	@echo "    T=<name>      Test name for 'make run' (default: test_hello)"
	@echo ""
	@echo "  Compilation (after 'make install'):"
	@echo "    ./sysroot/bin/aic-gcc hello.c -o hello    # Recommended wrapper"
	@echo "    ./scripts/aic-gcc hello.c -o hello        # Same as above"
	@echo "    ./scripts/aic-cc hello.c -o hello         # Alternative wrapper"
	@echo ""
	@echo "    Using gcc directly (manual):"
	@echo "    gcc --sysroot=./sysroot -nostdlib \\"
	@echo "        ./sysroot/usr/lib/crt1.o hello.c \\"
	@echo "        -L./sysroot/usr/lib -laic -o hello"
	@echo ""
	@echo "  Examples:"
	@echo "    make                          # Build everything"
	@echo "    make run T=test_malloc        # Run malloc test"
	@echo "    make install SYSROOT=/opt/aic # Install to custom sysroot"
	@echo "    make package                  # Create distribution package"
	@echo "    make clean && make debug      # Clean debug build"
	@echo ""
	@echo "  Output Files:"
	@echo "    build/lib/libaic.a   - Static library"
	@echo "    build/lib/libaic.so  - Shared library"
	@echo "    build/lib/libc.a     - Alias (libc-compatible)"
	@echo "    build/lib/libc.so    - Alias (libc-compatible)"
	@echo "    build/lib/libm.a     - Static math library"
	@echo "    build/lib/libm.so    - Shared math library"
	@echo "    sysroot/             - Complete sysroot with headers & libs"
	@echo "    build/aic-<arch>-<platform>.tar.gz - Distributable package"
	@echo ""
	@echo "  Available architectures: aarch64, x86_64, i386 (or x86)"
	@echo "  Available platforms: linux"
	@echo ""
	@echo "============================================================================="

-include $(DEPS)

.PHONY: all dirs install install-sysroot install-system package run debug clean distclean help
