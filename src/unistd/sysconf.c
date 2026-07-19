#include <unistd.h>
#include <errno.h>
#include <internal/pal.h>

long sysconf(int name) {
    switch (name) {
        case _SC_ARG_MAX:
            return 131072;
        case _SC_CHILD_MAX:
            return 256;
        case _SC_CLK_TCK:
            return 100;
        case _SC_OPEN_MAX:
            return pal_get_open_max();
        case _SC_NGROUPS_MAX:
            return 65536;
        case _SC_PAGESIZE:
            return pal_get_pagesize();
        case _SC_NPROCESSORS_CONF:
        case _SC_NPROCESSORS_ONLN:
            return pal_get_nprocessors();
        default:
            errno = EINVAL;
            return -1;
    }
}

uid_t getuid(void) {
    return (uid_t)pal_getuid();
}

uid_t geteuid(void) {
    return (uid_t)pal_geteuid();
}

gid_t getgid(void) {
    return (gid_t)pal_getgid();
}

gid_t getegid(void) {
    return (gid_t)pal_getegid();
}

int setuid(uid_t uid) {
    long ret = pal_setuid(uid);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

int setgid(gid_t gid) {
    long ret = pal_setgid(gid);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

int setreuid(uid_t ruid, uid_t euid) {
    long ret = pal_setreuid(ruid, euid);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

int setregid(gid_t rgid, gid_t egid) {
    long ret = pal_setregid(rgid, egid);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

