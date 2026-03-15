extern unsigned char _ctype_[];

char* strlwr(char* s) {
    char* p = s;
    int c;

    if (*p == '\0')
        return s;

    do {
        c = (signed char)*p;
        if ((_ctype_ + 1)[c] & 1)
            *p = c + 0x20;
    } while (*++p != '\0');

    return s;
}
