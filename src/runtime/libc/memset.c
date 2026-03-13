#include "string.h"

void* memset(void* dest, int c, size_t count)
{
    unsigned char* p;
    unsigned long* aligned_p;
    unsigned long fill;

    p = (unsigned char*)dest;

    if (count > 3 && (((unsigned long)dest & 3) == 0)) {
        c &= 0xFF;
        aligned_p = (unsigned long*)dest;
        fill = (c << 8) | c;
        fill |= fill << 16;

        while (count > 15) {
            *aligned_p = fill;
            count -= 16;
            *++aligned_p = fill;
            *++aligned_p = fill;
            *++aligned_p = fill;
            aligned_p++;
        }

        while (count > 3) {
            *aligned_p++ = fill;
            count -= 4;
        }

        p = (unsigned char*)aligned_p;
    }

    if (count-- != 0) {
        do {
            *p++ = c;
        } while (count-- != 0);
    }

    return dest;
}
