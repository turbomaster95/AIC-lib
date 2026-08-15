#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <features.h>

#define __NEED_time_t
#define __NEED_suseconds_t
#define __NEED_select_arg234
#include <bits/alltypes.h>

struct timeval {
	time_t tv_sec;
	suseconds_t tv_usec;
};

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

struct itimerval {
	struct timeval it_interval;
	struct timeval it_value;
};

int gettimeofday(struct timeval *restrict, void *restrict);
int settimeofday(const struct timeval *, const struct timezone *);
int getitimer(int, struct itimerval *);
int setitimer(int, const struct itimerval *restrict, struct itimerval *restrict);

#endif /* _SYS_TIME_H */
