#include <stdarg.h>

typedef struct __sFILE FILE;

struct _reent {
    int _errno;
    FILE *_stdin;
    FILE *_stdout;
    FILE *_stderr;
};

extern struct _reent *_impure_ptr;

int vfprintf(FILE *, const char *, va_list);

int vprintf(const char *fmt, va_list ap) {
    return vfprintf(_impure_ptr->_stdout, fmt, ap);
}
