#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <features.h>

#define __NEED_size_t
#define __NEED_ssize_t
#define __NEED_off_t
#define __NEED_pid_t
#define __NEED_mode_t
#define __NEED_uid_t
#define __NEED_gid_t
#define __NEED_dev_t
#define __NEED_ino_t
#define __NEED_nlink_t
#define __NEED_time_t

#if defined(__USE_POSIX199309) || defined(__USE_MISC)
#  define __NEED_timer_t
#  define __NEED_clockid_t
#endif

#if defined(__USE_XOPEN) || defined(__USE_POSIX200112) || defined(__USE_MISC)
#  define __NEED_id_t
#  define __NEED_clock_t
#  define __NEED_useconds_t
#  define __NEED_suseconds_t
#  define __NEED_blksize_t
#  define __NEED_blkcnt_t
#  define __NEED_fsblkcnt_t
#  define __NEED_fsfilcnt_t
#endif

#if defined(__USE_MISC) || defined(__USE_GNU)
#  define __NEED_u_int8_t
#  define __NEED_u_int16_t
#  define __NEED_u_int32_t
#  define __NEED_u_int64_t
#endif

#include <bits/alltypes.h>

#if defined(__USE_MISC) || defined(__USE_GNU)
typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;
typedef unsigned long  u_long;
typedef char *         caddr_t;
#endif

#endif /* _SYS_TYPES_H */
