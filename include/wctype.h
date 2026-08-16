#ifndef _WCTYPE_H
#define _WCTYPE_H

#include <features.h>

#define __NEED_wint_t
#define __NEED_wctype_t
#define __NEED_wctrans_t
#include <bits/alltypes.h>

#ifndef WEOF
#define WEOF ((wint_t)-1)
#endif

/* Character Classification Functions */
int iswalnum(wint_t wc);
int iswalpha(wint_t wc);
int iswblank(wint_t wc);
int iswcntrl(wint_t wc);
int iswdigit(wint_t wc);
int iswgraph(wint_t wc);
int iswlower(wint_t wc);
int iswprint(wint_t wc);
int iswpunct(wint_t wc);
int iswspace(wint_t wc);
int iswupper(wint_t wc);
int iswxdigit(wint_t wc);

/* Case Mapping Functions */
wint_t towlower(wint_t wc);
wint_t towupper(wint_t wc);

/* Dynamic Extensible Character Classes */
wctype_t wctype(const char *property);
int iswctype(wint_t wc, wctype_t desc);

wctrans_t wctrans(const char *property);
wint_t towctrans(wint_t wc, wctrans_t desc);

#endif /* _WCTYPE_H */
