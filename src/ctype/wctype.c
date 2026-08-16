#include <wctype.h>
#include <string.h>

int iswascii(wint_t wc) {
    return (unsigned)wc < 128;
}

int iswdigit(wint_t wc) {
    return (unsigned)(wc - '0') < 10;
}

int iswalpha(wint_t wc) {
    return (unsigned)((wc | 32) - 'a') < 26;
}

int iswalnum(wint_t wc) {
    return iswalpha(wc) || iswdigit(wc);
}

int iswblank(wint_t wc) {
    return wc == ' ' || wc == '\t';
}

int iswcntrl(wint_t wc) {
    return wc < 32 || wc == 127;
}

int iswlower(wint_t wc) {
    return (unsigned)(wc - 'a') < 26;
}

int iswupper(wint_t wc) {
    return (unsigned)(wc - 'A') < 26;
}

int iswspace(wint_t wc) {
    return wc == ' ' || (unsigned)(wc - '\t') < 5;
}

int iswxdigit(wint_t wc) {
    return iswdigit(wc) || (unsigned)((wc | 32) - 'a') < 6;
}

wint_t towlower(wint_t wc) {
    if (iswupper(wc)) return wc + 32;
    return wc;
}

wint_t towupper(wint_t wc) {
    if (iswlower(wc)) return wc - 32;
    return wc;
}
