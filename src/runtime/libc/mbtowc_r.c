#include "stddef.h"

struct _reent;

int _mbtowc_r(struct _reent *r, int *pwc, const char *s, size_t n)
{
    int dummy;

    if (pwc == NULL)
        pwc = &dummy;

    if (s != NULL) {
        if (n == 0)
            return -1;

        *pwc = (unsigned char)*s;

        if (*s == '\0')
            return 0;

        return 1;
    }

    return 0;
}

typedef void (*func_ptr)(void);
extern func_ptr __CTOR_LIST__[];

void __do_global_ctors(void)
{
    unsigned long nptrs;
    unsigned int i;

    nptrs = (unsigned long)__CTOR_LIST__[0];
    if (nptrs == (unsigned long)-1)
        for (nptrs = 0; __CTOR_LIST__[nptrs + 1] != 0; nptrs++);

    for (i = nptrs; i >= 1; i--)
        __CTOR_LIST__[i]();
}

static int initialized;

void __main(void)
{
    if (!initialized) {
        initialized = 1;
        __do_global_ctors();
    }
}
