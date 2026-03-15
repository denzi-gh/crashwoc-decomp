#include "string.h"

#define ALIGNED(X) (((long)(X) & (sizeof(long) - 1)) == 0)
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)

char* strcat(char* s1, const char* s2)
{
    char* s = s1;

    /* Skip over the data in s1 as quickly as possible. */
    if (ALIGNED(s1)) {
        unsigned long* aligned_s1 = (unsigned long*)s1;
        while (!DETECTNULL(*aligned_s1))
            aligned_s1++;
        s1 = (char*)aligned_s1;
    }

    while (*s1)
        s1++;

    strcpy(s1, s2);
    return s;
}
