#ifndef _TIME_H
#define _TIME_H

#include <bits/types.h>
#include <stddef.h>

/* Time types */
typedef long time_t;
typedef long clock_t;
typedef long suseconds_t;

/* Clock IDs */
#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 1

/* timespec structure */
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

/* timeval structure */
struct timeval {
    time_t tv_sec;
    suseconds_t tv_usec;
};

/* timezone structure */
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

/* tm structure */
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

/* itimerval structure */
struct itimerval {
    struct timeval it_interval;
    struct timeval it_value;
};

/* Time constants */
#define CLOCKS_PER_SEC 1000000L
#define CLK_TCK CLOCKS_PER_SEC

/* Function declarations */
time_t time(time_t *tloc);
int gettimeofday(struct timeval *tv, struct timezone *tz);
int settimeofday(const struct timeval *tv, const struct timezone *tz);
int clock_gettime(clockid_t clock_id, struct timespec *tp);
int clock_getres(clockid_t clock_id, struct timespec *tp);
struct tm *gmtime(const time_t *timep);
struct tm *localtime(const time_t *timep);
struct tm *gmtime_r(const time_t *timep, struct tm *result);
struct tm *localtime_r(const time_t *timep, struct tm *result);
time_t mktime(struct tm *tm);
char *asctime(const struct tm *tm);
char *asctime_r(const struct tm *tm, char *buf);
char *ctime(const time_t *timep);
char *ctime_r(const time_t *timep, char *buf);
size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);
int nanosleep(const struct timespec *req, struct timespec *rem);
int usleep(unsigned long useconds);
unsigned int sleep(unsigned int seconds);
int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
int getitimer(int which, struct itimerval *old_value);

/* Clock ID type */
typedef int clockid_t;

#endif /* _TIME_H */
