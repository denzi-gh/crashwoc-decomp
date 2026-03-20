/* Minimal Dolphin SDK type definitions for standalone SDK source compilation. */
#ifndef _SDK_TYPES_H_
#define _SDK_TYPES_H_

typedef signed   char          s8;
typedef unsigned char          u8;
typedef signed   short int     s16;
typedef unsigned short int     u16;
typedef signed   long          s32;
typedef unsigned long          u32;
typedef signed   long long int s64;
typedef unsigned long long int u64;

typedef float  f32;
typedef double f64;

typedef int BOOL;

#define FALSE 0
#define TRUE 1

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef void (*OSAlarmHandler)(void* alarm, void* context);

/* Minimal string/mem functions */
void *memcpy(void *, const void *, u32);
void *memset(void *, int, u32);
int memcmp(const void *, const void *, u32);
u32 strlen(const char *);
char *strncpy(char *, const char *, u32);
int strcmp(const char *, const char *);

/* ASSERTMSG stubs - strip asserts for matching */
#define ASSERTMSGLINE(line, cond, msg)
#define ASSERTLINE(line, cond)
#define ASSERTMSG(cond, msg)

/* OS basics */
typedef s32 OSPriority;
typedef void (*OSIdleFunction)(void *param);

#endif /* _SDK_TYPES_H_ */
