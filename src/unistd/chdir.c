#include <internal/pal.h>
#include <unistd.h>

int chdir(const char *path) {
    return (int)pal_chdir(path);
}
