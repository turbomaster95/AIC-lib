# 1. Identity & Architecture Detection
ARCH   ?= $(shell uname -m)
CC      = gcc
AR      = ar
LD      = ld.lld  # Use LLD directly to avoid the 'false' trap

# 2. Path Definitions
AIC_ROOT = $(shell pwd)
INCLUDES = -I$(AIC_ROOT)/include -I$(AIC_ROOT)/src -I$(AIC_ROOT)/src/arch/$(ARCH)
# Added -fPIC for Termux compatibility
CFLAGS   = $(INCLUDES) -MMD -MP -nostdlib -ffreestanding -Wall -O2 -fno-stack-protector -fPIC
ASFLAGS  = $(INCLUDES) -nostdlib -Wall
LIB_NAME = build/lib/libaic.a

# TCC Paths
TCC_DIR  = $(AIC_ROOT)/third_party/tinycc
TCC_BIN  = $(TCC_DIR)/tcc

# 3. Automatic Source Discovery
SRCS        = $(shell find src -name "*.c")
STARTUP     = src/arch/$(ARCH)/crt1.s
TEST_SRCS   = $(wildcard tests/*.c)

# 4. Object & Binary Mapping
OBJS        = $(SRCS:src/%.c=build/objs/%.o)
STARTUP_OBJ = build/objs/arch/$(ARCH)/crt1.o
TEST_BINS   = $(TEST_SRCS:tests/%.c=bin/%)
DEPS        = $(OBJS:.o=.d) $(STARTUP_OBJ:.o=.d)

# 5. Build Rules
all: $(LIB_NAME) $(TCC_BIN) $(TEST_BINS)

-include $(DEPS)

# --- TINYCC BUILD & PATCH ---
# We inject your include paths directly into TCC's binary
$(TCC_BIN):
	@echo "[AIC] Configuring TinyCC..."
	@cd $(TCC_DIR) && ./configure --cc=$(CC) --prefix=$(AIC_ROOT)/build/tcc \
		--extra-cflags="-DTCC_IS_NATIVE -DCONFIG_TCC_SYSINCLUDEPATHS=\"\\\"$(AIC_ROOT)/include:$(TCC_DIR)/include\\\"\""
	@echo "[AIC] Compiling TinyCC..."
	@$(MAKE) -C $(TCC_DIR) -j$(shell nproc)

# --- THE LIBRARY STEP ---
$(LIB_NAME): $(OBJS)
	@mkdir -p $(dir $@)
	@echo "[AR] $@"
	@$(AR) rcs $@ $(OBJS)

# --- LINKING WITH LLD (The Static-PIE Fix) ---
bin/%: build/tests/%.o $(STARTUP_OBJ) $(LIB_NAME)
	@mkdir -p bin
	@echo "[LD] $@"
	@$(LD) -static -pie --no-dynamic-linker $(STARTUP_OBJ) $< $(LIB_NAME) -o $@

# Standard Rules
build/objs/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	@echo "[TEST-CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/objs/arch/$(ARCH)/crt1.o: src/arch/$(ARCH)/crt1.s
	@mkdir -p $(dir $@)
	@echo "[AS] $<"
	@$(CC) $(ASFLAGS) -c $< -o $@

# --- CONVENIENCE RULE ---
# Run a specific test using your new TCC: make run T=hello
# Default test to run if none is specified
T ?= test_hello

run: all
	@echo "[AIC-TCC] Compiling tests/$(T).c..."
	@mkdir -p bin build/tests
	# Added -I$(AIC_ROOT)/include explicitly to bypass any TCC config issues
	@LD_LIBRARY_PATH=$(TCC_DIR) $(TCC_BIN) -B$(TCC_DIR) \
		-nostdinc -nostdlib \
		-I$(AIC_ROOT)/include \
		-I$(TCC_DIR)/include \
		-c tests/$(T).c -o build/tests/$(T).o
	@$(LD) -static -pie --no-dynamic-linker $(STARTUP_OBJ) build/tests/$(T).o $(LIB_NAME) -o bin/$(T)
	@echo "[AIC] Running bin/$(T):"
	@./bin/$(T)

clean:
	@rm -rf build bin
	@if [ -d "$(TCC_DIR)" ]; then $(MAKE) -C $(TCC_DIR) clean; fi
	@echo "Cleaned up."

.PHONY: all clean run
