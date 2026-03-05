#ifndef _STDIO_H
#define _STDIO_H

#include "stddef.h"
#include "stdarg.h"

typedef struct __sFILE FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int printf(const char* fmt, ...);
int vprintf(const char* fmt, va_list ap);
int sprintf(char* str, const char* fmt, ...);
int vsprintf(char* str, const char* fmt, va_list ap);
int snprintf(char* str, size_t size, const char* fmt, ...);
int vsnprintf(char* str, size_t size, const char* fmt, va_list ap);
int sscanf(const char* str, const char* fmt, ...);
int puts(const char* str);
int fputs(const char* str, FILE* stream);
int fflush(FILE* stream);
FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);
size_t fread(void* ptr, size_t size, size_t count, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream);
int fseek(FILE* stream, long offset, int origin);
long ftell(FILE* stream);
void rewind(FILE* stream);
int feof(FILE* stream);

#endif
