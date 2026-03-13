struct __va_list_tag
{
    unsigned char gpr;
    unsigned char fpr;
    void* overflow_arg_area;
    void* reg_save_area;
};

typedef struct __va_list_tag va_list[1];

#define va_start(ap, last) ((void)(last), __builtin_memcpy(&(ap), __builtin_saveregs(), sizeof(va_list)))

struct _reent;

struct __sbuf
{
    unsigned char* _base;
    int _size;
};

typedef struct __sFILE
{
    unsigned char* _p;
    int _r;
    int _w;
    short _flags;
    short _file;
    struct __sbuf _bf;
    int _lbfsize;
    void* _cookie;
    int (*_read)(void*, char*, int);
    int (*_write)(void*, const char*, int);
    int (*_seek)(void*, int, int);
    int (*_close)(void*);
    struct __sbuf _ub;
    unsigned char* _up;
    int _ur;
    unsigned char _ubuf[3];
    unsigned char _nbuf[1];
    struct __sbuf _lb;
    int _blksize;
    int _offset;
    char _unused[8];
    struct _reent* _data;
} FILE;

struct _reent
{
    int _errno;
    FILE* _stdin;
    FILE* _stdout;
};

extern struct _reent* _impure_ptr;
extern unsigned char _ctype_[];
extern int vfprintf(FILE* stream, const char* format, va_list ap);

#define ERANGE 34

long _strtol_r(struct _reent* reent, const char* nptr, char** endptr, int base)
{
    const char* s;
    unsigned long acc;
    unsigned long cutoff;
    int c;
    int cutlim;
    int neg;
    int any;

    s = nptr;
    neg = 0;

    do {
        c = *s++;
    } while (((& _ctype_[1])[c] & 8) != 0);

    if (c == '-') {
        c = *s++;
        neg = 1;
    } else if (c == '+') {
        c = *s++;
    }

    if ((base == 0 || base == 16) && c == '0') {
        if (*s == 'x' || *s == 'X') {
            c = s[1];
            s += 2;
            base = 16;
        }
    }

    if (base == 0) {
        base = (c == '0') ? 8 : 10;
    }

    cutoff = 0x7FFFFFFFUL;
    if (neg != 0) {
        cutoff = 0x80000000UL;
    }

    cutlim = (int)(cutoff % (unsigned long)base);
    cutoff /= (unsigned long)base;

    acc = 0;
    any = 0;

    for (;;) {
        if (((_ctype_ + 1)[c] & 4) != 0) {
            c -= '0';
        } else if (((_ctype_ + 1)[c] & 3) != 0) {
            c -= (((_ctype_ + 1)[c] & 1) != 0) ? ('A' - 10) : ('a' - 10);
        } else {
            break;
        }

        if (c >= base) {
            break;
        }

        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= (unsigned long)base;
            acc += (unsigned long)c;
        }

        c = *s++;
    }

    if (any < 0) {
        acc = 0x7FFFFFFFUL;
        if (neg != 0) {
            acc = 0x80000000UL;
        }
        reent->_errno = ERANGE;
    } else if (neg != 0) {
        acc = (unsigned long)(-(long)acc);
    }

    if (endptr != 0) {
        if (any != 0) {
            nptr = s - 1;
        }
        *endptr = (char*)nptr;
    }

    return (long)acc;
}

long strtol(const char* nptr, char** endptr, int base)
{
    return _strtol_r(_impure_ptr, nptr, endptr, base);
}

int printf(const char* format, ...)
{
    va_list ap;

    va_start(ap, format);
    _impure_ptr->_stdout->_data = _impure_ptr;
    return vfprintf(_impure_ptr->_stdout, format, ap);
}

int sprintf(char* str, const char* format, ...)
{
    va_list ap;
    FILE f;
    int ret;

    f._flags = 0x208;
    f._p = (unsigned char*)str;
    f._w = 0x7FFFFFFF;
    f._bf._size = 0x7FFFFFFF;
    f._bf._base = (unsigned char*)str;
    f._data = _impure_ptr;

    va_start(ap, format);
    ret = vfprintf(&f, format, ap);
    *f._p = '\0';
    return ret;
}
