#include "string.h"

#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)

size_t strlen(const char *str) {
    const char *start = str;
    unsigned long *aligned_addr;

    if (((unsigned long)str & 3) == 0) {
        aligned_addr = (unsigned long *)str;
        if (!DETECTNULL(*aligned_addr)) {
            do {
                aligned_addr++;
            } while (!DETECTNULL(*aligned_addr));
        }
        str = (const char *)aligned_addr;
    }

    while (*str) {
        str++;
    }

    return str - start;
}
