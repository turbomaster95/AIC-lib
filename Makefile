# 1. Identity & Architecture Detection
ARCH   ?= $(shell uname -m)
CC      = gcc
AR      = ar

# 2. Path Definitions
INCLUDES = -Iinclude -Isrc -Isrc/arch/$(ARCH)
CFLAGS   = $(INCLUDES) -MMD -MP -nostdlib -ffreestanding -Wall -O2 -fno-stack-protector
ASFLAGS  = $(INCLUDES) -nostdlib -Wall
LIB_NAME = build/lib/libaic.a

# 3. Automatic Source Discovery
SRCS        = $(shell find src -name "*.c")
STARTUP     = src/arch/$(ARCH)/crt1.s
TEST_SRCS   = $(wildcard tests/*.c)

# 4. Object & Binary Mapping
OBJS        = $(SRCS:src/%.c=build/objs/%.o)
STARTUP_OBJ = build/objs/arch/$(ARCH)/crt1.o
TEST_BINS   = $(TEST_SRCS:tests/%.c=bin/%)

# Find all generated dependency files
DEPS        = $(OBJS:.o=.d) $(STARTUP_OBJ:.o=.d)

# 5. Build Rules
all: $(LIB_NAME) $(TEST_BINS)

-include $(DEPS)

# --- THE LIBRARY STEP ---
# This merges all core objects (except crt1.o) into a single archive
$(LIB_NAME): $(OBJS)
	@mkdir -p $(dir $@)
	@echo "[AR] $@"
	@$(AR) rcs $@ $(OBJS)

# Rule to build each individual test binary
# Note: We link against the .a file and the startup .o separately
bin/%: build/tests/%.o $(STARTUP_OBJ) $(LIB_NAME)
	@mkdir -p bin
	@echo "[LD] $@"
	@$(CC) $(CFLAGS) $(STARTUP_OBJ) $< $(LIB_NAME) -o $@

# Rule for library C files
build/objs/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Rule for the individual test files
build/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	@echo "[TEST-CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Rule for Assembly startup
build/objs/arch/$(ARCH)/crt1.o: src/arch/$(ARCH)/crt1.s
	@mkdir -p $(dir $@)
	@echo "[AS] $<"
	@$(CC) $(ASFLAGS) -c $< -o $@

clean:
	@rm -rf build bin
	@echo "Cleaned up."

.PHONY: all clean
