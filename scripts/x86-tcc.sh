#!/bin/sh
echo "running x86 custom builder"

# 1. Enter the directory
cd third_party/tinycc

# 2. Wipe any 64-bit or stale objects
make clean

# 3. Explicitly build the library and the executable sequentially
# Using -j1 here is crucial because of the race condition we saw earlier
make -j1 tccdefs_.h
CMD="$1 -o tcc tcc.c -DONE_SOURCE=1 ${@:2} -lm -ldl -lpthread -I."
echo $CMD
$CMD
