#ifndef __DEFINED_wchar_t
typedef int wchar_t;
#define __DEFINED_wchar_t
#endif

#if defined(__FLT_EVAL_METHOD__) && __FLT_EVAL_METHOD__ == 2
typedef long double float_t;
typedef long double double_t;
#else
typedef float float_t;
typedef double double_t;
#endif

#if defined(__NEED_NULL) && !defined(__DEFINED_NULL)
#undef NULL
#define NULL ((void *)0)
#define __DEFINED_NULL
#endif

#if defined(__NEED_size_t) && !defined(__DEFINED_size_t)
typedef __SIZE_TYPE__ size_t;
#define __DEFINED_size_t
#endif

#if defined(__NEED_ssize_t) && !defined(__DEFINED_ssize_t)
typedef long ssize_t;
#define __DEFINED_ssize_t
#endif

#if defined(__NEED_ptrdiff_t) && !defined(__DEFINED_ptrdiff_t)
typedef __PTRDIFF_TYPE__ ptrdiff_t;
#define __DEFINED_ptrdiff_t
#endif

#if defined(__NEED_intptr_t) && !defined(__DEFINED_intptr_t)
typedef __INTPTR_TYPE__ intptr_t;
#define __DEFINED_intptr_t
#endif

#if defined(__NEED_uintptr_t) && !defined(__DEFINED_uintptr_t)
typedef __UINTPTR_TYPE__ uintptr_t;
#define __DEFINED_uintptr_t
#endif

#if defined(__NEED_time_t) && !defined(__DEFINED_time_t)
typedef long time_t;
#define __DEFINED_time_t
#endif

#if defined(__NEED_clock_t) && !defined(__DEFINED_clock_t)
typedef long clock_t;
#define __DEFINED_clock_t
#endif

#if defined(__NEED_clockid_t) && !defined(__DEFINED_clockid_t)
typedef int clockid_t;
#define __DEFINED_clockid_t
#endif

#if defined(__NEED_timer_t) && !defined(__DEFINED_timer_t)
typedef void *timer_t;
#define __DEFINED_timer_t
#endif

#if defined(__NEED_suseconds_t) && !defined(__DEFINED_suseconds_t)
typedef long suseconds_t;
#define __DEFINED_suseconds_t
#endif

#if defined(__NEED_useconds_t) && !defined(__DEFINED_useconds_t)
typedef unsigned int useconds_t;
#define __DEFINED_useconds_t
#endif

#if defined(__NEED_pid_t) && !defined(__DEFINED_pid_t)
typedef int pid_t;
#define __DEFINED_pid_t
#endif

#if defined(__NEED_uid_t) && !defined(__DEFINED_uid_t)
typedef unsigned int uid_t;
#define __DEFINED_uid_t
#endif

#if defined(__NEED_gid_t) && !defined(__DEFINED_gid_t)
typedef unsigned int gid_t;
#define __DEFINED_gid_t
#endif

#if defined(__NEED_id_t) && !defined(__DEFINED_id_t)
typedef unsigned int id_t;
#define __DEFINED_id_t
#endif

#if defined(__NEED_off_t) && !defined(__DEFINED_off_t)
typedef long off_t;
#define __DEFINED_off_t
#endif

#if defined(__NEED_mode_t) && !defined(__DEFINED_mode_t)
typedef unsigned int mode_t;
#define __DEFINED_mode_t
#endif

#if defined(__NEED_dev_t) && !defined(__DEFINED_dev_t)
typedef unsigned long dev_t;
#define __DEFINED_dev_t
#endif

#if defined(__NEED_ino_t) && !defined(__DEFINED_ino_t)
typedef unsigned long ino_t;
#define __DEFINED_ino_t
#endif

#if defined(__NEED_wint_t) && !defined(__DEFINED_wint_t)
typedef unsigned int wint_t;
#define __DEFINED_wint_t
#endif

#if defined(__NEED_wctype_t) && !defined(__DEFINED_wctype_t)
typedef unsigned long wctype_t;
#define __DEFINED_wctype_t
#endif

#if defined(__NEED_wctrans_t) && !defined(__DEFINED_wctrans_t)
typedef const int *wctrans_t;
#define __DEFINED_wctrans_t
#endif

#if defined(__NEED_nlink_t) && !defined(__DEFINED_nlink_t)
typedef unsigned long nlink_t;
#define __DEFINED_nlink_t
#endif

#if defined(__NEED_blksize_t) && !defined(__DEFINED_blksize_t)
typedef long blksize_t;
#define __DEFINED_blksize_t
#endif

#if defined(__NEED_blkcnt_t) && !defined(__DEFINED_blkcnt_t)
typedef long blkcnt_t;
#define __DEFINED_blkcnt_t
#endif

#if defined(__NEED_fsblkcnt_t) && !defined(__DEFINED_fsblkcnt_t)
typedef unsigned long fsblkcnt_t;
#define __DEFINED_fsblkcnt_t
#endif

#if defined(__NEED_fsfilcnt_t) && !defined(__DEFINED_fsfilcnt_t)
typedef unsigned long fsfilcnt_t;
#define __DEFINED_fsfilcnt_t
#endif

#if defined(__NEED_u_int8_t) && !defined(__DEFINED_u_int8_t)
typedef unsigned char u_int8_t;
#define __DEFINED_u_int8_t
#endif

#if defined(__NEED_u_int16_t) && !defined(__DEFINED_u_int16_t)
typedef unsigned short u_int16_t;
#define __DEFINED_u_int16_t
#endif

#if defined(__NEED_u_int32_t) && !defined(__DEFINED_u_int32_t)
typedef unsigned int u_int32_t;
#define __DEFINED_u_int32_t
#endif

#if defined(__NEED_u_int64_t) && !defined(__DEFINED_u_int64_t)
typedef unsigned long long u_int64_t;
#define __DEFINED_u_int64_t
#endif

#if defined(__NEED_nfds_t) && !defined(__DEFINED_nfds_t)
typedef unsigned long nfds_t;
#define __DEFINED_nfds_t
#endif

#if defined(__NEED_sigset_t) && !defined(__DEFINED_sigset_t)
typedef struct { unsigned long __bits[128/sizeof(long)]; } sigset_t;
#define __DEFINED_sigset_t
#endif

#if defined(__NEED_struct_timespec) && !defined(__DEFINED_struct_timespec)
struct timespec { time_t tv_sec; long tv_nsec; };
#define __DEFINED_struct_timespec
#endif

#if defined(__NEED_struct_timeval) && !defined(__DEFINED_struct_timeval)
struct timeval {
	time_t tv_sec;
	suseconds_t tv_usec;
};
#define __DEFINED_struct_timeval
#endif

#undef __NEED_NULL
#undef __NEED_size_t
#undef __NEED_ssize_t
#undef __NEED_ptrdiff_t
#undef __NEED_intptr_t
#undef __NEED_uintptr_t
#undef __NEED_sigset_t
#undef __NEED_struct_timespec
#undef __NEED_struct_timeval
#undef __NEED_time_t
#undef __NEED_clock_t
#undef __NEED_clockid_t
#undef __NEED_timer_t
#undef __NEED_suseconds_t
#undef __NEED_useconds_t
#undef __NEED_pid_t
#undef __NEED_uid_t
#undef __NEED_gid_t
#undef __NEED_id_t
#undef __NEED_off_t
#undef __NEED_mode_t
#undef __NEED_dev_t
#undef __NEED_ino_t
#undef __NEED_nlink_t
#undef __NEED_blksize_t
#undef __NEED_blkcnt_t
#undef __NEED_fsblkcnt_t
#undef __NEED_fsfilcnt_t
#undef __NEED_u_int8_t
#undef __NEED_u_int16_t
#undef __NEED_u_int32_t
#undef __NEED_u_int64_t
#undef __NEED_nfds_t
