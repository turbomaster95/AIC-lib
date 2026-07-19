#ifndef CTYPE_H
#define CTYPE_H

static inline int isdigit(int c) {
	return (unsigned)c - '0' < 10;
}

static inline int isalpha(int c) {
	return ((unsigned)c | 32) - 'a' < 26;
}

static inline int isalnum(int c) {
	return isdigit(c) || isalpha(c);
}

static inline int isspace(int c) {
	return c == ' ' || (unsigned)c - '\t' < 5;
}

static inline int isupper(int c) {
	return (unsigned)c - 'A' < 26;
}

static inline int islower(int c) {
	return (unsigned)c - 'a' < 26;
}

static inline int toupper(int c) {
	return islower(c) ? (c - 0x20) : c;
}

static inline int tolower(int c) {
	return isupper(c) ? (c + 0x20) : c;
}

#endif
