#include "stdlib.h"

int atoi(const char* nptr)
{
    return (int)strtol(nptr, (char**)0, 10);
}
