#ifndef _TIME_H
#define _TIME_H

#include <features.h>

#define __NEED_NULL
#define __NEED_size_t
#define __NEED_time_t
#define __NEED_clockid_t
#define __NEED_clock_t
#define __NEED_timer_t
#define __NEED_struct_timespec
#include <bits/alltypes.h>

#define CLOCKS_PER_SEC 1000000L

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
	long tm_gmtoff;
	const char *tm_zone;
};

clock_t clock(void);
time_t time(time_t *);
double difftime(time_t, time_t);
time_t mktime(struct tm *);
size_t strftime(char *restrict, size_t, const char *restrict, const struct tm *restrict);
struct tm *gmtime(const time_t *);
struct tm *localtime(const time_t *);
char *asctime(const struct tm *);
char *ctime(const time_t *);

#if defined(_POSIX_C_SOURCE) || defined(__USE_XOPEN) || defined(__USE_GNU)
#define CLOCK_REALTIME           0
#define CLOCK_MONOTONIC          1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID  3

struct tm *gmtime_r(const time_t *restrict, struct tm *restrict);
struct tm *localtime_r(const time_t *restrict, struct tm *restrict);
char *asctime_r(const struct tm *restrict, char *restrict);
char *ctime_r(const time_t *, char *);

int nanosleep(const struct timespec *, struct timespec *);
int clock_gettime(clockid_t, struct timespec *);
int clock_getres(clockid_t, struct timespec *);
int clock_settime(clockid_t, const struct timespec *);
#endif

#if defined(__USE_GNU) || defined(__USE_BSD)
int usleep(unsigned int useconds);
unsigned int sleep(unsigned int);
#endif

#endif /* _TIME_H */
