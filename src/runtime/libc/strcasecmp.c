extern unsigned char _ctype_[];

int strcasecmp(const char *s1, const char *s2) {
    int c1, c2;

    for (;;) {
        if (*s1 == '\0')
            break;

        c1 = *s1;
        if ((_ctype_ + 1)[c1] & 1)
            c1 += 0x20;

        c2 = *s2;
        if ((_ctype_ + 1)[c2] & 1)
            c2 += 0x20;

        if (c1 != c2)
            break;

        s1++;
        s2++;
    }

    c2 = (unsigned char)*s2;
    if ((_ctype_ + 1)[c2] & 1)
        c2 += 0x20;

    c1 = (unsigned char)*s1;
    if ((_ctype_ + 1)[c1] & 1)
        c1 += 0x20;

    return c1 - c2;
}
