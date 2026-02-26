#ifndef _STDLIB_H
#define _STDLIB_H

#include "stddef.h"

void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
int abs(int value);
long strtol(const char* nptr, char** endptr, int base);
unsigned long strtoul(const char* nptr, char** endptr, int base);
double strtod(const char* nptr, char** endptr);

#endif
