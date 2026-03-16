/* SN Systems ProDG runtime stubs */

int InitializeUART();
int WriteUARTN(void* buf, unsigned long len);
int PCwrite();
int PCread();

static int first = 1;

int __sn_serialp(char* buf, int len) {
    if (first) {
        first = 0;
        InitializeUART(0);
    }
    WriteUARTN(buf, len);
    return len;
}

int _write(int fd, char* buf, int count) {
    if (fd == 1 || fd == 2) {
        return __sn_serialp(buf, count);
    }
    if (fd == 0) {
        return -1;
    }
    return PCwrite(fd);
}

int read(int fd, void* buf, int count) {
    return PCread(fd, buf, count);
}
