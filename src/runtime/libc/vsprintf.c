#include <stdarg.h>

struct _reent;

struct __sbuf {
    unsigned char *_base;
    int _size;
};

typedef struct {
    int __count;
    union {
        unsigned int __wch;
        unsigned char __wchb[4];
    } __value;
} _mbstate_t;

typedef struct __sFILE {
    unsigned char *_p;
    int _r;
    int _w;
    short _flags;
    short _file;
    struct __sbuf _bf;
    int _lbfsize;
    void *_cookie;
    int (*_read)(void *, char *, int);
    int (*_write)(void *, const char *, int);
    int (*_seek)(void *, int, int);
    int (*_close)(void *);
    struct __sbuf _ub;
    unsigned char *_up;
    int _ur;
    unsigned char _ubuf[3];
    unsigned char _nbuf[1];
    struct __sbuf _lb;
    int _blksize;
    int _offset;
    _mbstate_t _mbstate;
    struct _reent *_data;
} FILE;

extern struct _reent *_impure_ptr;

int vfprintf(FILE *, const char *, va_list);

int vsprintf(char *str, const char *fmt, va_list ap) {
    int ret;
    FILE f;

    f._flags = 0x0208;
    f._bf._base = f._p = (unsigned char *)str;
    f._bf._size = f._w = 0x7FFFFFFF;
    f._data = _impure_ptr;

    ret = vfprintf(&f, fmt, ap);
    *f._p = '\0';
    return ret;
}
