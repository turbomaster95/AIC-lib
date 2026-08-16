#ifndef _NL_TYPES_H
#define _NL_TYPES_H

typedef int nl_item;
typedef void *nl_catd;

#define NL_SETD 1
#define NL_CAT_LOCALE 1

nl_catd catopen(const char *name, int flag);
char   *catgets(nl_catd catalog, int set_number, int message_number, const char *message);
int     catclose(nl_catd catalog);

#endif /* _NL_TYPES_H */
