#include <time.h>
#include <internal/syscall.h>
#include <errno.h>
#include <unistd.h>

time_t time(time_t *tloc) {
    struct timeval tv;
    
    if (gettimeofday(&tv, NULL) < 0) {
        return (time_t)-1;
    }
    
    if (tloc) {
        *tloc = tv.tv_sec;
    }
    
    return tv.tv_sec;
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
    if (!tv) {
        errno = EFAULT;
        return -1;
    }
    
    long ret = __syscall2(SYS_gettimeofday, (long)tv, (long)tz);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int settimeofday(const struct timeval *tv, const struct timezone *tz) {
    long ret = __syscall2(SYS_settimeofday, (long)tv, (long)tz);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int clock_gettime(clockid_t clock_id, struct timespec *tp) {
    if (!tp) {
        errno = EFAULT;
        return -1;
    }
    
    long ret = __syscall2(SYS_clock_gettime, (long)clock_id, (long)tp);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int clock_getres(clockid_t clock_id, struct timespec *tp) {
    long ret = __syscall2(SYS_clock_getres, (long)clock_id, (long)tp);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

struct tm *gmtime(const time_t *timep) {
    static struct tm result;
    return gmtime_r(timep, &result);
}

struct tm *localtime(const time_t *timep) {
    static struct tm result;
    return localtime_r(timep, &result);
}

struct tm *gmtime_r(const time_t *timep, struct tm *result) {
    if (!timep || !result) {
        errno = EINVAL;
        return NULL;
    }
    
    time_t t = *timep;
    
    /* Simple UTC conversion - doesn't handle all edge cases */
    result->tm_sec = t % 60;
    t /= 60;
    result->tm_min = t % 60;
    t /= 60;
    result->tm_hour = t % 24;
    t /= 24;
    
    /* Days since epoch */
    result->tm_yday = 0;
    result->tm_wday = (t + 4) % 7;  /* Jan 1, 1970 was Thursday */
    
    /* Approximate year/month/day */
    int days = (int)t;
    int year = 1970;
    while (days >= 365) {
        int leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        int year_days = 365 + leap;
        if (days < year_days) break;
        days -= year_days;
        year++;
    }
    result->tm_year = year - 1900;
    
    /* Approximate month */
    static const int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    int month = 0;
    while (month < 11 && days >= month_days[month] + (month == 1 && leap)) {
        days -= month_days[month] + (month == 1 && leap);
        month++;
    }
    result->tm_mon = month;
    result->tm_mday = days + 1;
    
    result->tm_isdst = 0;
    result->tm_gmtoff = 0;
    result->tm_zone = "UTC";
    
    return result;
}

struct tm *localtime_r(const time_t *timep, struct tm *result) {
    /* For now, just return gmtime - proper timezone support is complex */
    return gmtime_r(timep, result);
}

time_t mktime(struct tm *tm) {
    if (!tm) {
        errno = EINVAL;
        return (time_t)-1;
    }
    
    /* Simplified conversion - doesn't handle all edge cases */
    int year = tm->tm_year + 1900;
    int days = 0;
    
    /* Days from years */
    for (int y = 1970; y < year; y++) {
        days += 365 + (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
    }
    
    /* Days from months */
    static const int month_days[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    days += month_days[tm->tm_mon];
    
    /* Leap year adjustment */
    if (tm->tm_mon > 1 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
        days++;
    }
    
    days += tm->tm_mday - 1;
    
    return ((time_t)days * 86400) + tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
}

char *asctime(const struct tm *tm) {
    static char buf[26];
    return asctime_r(tm, buf);
}

char *asctime_r(const struct tm *tm, char *buf) {
    if (!tm || !buf) {
        errno = EINVAL;
        return NULL;
    }
    
    static const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    /* Format: "Sun Sep 16 01:03:52 1973\n" */
    int len = 0;
    len += __builtin_sprintf(buf + len, "%.3s ", days[tm->tm_wday % 7]);
    len += __builtin_sprintf(buf + len, "%.3s ", months[tm->tm_mon % 12]);
    len += __builtin_sprintf(buf + len, "%2d ", tm->tm_mday);
    len += __builtin_sprintf(buf + len, "%02d:", tm->tm_hour);
    len += __builtin_sprintf(buf + len, "%02d:", tm->tm_min);
    len += __builtin_sprintf(buf + len, "%02d ", tm->tm_sec);
    len += __builtin_sprintf(buf + len, "%d\n", tm->tm_year + 1900);
    
    return buf;
}

char *ctime(const time_t *timep) {
    return asctime(localtime(timep));
}

char *ctime_r(const time_t *timep, char *buf) {
    return asctime_r(localtime(timep), buf);
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    long ret = __syscall2(SYS_nanosleep, (long)req, (long)rem);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int usleep(unsigned long useconds) {
    struct timespec ts;
    ts.tv_sec = useconds / 1000000;
    ts.tv_nsec = (useconds % 1000000) * 1000;
    return nanosleep(&ts, NULL);
}

unsigned int sleep(unsigned int seconds) {
    struct timespec ts;
    ts.tv_sec = seconds;
    ts.tv_nsec = 0;
    
    if (nanosleep(&ts, &ts) < 0) {
        return ts.tv_sec;
    }
    
    return 0;
}
