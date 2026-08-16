#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

#include <features.h>
#include <limits.h>
#include <sys/types.h>
#include <endian.h>

#define MAXSYMLINKS 40
#define MAXHOSTNAMELEN 64
#define MAXPATHLEN PATH_MAX
#define NOFILE OPEN_MAX

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define powerof2(x) ((((x) - 1) & (x)) == 0)

#ifndef DEV_BSIZE
#define DEV_BSIZE 512
#endif

#endif /* _SYS_PARAM_H */
