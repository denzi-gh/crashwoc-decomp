#include "stddef.h"

extern unsigned char _ctype_[];

int strncasecmp(const char *s1, const char *s2, size_t n) {
    unsigned char c1, c2;
    int l1, l2;

    if (!n)
        return 0;

    for (;;) {
        c1 = (unsigned char)*s1;
        c2 = (unsigned char)*s2;
        n--;

        l1 = (signed char)c1;
        if ((_ctype_ + 1)[l1] & 1)
            l1 += 0x20;

        l2 = (signed char)c2;
        if ((_ctype_ + 1)[l2] & 1)
            l2 += 0x20;

        if (l1 != l2)
            break;

        if (n == 0)
            break;
        if ((signed char)c1 == 0)
            break;
        if ((signed char)c2 == 0)
            break;

        s1++;
        s2++;
    }

    l1 = c1;
    if ((_ctype_ + 1)[l1] & 1)
        l1 += 0x20;

    l2 = c2;
    if ((_ctype_ + 1)[l2] & 1)
        l2 += 0x20;

    return l1 - l2;
}
