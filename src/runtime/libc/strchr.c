#include "string.h"

#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#define DETECTCHAR(X, MASK) (((((X) ^ (MASK)) - 0x01010101) & ~((X) ^ (MASK))) & 0x80808080)

char *strchr(const char *s1, int i)
{
    const unsigned char *s = (const unsigned char *)s1;
    unsigned char c = i;
    unsigned long mask;
    unsigned long *aligned_addr;
    int j;

    if (((long)s & 3) == 0)
    {
        mask = 0;
        for (j = 0; j < 4; j++)
            mask = (mask << 8) | c;

        aligned_addr = (unsigned long *)s;
        while (!DETECTNULL(*aligned_addr) && !DETECTCHAR(*aligned_addr, mask))
            aligned_addr++;

        s = (const unsigned char *)aligned_addr;
    }

    while (*s && *s != c)
        s++;

    if (*s == c)
        return (char *)s;
    return NULL;
}
