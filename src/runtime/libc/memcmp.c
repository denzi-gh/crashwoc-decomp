#include "string.h"

#define UNALIGNED(X, Y) ((((long)X) & (sizeof(long) - 1)) | (((long)Y) & (sizeof(long) - 1)))

int memcmp(const void *lhs, const void *rhs, size_t count)
{

  const unsigned char *p1;
  p1 = (const unsigned char *) lhs;
  if ((count > 3) && (!((((long) p1) & ((sizeof(long)) - 1)) | (((long) rhs) & ((sizeof(long)) - 1)))))
  {
    p1 = rhs;
    if ((*((const unsigned long *) p1)) != (*((const unsigned long *) lhs)))
    {
      goto bytes;
    }
    do
    {
      count = count - 4;
      lhs = ((const unsigned char *) lhs) + 4;
      rhs = ((const unsigned char *) rhs) + 4;
      if (count <= 3)
      {
        break;
      }
    }
    while ((*((const unsigned long *) lhs)) == (*((const unsigned long *) rhs)));
    bytes:
    p1 = (const unsigned char *) lhs;

  }
  if ((count--) != 0)
  {
    do
    {
      int a;
      int b;
      a = *p1;
      b = *((const unsigned char *) rhs);
      if (a != b)
      {
        return a - b;
      }
      p1++;
      rhs = ((const unsigned char *) rhs) + 1;
    }
    while ((count--) != 0);
  }
  return 0;

}
