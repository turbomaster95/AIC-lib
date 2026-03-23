#ifndef _GETOPT_H
#define _GETOPT_H

#include <bits/types.h>

/* Global variables */
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

/* option structure for getopt_long */
struct option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
};

/* Argument types */
#define no_argument       0
#define required_argument 1
#define optional_argument 2

/* Function declarations */
int getopt(int argc, char *const argv[], const char *optstring);
int getopt_long(int argc, char *const argv[], const char *optstring, const struct option *longopts, int *longindex);
int getopt_long_only(int argc, char *const argv[], const char *optstring, const struct option *longopts, int *longindex);

#endif /* _GETOPT_H */
