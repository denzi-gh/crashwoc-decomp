#include "string.h"

#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)

char *strcpy(char *dst0, const char *src0) {
    char *dst = dst0;
    const char *src = src0;
    unsigned long *aligned_dst;
    const unsigned long *aligned_src;

    if (!(((unsigned long)src | (unsigned long)dst) & 3)) {
        aligned_dst = (unsigned long *)dst;
        aligned_src = (const unsigned long *)src;

        while (!DETECTNULL(*aligned_src)) {
            *aligned_dst++ = *aligned_src++;
        }

        dst = (char *)aligned_dst;
        src = (const char *)aligned_src;
    }

    while ((*dst++ = *src++) != '\0')
        ;

    return dst0;
}
