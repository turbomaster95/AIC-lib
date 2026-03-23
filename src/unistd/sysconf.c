#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

long sysconf(int name) {
    switch (name) {
        case _SC_ARG_MAX:
            return 131072;
        case _SC_CHILD_MAX:
            return 256;
        case _SC_CLK_TCK:
            return 100;
        case _SC_OPEN_MAX:
            return 1024;
        case _SC_NGROUPS_MAX:
            return 65536;
        case _SC_PAGESIZE:
            return 4096;
        case _SC_NPROCESSORS_CONF:
        case _SC_NPROCESSORS_ONLN:
            return 1;  /* Simplified - would need CPU detection */
        default:
            errno = EINVAL;
            return -1;
    }
}

uid_t getuid(void) {
    return (uid_t)__syscall0(SYS_getuid);
}

uid_t geteuid(void) {
    return (uid_t)__syscall0(SYS_geteuid);
}

gid_t getgid(void) {
    return (gid_t)__syscall0(SYS_getgid);
}

gid_t getegid(void) {
    return (gid_t)__syscall0(SYS_getegid);
}

int setuid(uid_t uid) {
    long ret = __syscall1(SYS_setuid, (long)uid);

    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }

    return 0;
}

int setgid(gid_t gid) {
    long ret = __syscall1(SYS_setgid, (long)gid);

    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }

    return 0;
}

int setreuid(uid_t ruid, uid_t euid) {
    long ret = __syscall2(SYS_setreuid, (long)ruid, (long)euid);

    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }

    return 0;
}

int setregid(gid_t rgid, gid_t egid) {
    long ret = __syscall2(SYS_setregid, (long)rgid, (long)egid);

    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }

    return 0;
}
