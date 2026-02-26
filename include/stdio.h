#ifndef _STDIO_H
#define _STDIO_H

#include "stddef.h"

typedef struct __sFILE FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int printf(const char* fmt, ...);
int sprintf(char* str, const char* fmt, ...);
int snprintf(char* str, size_t size, const char* fmt, ...);
int vsnprintf(char* str, size_t size, const char* fmt, void* ap);
int puts(const char* str);
int fputs(const char* str, FILE* stream);
int fflush(FILE* stream);

#endif
