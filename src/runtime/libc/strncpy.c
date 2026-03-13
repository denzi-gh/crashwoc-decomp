#include "string.h"

#define UNALIGNED(X, Y) ((((long)X) & (sizeof(long) - 1)) | (((long)Y) & (sizeof(long) - 1)))
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#define TOO_SMALL(LEN) ((LEN) < sizeof(long))

char* strncpy(char* dst0, const char* src0, size_t count)
{
    char* dst;
    const char* src;
    long* aligned_dst;
    const long* aligned_src;

    dst = dst0;
    src = src0;

    if (!UNALIGNED(src, dst) && !TOO_SMALL(count)) {
        aligned_dst = (long*)dst;
        aligned_src = (const long*)src;

        while (count >= sizeof(long) && !DETECTNULL(*aligned_src)) {
            count -= sizeof(long);
            *aligned_dst++ = *aligned_src++;
        }

        dst = (char*)aligned_dst;
        src = (const char*)aligned_src;
    }

    while (count > 0) {
        --count;
        if ((*dst++ = *src++) == '\0') {
            break;
        }
    }

    while (count-- > 0) {
        *dst++ = '\0';
    }

    return dst0;
}
