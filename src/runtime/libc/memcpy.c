#include "string.h"

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d;
    const unsigned char* s;
    unsigned long* dl;
    unsigned long* sl;

    d = (unsigned char*)dest;
    s = (const unsigned char*)src;

    if (n > 15 && (((unsigned long)s | (unsigned long)d) & 3) == 0) {
        dl = (unsigned long*)d;
        sl = (unsigned long*)s;

        while (n > 15) {
            *dl = *sl;
            n -= 16;
            *++dl = *++sl;
            *++dl = *++sl;
            *++dl = *++sl;
            dl++;
            sl++;
        }

        while (n > 3) {
            *dl++ = *sl++;
            n -= 4;
        }

        d = (unsigned char*)dl;
        s = (const unsigned char*)sl;
    }

    while (n-- != 0) {
        *d++ = *s++;
    }

    return dest;
}
