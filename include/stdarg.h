#ifndef _STDARG_H
#define _STDARG_H

typedef char* va_list;

#define _VA_SIZE(type) (((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define va_start(ap, last) (ap = ((va_list)&(last) + _VA_SIZE(last)))
#define va_arg(ap, type) (*(type*)(((ap += _VA_SIZE(type)) - _VA_SIZE(type))))
#define va_end(ap) (ap = (va_list)0)

#endif
