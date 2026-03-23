#include <getopt.h>
#include <string.h>
#include <stdio.h>

/* Global variables */
char *optarg = NULL;
int optind = 1;
int opterr = 1;
int optopt = 0;

static int optpos = 1;

int getopt(int argc, char *const argv[], const char *optstring) {
    char *opt;
    char c;
    
    optarg = NULL;
    
    if (optind == 0) {
        optind = 1;
        optpos = 1;
    }
    
    if (optind >= argc || argv[optind] == NULL || argv[optind][0] != '-') {
        return -1;
    }
    
    if (strcmp(argv[optind], "--") == 0) {
        optind++;
        return -1;
    }
    
    c = argv[optind][optpos];
    
    if (c == '\0') {
        optind++;
        optpos = 1;
        return getopt(argc, argv, optstring);
    }
    
    if (c == ':') {
        if (opterr) {
            fprintf(stderr, "%s: invalid option -- ':'\n", argv[0]);
        }
        optopt = c;
        optind++;
        optpos = 1;
        return '?';
    }
    
    opt = strchr(optstring, c);
    
    if (opt == NULL || c == '?') {
        if (opterr) {
            fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], c);
        }
        optopt = c;
        optpos++;
        if (argv[optind][optpos] == '\0') {
            optind++;
            optpos = 1;
        }
        return '?';
    }
    
    optpos++;
    
    if (opt[1] == ':') {
        if (optpos >= (int)strlen(argv[optind])) {
            optind++;
            optpos = 1;
            if (optind >= argc) {
                if (opt[2] != ':') {
                    if (opterr) {
                        fprintf(stderr, "%s: option requires an argument -- '%c'\n", argv[0], c);
                    }
                    optopt = c;
                    return '?';
                }
            } else {
                optarg = argv[optind];
                optind++;
                optpos = 1;
            }
        } else {
            optarg = argv[optind] + optpos;
            optind++;
            optpos = 1;
        }
    }
    
    if (argv[optind] == NULL || argv[optind][optpos] == '\0') {
        optind++;
        optpos = 1;
    }
    
    return c;
}

int getopt_long(int argc, char *const argv[], const char *optstring, const struct option *longopts, int *longindex) {
    char *longopt;
    size_t len;
    int i;
    
    optarg = NULL;
    
    if (optind == 0) {
        optind = 1;
        optpos = 1;
    }
    
    if (optind >= argc || argv[optind] == NULL) {
        return -1;
    }
    
    /* Check for long option */
    if (strncmp(argv[optind], "--", 2) == 0) {
        longopt = argv[optind] + 2;
        
        if (*longopt == '\0') {
            optind++;
            return -1;
        }
        
        len = strcspn(longopt, "=");
        
        for (i = 0; longopts[i].name != NULL; i++) {
            if (strncmp(longopt, longopts[i].name, len) == 0 && 
                strlen(longopts[i].name) == len) {
                
                if (longindex) {
                    *longindex = i;
                }
                
                if (longopts[i].has_arg == required_argument) {
                    char *eq = strchr(longopt, '=');
                    if (eq) {
                        optarg = eq + 1;
                    } else {
                        optind++;
                        if (optind < argc) {
                            optarg = argv[optind];
                        } else {
                            if (opterr) {
                                fprintf(stderr, "%s: option '--%s' requires an argument\n", argv[0], longopts[i].name);
                            }
                            optopt = longopts[i].val;
                            return '?';
                        }
                    }
                    optind++;
                } else if (longopts[i].has_arg == optional_argument) {
                    char *eq = strchr(longopt, '=');
                    if (eq) {
                        optarg = eq + 1;
                    }
                    optind++;
                } else {
                    optind++;
                }
                
                return longopts[i].flag ? (*longopts[i].flag = longopts[i].val, 0) : longopts[i].val;
            }
        }
        
        if (opterr) {
            fprintf(stderr, "%s: unrecognized option '--%s'\n", argv[0], longopt);
        }
        optopt = 0;
        optind++;
        return '?';
    }
    
    /* Check for short option */
    if (argv[optind][0] == '-' && argv[optind][1] != '\0') {
        return getopt(argc, argv, optstring);
    }
    
    return -1;
}

int getopt_long_only(int argc, char *const argv[], const char *optstring, const struct option *longopts, int *longindex) {
    /* Similar to getopt_long but accepts single dash for long options */
    if (optind < argc && argv[optind] != NULL && strncmp(argv[optind], "--", 2) != 0 && 
        argv[optind][0] == '-' && argv[optind][1] != '\0') {
        /* Could be a long option with single dash - check if it matches */
        char *longopt = argv[optind] + 1;
        size_t len = strcspn(longopt, "=");
        
        for (int i = 0; longopts[i].name != NULL; i++) {
            if (strncmp(longopt, longopts[i].name, len) == 0 && 
                strlen(longopts[i].name) == len) {
                return getopt_long(argc, argv, optstring, longopts, longindex);
            }
        }
    }
    
    return getopt_long(argc, argv, optstring, longopts, longindex);
}
