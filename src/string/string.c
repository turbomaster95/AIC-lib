#include <string.h>
#include <stdlib.h>

size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (n && (*d++ = *src++)) n--;
    while (n--) *d++ = '\0';
    return dest;
}

char *strcat(char *dest, const char *src) {
    char *d = dest;
    while (*d) d++;
    while ((*d++ = *src++));
    return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (*d) d++;
    while (n && *src) {
        *d++ = *src++;
        n--;
    }
    *d = '\0';
    return dest;
}

char *strchr(const char *s, int c) {
    while (*s != (char)c) {
        if (!*s) return 0;
        s++;
    }
    return (char *)s;
}

char *strrchr(const char *s, int c) {
    const char *last = 0;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    if ((char)c == '\0') return (char *)s;
    return (char *)last;
}

char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    
    while (*haystack) {
        const char *h = haystack;
        const char *n = needle;
        
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        
        if (!*n) return (char *)haystack;
        haystack++;
    }
    
    return 0;
}

char *strtok(char *str, const char *delim) {
    static char *saveptr;
    return strtok_r(str, delim, &saveptr);
}

char *strtok_r(char *str, const char *delim, char **saveptr) {
    char *token;
    
    if (str == NULL) {
        str = *saveptr;
    }
    
    /* Skip leading delimiters */
    str += strspn(str, delim);
    
    if (*str == '\0') {
        *saveptr = str;
        return NULL;
    }
    
    token = str;
    str += strcspn(str, delim);
    
    if (*str) {
        *str++ = '\0';
    }
    
    *saveptr = str;
    return token;
}

int strcasecmp(const char *s1, const char *s2) {
    unsigned char c1, c2;
    
    while (1) {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        
        /* Convert to lowercase */
        if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
        if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
        
        if (c1 != c2) return c1 - c2;
        if (c1 == '\0') return 0;
    }
}

int strncasecmp(const char *s1, const char *s2, size_t n) {
    unsigned char c1, c2;
    
    while (n--) {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        
        if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
        if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
        
        if (c1 != c2) return c1 - c2;
        if (c1 == '\0') return 0;
    }
    
    return 0;
}

char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *d = (char *)malloc(len);
    if (d) {
        memcpy(d, s, len);
    }
    return d;
}

char *strndup(const char *s, size_t n) {
    size_t len = strlen(s);
    if (len > n) len = n;
    char *d = (char *)malloc(len + 1);
    if (d) {
        memcpy(d, s, len);
        d[len] = '\0';
    }
    return d;
}

size_t strspn(const char *s, const char *accept) {
    size_t count = 0;
    while (*s) {
        const char *a = accept;
        int found = 0;
        while (*a) {
            if (*s == *a) {
                found = 1;
                break;
            }
            a++;
        }
        if (!found) break;
        s++;
        count++;
    }
    return count;
}

size_t strcspn(const char *s, const char *reject) {
    size_t count = 0;
    while (*s) {
        const char *r = reject;
        while (*r) {
            if (*s == *r) return count;
            r++;
        }
        s++;
        count++;
    }
    return count;
}

char *strpbrk(const char *s, const char *accept) {
    while (*s) {
        const char *a = accept;
        while (*a) {
            if (*s == *a) return (char *)s;
            a++;
        }
        s++;
    }
    return 0;
}
