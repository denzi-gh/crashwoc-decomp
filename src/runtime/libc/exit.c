struct _atexit
{
    struct _atexit* _next;
    int _ind;
    void (*_fns[32])();
};

struct _reent
{
    char _pad0[0x3C];
    void (*__cleanup)(struct _reent*);
    char _pad1[0x108];
    struct _atexit* _atexit;
};

extern struct _reent* _impure_ptr;
extern void _exit(int status) __attribute__((noreturn));

void exit(int status)
{
    register struct _atexit* atexit;
    register int i;

    atexit = _impure_ptr->_atexit;
    while (atexit != 0) {
        i = atexit->_ind;
        while (--i >= 0) {
            (*atexit->_fns[i])();
        }
        atexit = atexit->_next;
    }

    if (_impure_ptr->__cleanup != 0) {
        _impure_ptr->__cleanup(_impure_ptr);
    }

    _exit(status);
}
