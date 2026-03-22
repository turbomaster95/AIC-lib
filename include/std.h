// std.h is a legacy header, only kept for backwards compatibiliity purposes
// You probably dont mean to use this header.

#ifndef AIC_STD_H
#define AIC_STD_H

#ifndef USING_STD_H_OK
    #warning "Using <std.h> is deprecated and is only there for backwards compatibility. Please include specific headers like <stdio.h> or <string.h> instead."
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#endif
