#include <sys/utsname.h>
#include <internal/syscall.h>
#include <errno.h>
#include <string.h>

int uname(struct utsname *buf) {
    if (!buf) {
        errno = EFAULT;
        return -1;
    }
    
    long ret = __syscall1(SYS_uname, (long)buf);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}
