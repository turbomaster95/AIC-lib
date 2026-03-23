#include <stdio.h>
#include <errno.h>
#include <setjmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

jmp_buf env;

void test_errno() {
    printf("=== Testing errno ===\n");
    
    /* Test that errno is initially 0 */
    printf("Initial errno: %d\n", errno);
    
    /* Try to open a non-existent file */
    int fd = open("/nonexistent_file_12345", O_RDONLY);
    if (fd < 0) {
        printf("open() failed as expected, errno = %d (ENOENT = %d)\n", errno, ENOENT);
    }
    
    /* Test strerror would go here if we had it */
    errno = 0;
    printf("errno after reset: %d\n", errno);
}

void test_stderr() {
    printf("=== Testing stderr ===\n");
    
    fprintf(stderr, "This is a message to stderr\n");
    fprintf(stdout, "This is a message to stdout\n");
    
    /* Test that stderr is unbuffered (should appear immediately) */
    fprintf(stderr, "stderr line 1\n");
    fprintf(stdout, "stdout line 1 (buffered)\n");
    fprintf(stderr, "stderr line 2\n");
    fflush(stdout);  /* Force stdout to flush */
}

void test_setjmp() {
    printf("=== Testing setjmp/longjmp ===\n");
    
    int val = setjmp(env);
    
    if (val == 0) {
        printf("setjmp() returned 0 (initial call)\n");
        printf("Calling longjmp(1)...\n");
        longjmp(env, 1);
        printf("This should not print\n");
    } else {
        printf("setjmp() returned %d (after longjmp)\n", val);
    }
}

void test_fcntl() {
    printf("=== Testing fcntl ===\n");
    
    /* Test O_CREAT flag */
    int fd = open("/data/data/com.termux/files/home/AIC/test_fcntl_tmp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        printf("Created file with fd = %d\n", fd);
        const char *msg = "Hello from fcntl test\n";
        write(fd, msg, strlen(msg));
        close(fd);
        printf("Wrote and closed file\n");
    } else {
        printf("Failed to create file, errno = %d\n", errno);
    }
    
    /* Test fcntl F_GETFD */
    fd = open("/data/data/com.termux/files/home/AIC/test_fcntl_tmp", O_RDONLY);
    if (fd >= 0) {
        int flags = fcntl(fd, F_GETFD);
        printf("fcntl F_GETFD returned: %d\n", flags);
        close(fd);
    }
}

int main() {
    printf("AIC New Features Test\n");
    printf("=====================\n\n");
    
    test_errno();
    printf("\n");
    
    test_stderr();
    printf("\n");
    
    test_setjmp();
    printf("\n");
    
    test_fcntl();
    printf("\n");
    
    printf("All tests completed!\n");
    return 0;
}
