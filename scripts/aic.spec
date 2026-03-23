# AIC (Ain't it C) GCC Spec File
# =============================================================================
# Usage: gcc --sysroot=/path/to/sysroot -specs=aic.spec myprog.c -o myprog
#
# This spec file modifies gcc's default behavior:
# - Disables standard system libraries (-nostdlib)
# - Links against libaic automatically
# - Uses AIC startup files
# =============================================================================

# Replace the default linker specification
*link:
%{!shared:%{!static:-static-pie}} --no-standard-libraries -laic

# Startup files - use our crt1.o
*startfile:
%{!shared:crt1.o}

# Ensure our lib path is searched
*libpath:
-L /usr/lib -L /lib

# Pass preprocessor defines
*cpp:
-D__AIC__=1 -D__FREESTANDING__=1
