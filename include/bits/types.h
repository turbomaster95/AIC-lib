#ifndef _BITS_TYPES_H
#define _BITS_TYPES_H

#include <features.h>

typedef int                __pid_t;
typedef __SIZE_TYPE__      __size_t;
typedef long               __ssize_t;
typedef long               __off_t;
typedef unsigned int       __uid_t;
typedef unsigned int       __gid_t;
typedef long               __clock_t;
typedef unsigned int       __mode_t;
typedef unsigned long      __ino_t;
typedef unsigned int       __dev_t;
typedef long               __blkcnt_t;
typedef long               __blksize_t;
typedef unsigned int       __nlink_t;
typedef long               __time_t;
typedef long               __suseconds_t;
typedef unsigned long      __nfds_t;
typedef int                __clockid_t;
typedef unsigned long      __fsfilcnt_t;
typedef unsigned long      __fsblkcnt_t;

#ifndef __pid_t_defined
#define __pid_t_defined
typedef __pid_t pid_t;
#endif

#ifndef __size_t_defined
#define __size_t_defined
typedef __size_t size_t;
#endif

#ifndef __ssize_t_defined
#define __ssize_t_defined
typedef __ssize_t ssize_t;
#endif

#ifndef __off_t_defined
#define __off_t_defined
typedef __off_t off_t;
#endif

#ifndef __uid_t_defined
#define __uid_t_defined
typedef __uid_t uid_t;
#endif

#ifndef __gid_t_defined
#define __gid_t_defined
typedef __gid_t gid_t;
#endif

#ifndef __clock_t_defined
#define __clock_t_defined
typedef __clock_t clock_t;
#endif

#ifndef __mode_t_defined
#define __mode_t_defined
typedef __mode_t mode_t;
#endif

#ifndef __ino_t_defined
#define __ino_t_defined
typedef __ino_t ino_t;
#endif

#ifndef __dev_t_defined
#define __dev_t_defined
typedef __dev_t dev_t;
#endif

#ifndef __blkcnt_t_defined
#define __blkcnt_t_defined
typedef __blkcnt_t blkcnt_t;
#endif

#ifndef __blksize_t_defined
#define __blksize_t_defined
typedef __blksize_t blksize_t;
#endif

#ifndef __nlink_t_defined
#define __nlink_t_defined
typedef __nlink_t nlink_t;
#endif

#ifndef __time_t_defined
#define __time_t_defined
typedef __time_t time_t;
#endif

#ifndef __suseconds_t_defined
#define __suseconds_t_defined
typedef __suseconds_t suseconds_t;
#endif

#ifndef __nfds_t_defined
#define __nfds_t_defined
typedef __nfds_t nfds_t;
#endif

#ifndef __clockid_t_defined
#define __clockid_t_defined
typedef __clockid_t clockid_t;
#endif

#ifndef __fsfilcnt_t_defined
#define __fsfilcnt_t_defined
typedef __fsfilcnt_t fsfilcnt_t;
#endif

#ifndef __fsblkcnt_t_defined
#define __fsblkcnt_t_defined
typedef __fsblkcnt_t fsblkcnt_t;
#endif

#endif /* _BITS_TYPES_H */
