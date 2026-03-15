#ifndef _STDARG_H
#define _STDARG_H

struct __va_list_tag {
    unsigned char gpr;
    unsigned char fpr;
    void* overflow_arg_area;
    void* reg_save_area;
};

typedef struct __va_list_tag va_list[1];

#define va_start(ap, last) \
  ((void)(last), *(ap) = *(struct __va_list_tag *)__builtin_saveregs())
#define va_end(ap) ((void)0)

#endif
