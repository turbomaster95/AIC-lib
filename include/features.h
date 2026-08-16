#ifndef _FEATURES_H
#define _FEATURES_H

#undef __USE_ISOC11
#undef __USE_ISOC99
#undef __USE_POSIX
#undef __USE_POSIX2
#undef __USE_POSIX199309
#undef __USE_POSIX199506
#undef __USE_POSIX200112
#undef __USE_POSIX200809
#undef __USE_XOPEN
#undef __USE_XOPEN2K
#undef __USE_XOPEN2K8
#undef __USE_BSD
#undef __USE_MISC
#undef __USE_GNU
#undef __USE_ATFILE

#if defined(__STDC_VERSION__)
  #if __STDC_VERSION__ >= 199901L
    #define __USE_ISOC99 1
  #endif
  #if __STDC_VERSION__ >= 201112L
    #define __USE_ISOC11 1
  #endif
#endif

#if !defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE) && \
    !defined(_XOPEN_SOURCE)  && !defined(_GNU_SOURCE) && \
    !defined(_BSD_SOURCE)    && !defined(_DEFAULT_SOURCE)
  #define _DEFAULT_SOURCE 1
#endif

/* Allow legacy _BSD_SOURCE or _SVID_SOURCE to act as _DEFAULT_SOURCE */
#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE)
  #ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE 1
  #endif
#endif

#if defined(_GNU_SOURCE)
  #undef _POSIX_SOURCE
  #define _POSIX_SOURCE 1

  #undef _POSIX_C_SOURCE
  #define _POSIX_C_SOURCE 200809L

  #undef _XOPEN_SOURCE
  #define _XOPEN_SOURCE 700

  #undef _DEFAULT_SOURCE
  #define _DEFAULT_SOURCE 1

  #define __USE_GNU 1
#endif

#if defined(_POSIX_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 1)
  #define __USE_POSIX 1
#endif

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 2)
  #define __USE_POSIX2 1
#endif

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
  #define __USE_POSIX199309 1
#endif

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 199506L)
  #define __USE_POSIX199506 1
#endif

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L)
  #define __USE_POSIX200112 1
#endif

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L)
  #define __USE_POSIX200809 1
  #define __USE_ATFILE 1
#endif

#if defined(_XOPEN_SOURCE)
  #define __USE_XOPEN 1
  #if (_XOPEN_SOURCE - 0) >= 500
    #define __USE_XOPEN_EXT 1
    #ifndef _POSIX_C_SOURCE
      #define _POSIX_C_SOURCE 199506L
    #endif
  #endif
  #if (_XOPEN_SOURCE - 0) >= 600
    #define __USE_XOPEN2K 1
    #ifndef _POSIX_C_SOURCE
      #define _POSIX_C_SOURCE 200112L
    #endif
  #endif
  #if (_XOPEN_SOURCE - 0) >= 700
    #define __USE_XOPEN2K8 1
    #ifndef _POSIX_C_SOURCE
      #define _POSIX_C_SOURCE 200809L
    #endif
  #endif
#endif

#if defined(_DEFAULT_SOURCE)
  #define __USE_MISC 1
  #define __USE_BSD 1
  
  /* Default mode brings in standard POSIX 2008 features automatically */
  #ifndef __USE_POSIX
    #define __USE_POSIX 1
  #endif
  #ifndef __USE_POSIX2
    #define __USE_POSIX2 1
  #endif
  #ifndef __USE_POSIX199309
    #define __USE_POSIX199309 1
  #endif
  #ifndef __USE_POSIX199506
    #define __USE_POSIX199506 1
  #endif
  #ifndef __USE_POSIX200112
    #define __USE_POSIX200112 1
  #endif
  #ifndef __USE_POSIX200809
    #define __USE_POSIX200809 1
    #define __USE_ATFILE 1
  #endif
#endif

#if defined(__GNUC__) || defined(__clang__)
  #define __restrict_arr __restrict
  #define __extension__  __extension__
#else
  #define __restrict_arr
  #define __extension__
#endif

#ifndef __THROW
  #define __THROW
#endif

#endif /* _FEATURES_H */
