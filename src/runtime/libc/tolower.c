extern unsigned char _ctype_[];

int tolower(int c) {
    if ((_ctype_ + 1)[c] & 1)
        c += 0x20;
    return c;
}
