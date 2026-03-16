struct _reent {
    char _pad[0x58];
    unsigned long _rand_next;
};

extern struct _reent* _impure_ptr;

void srand(unsigned int seed) {
    _impure_ptr->_rand_next = seed;
}

int rand(void) {
    struct _reent* reent = _impure_ptr;
    reent->_rand_next = reent->_rand_next * 1103515245 + 12345;
    return (int)(reent->_rand_next & 0x7fffffff);
}
