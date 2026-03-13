#include "string.h"

#define DETECTCHAR(X, MASK) ((((X) ^ (MASK)) - 0x01010101) & ~((X) ^ (MASK)) & 0x80808080)

void* memchr(const void* src, int c, size_t count)
{
    const unsigned char* p;
    const unsigned long* aligned_p;
    unsigned long mask;
    int i;

    c &= 0xFF;

    if (count > 3 && (((unsigned long)src & 3) == 0)) {
        aligned_p = (const unsigned long*)src;
        mask = 0;

        for (i = 0; i < 4; i++) {
            mask = (mask << 8) + c;
        }

        while (count > 3) {
            if (DETECTCHAR(*aligned_p, mask) != 0) {
                p = (const unsigned char*)aligned_p;

                for (i = 0; i < 4; i++) {
                    if (*p == c) {
                        return (void*)p;
                    }

                    p++;
                }
            }

            count -= 4;
            aligned_p++;
        }

        src = aligned_p;
    }

    p = (const unsigned char*)src;

    if (count-- != 0) {
        do {
            if (*p == c) {
                return (void*)p;
            }

            p++;
        } while (count-- != 0);
    }

    return 0;
}
