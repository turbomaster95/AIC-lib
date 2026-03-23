# =============================================================================
# AIC (Ain't it C) Standard Library - Makefile
# =============================================================================
# A freestanding C standard library implementation for Linux
# Supports: x86_64, aarch64
# =============================================================================

# --- 1. Identity & Architecture Detection ---
ARCH   ?= $(shell uname -m)
CC      = gcc
AR      = ar
LD      = ld.lld
OBJCOPY = objcopy

# --- 2. Path Definitions ---
AIC_ROOT    = $(shell pwd)
PREFIX      ?= /usr/local
SYSROOT     ?= $(AIC_ROOT)/sysroot
INCLUDES    = -I$(AIC_ROOT)/include -I$(AIC_ROOT)/src -I$(AIC_ROOT)/src/arch/$(ARCH)

# Build flags
CFLAGS      = $(INCLUDES) -MMD -MP -nostdlib -ffreestanding -Wall -O2 -fno-stack-protector -fPIC
CFLAGS_DBG  = $(INCLUDES) -MMD -MP -nostdlib -ffreestanding -Wall -g -fno-stack-protector -fPIC
ASFLAGS     = $(INCLUDES) -nostdlib -Wall

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

# TCC Paths
TCC_DIR     = $(AIC_ROOT)/third_party/tinycc
TCC_BIN     = $(TCC_DIR)/tcc

# --- 3. Automatic Source Discovery ---
SRCS        = $(shell find src -name "*.c" ! -path "*/arch/*/*")
ARCH_SRCS   = $(shell find src/arch/$(ARCH) -name "*.c" 2>/dev/null)
ARCH_ASMS   = $(shell find src/arch/$(ARCH) \( -name "*.S" -o -name "*.s" \) ! -name "crt1.s" 2>/dev/null)
ALL_SRCS    = $(SRCS) $(ARCH_SRCS)
STARTUP     = src/arch/$(ARCH)/crt1.s
TEST_SRCS   = $(wildcard tests/*.c)

# --- 4. Object & Binary Mapping ---
OBJS        = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)
ARCH_OBJS   = $(ARCH_SRCS:src/%.c=$(OBJ_DIR)/%.o)
ARCH_ASM_OBJS = $(ARCH_ASMS:src/%.S=$(OBJ_DIR)/%.o)
ALL_OBJS    = $(OBJS) $(ARCH_OBJS) $(ARCH_ASM_OBJS)
STARTUP_OBJ = $(OBJ_DIR)/arch/$(ARCH)/crt1.o
TEST_BINS   = $(TEST_SRCS:tests/%.c=bin/%)
DEPS        = $(ALL_OBJS:.o=.d) $(STARTUP_OBJ:.o=.d)

# --- 5. Default Target ---
.PHONY: all
all: dirs $(LIB_STATIC) $(LIB_SHARED) $(LIBC_A) $(LIBC_SO) $(TCC_BIN) $(TEST_BINS)
	@echo "[AIC] Build complete."

# --- 6. Directory Creation ---
.PHONY: dirs
dirs:
	@mkdir -p $(LIB_DIR) $(OBJ_DIR) $(SYSROOT_DIR)/include $(SYSROOT_DIR)/lib $(SYSROOT_DIR)/usr/include $(SYSROOT_DIR)/usr/lib

# --- 7. TinyCC Build ---
# Note: TCC's runtime (libtcc1.a) needs system headers, so we don't use
# sysroot paths here. The AIC headers are added when TCC compiles user code.
$(TCC_BIN):
	@echo "[AIC] Configuring TinyCC..."
	@cd $(TCC_DIR) && ./configure --cc=$(CC) --prefix=$(AIC_ROOT)/build/tcc
	@echo "[AIC] Compiling TinyCC..."
	@$(MAKE) -C $(TCC_DIR) -j$(shell nproc)

# --- 8. Library Build Rules ---
$(LIB_STATIC): $(ALL_OBJS) $(STARTUP_OBJ)
	@echo "[AR] $@"
	@$(AR) rcs $@ $(ALL_OBJS)

$(LIB_SHARED): $(ALL_OBJS) $(STARTUP_OBJ)
	@echo "[LD] $@ (shared)"
	@$(CC) -shared -o $@ $(ALL_OBJS) $(STARTUP_OBJ)

$(LIBC_A): $(LIB_STATIC)
	@echo "[CP] $@"
	@cp $< $@

$(LIBC_SO): $(LIB_SHARED)
	@echo "[CP] $@"
	@cp $< $@

# --- 9. Object Compilation Rules ---
$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/$(ARCH)/%.o: src/arch/$(ARCH)/%.c
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/$(ARCH)/crt1.o: src/arch/$(ARCH)/crt1.s
	@mkdir -p $(dir $@)
	@echo "[AS] $<"
	@$(CC) $(ASFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/$(ARCH)/%.o: src/arch/$(ARCH)/%.S
	@mkdir -p $(dir $@)
	@echo "[AS] $<"
	@$(CC) $(ASFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/$(ARCH)/%.o: src/arch/$(ARCH)/%.s
	@mkdir -p $(dir $@)
	@echo "[AS] $<"
	@$(CC) $(ASFLAGS) -c $< -o $@

# --- 10. Test Binary Rules ---
build/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	@echo "[TEST-CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

bin/%: build/tests/%.o $(STARTUP_OBJ) $(LIB_STATIC)
	@mkdir -p bin
	@echo "[LD] $@"
	@$(LD) -static -pie --no-dynamic-linker $(STARTUP_OBJ) $< $(LIB_STATIC) -o $@

# --- 11. Sysroot Installation ---
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
	@cp $(LIB_STATIC) $(SYSROOT_DIR)/usr/lib/
	@cp $(LIB_SHARED) $(SYSROOT_DIR)/usr/lib/
	@cp $(LIBC_A) $(SYSROOT_DIR)/usr/lib/
	@cp $(LIBC_SO) $(SYSROOT_DIR)/usr/lib/
	@echo "[INSTALL] Startup files..."
	@cp $(STARTUP_OBJ) $(SYSROOT_DIR)/lib/crt1.o
	@cp $(STARTUP_OBJ) $(SYSROOT_DIR)/usr/lib/crt1.o
	@echo "[INSTALL] Toolchain scripts..."
	@cp scripts/aic-gcc $(SYSROOT_DIR)/bin/
	@cp scripts/aic.spec $(SYSROOT_DIR)/lib/
	@echo "[AIC] Sysroot ready at: $(SYSROOT_DIR)"

# --- 12. System-wide Installation ---
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
	@echo "[AIC] Installed to $(PREFIX)"

# --- 13. Package Creation ---
.PHONY: package
package: all install-sysroot
	@echo "[PACKAGE] Creating AIC distribution package..."
	@mkdir -p $(BUILD_DIR)/package/aic-$(ARCH)
	@cp -r $(SYSROOT_DIR)/* $(BUILD_DIR)/package/aic-$(ARCH)/
	@cp scripts/aic-cc $(BUILD_DIR)/package/aic-$(ARCH)/bin/ 2>/dev/null || true
	@echo "[TAR] Creating aic-$(ARCH).tar.gz..."
	@cd $(BUILD_DIR)/package && tar -czf ../aic-$(ARCH).tar.gz aic-$(ARCH)
	@rm -rf $(BUILD_DIR)/package
	@echo "[AIC] Package created: $(BUILD_DIR)/aic-$(ARCH).tar.gz"

# --- 14. Convenience: Run Test ---
T ?= test_hello

.PHONY: run
run: all
	@echo "[AIC-TCC] Compiling tests/$(T).c..."
	@mkdir -p bin build/tests
	@LD_LIBRARY_PATH=$(TCC_DIR) $(TCC_BIN) -B$(TCC_DIR) \
		-nostdinc -nostdlib \
		-I$(AIC_ROOT)/include \
		-I$(TCC_DIR)/include \
		-c tests/$(T).c -o build/tests/$(T).o
	@$(LD) -static -pie --no-dynamic-linker $(STARTUP_OBJ) build/tests/$(T).o $(LIB_STATIC) -o bin/$(T)
	@echo "[AIC] Running bin/$(T):"
	@./bin/$(T)

# --- 15. Debug Build ---
.PHONY: debug
debug: CFLAGS = $(CFLAGS_DBG)
debug: clean all
	@echo "[AIC] Debug build complete."

# --- 16. Cleanup ---
.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR) bin
	@if [ -d "$(TCC_DIR)" ]; then $(MAKE) -C $(TCC_DIR) clean; fi
	@rm -rf $(SYSROOT_DIR)
	@echo "[AIC] Cleaned."

.PHONY: distclean
distclean: clean
	@rm -rf $(BUILD_DIR)
	@echo "[AIC] Full clean."

# --- 17. Help Target ---
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
	@echo "    distclean     Remove all generated files"
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
	@echo "  Why use the wrapper scripts?"
	@echo "    Unlike complete toolchains (e.g. arm-linux-gnueabi-gcc), AIC is a"
	@echo "    standalone libc. The wrappers automatically handle:"
	@echo "    - Finding crt1.o startup files"
	@echo "    - Linking against libaic (instead of system libc)"
	@echo "    - Setting up include paths"
	@echo "    - Using correct flags (-nostdlib -ffreestanding -static-pie)"
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
	@echo "    sysroot/             - Complete sysroot with headers & libs"
	@echo "    build/aic-<arch>.tar.gz - Distributable package"
	@echo ""
	@echo "============================================================================="

# --- 18. Dependencies ---
-include $(DEPS)

# --- 19. Phony Targets Declaration ---
.PHONY: all dirs install install-sysroot install-system package run debug clean distclean help
