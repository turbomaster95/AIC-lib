# =============================================================================
# Usage: gcc --sysroot=/path/to/sysroot -specs=kora.spec myprog.c -o myprog
#
# This spec file modifies gcc's default behavior:
# - Disables standard system libraries (-nostdlib)
# - Links against libkora automatically
# - Uses Kora startup files
# =============================================================================

# Replace the default linker specification
*link:
%{!shared:%{!static:-static-pie}} --no-standard-libraries -lkora

# Startup files - use our crt1.o
*startfile:
%{!shared:crt1.o}

# Ensure our lib path is searched
*libpath:
-L /usr/lib -L /lib

# Pass preprocessor defines
*cpp:
-D__FREESTANDING__=1
