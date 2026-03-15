#include "nuerror.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void exit(int);
void _assert(const char* expr, const char* file, int line);

s32 NuFileOpen(char* filename, s32 mode);
void NuFileClose(fileHandle handle);
s32 NuFileSeek(fileHandle handle, s32 offset, s32 origin);
s32 NuFileWrite(fileHandle handle, const void* data, s32 size);

char captxt[0x100];
char* nufile;
u32 nuline;
u32 err_msg_quiet = 0;
u32 initialised = 0;
u32 dbg_msg_cnt = 0;
char* errfile;
char* errfilepath;
u32 errline;
u32 allow_printf = 1;
u32 errmsg_to_file = 0;

static void NuErrorFunction(char* msg, ...)
{
	va_list aptr;

	sprintf(captxt, "NuError - %s Line %d : ", nufile, nuline);
	va_start(aptr, msg);
	vsprintf(txt, msg, aptr);
	va_end(aptr);
	_assert(txt, nufile, nuline);
}

// PS2 MATCH
static void NuDebugMsgFunction(char* msg, ...)
{
	int fd;
	size_t len;
	char buf[0x400];
	char buf2[0x400];
	va_list aptr;

	if (err_msg_quiet == 0) {
		va_start(aptr, msg);
		NuDisableVBlankE();
		if (!initialised) {
			initialised = 1;
			fd = NuFileOpen("nu2.err", 1);
			if (fd != 0)
				NuFileClose(fd);
		}

		dbg_msg_cnt++;
		sprintf(buf2, "%05d NuDebugMsg - %s(%d) : ", dbg_msg_cnt, errfile, errline);
		vsprintf(buf, msg, aptr);
		strcat(buf2, buf);
		strcat(buf2, "\r\n");
		va_end(aptr);
		if (errmsg_to_file == 0) {
			if (allow_printf == 1)
				printf(buf2);
		} else {
			err_msg_quiet = 1;
			fd = NuFileOpen("nu2.err", 2);
			if (fd == 0)
				fd = NuFileOpen("nu2.err", 1);
			else
				NuFileSeek(fd, 0, 2);

			if (fd == 0)
				err_msg_quiet = 1;
			else {
				len = strlen(buf2);
				NuFileWrite(fd, buf2, len);
				NuFileClose(fd);
				err_msg_quiet = 0;
			}
		}
		NuEnableVBlankE();
	}
}

error_func* NuErrorProlog(char* file, s32 line)
{
	nufile = file;
	nuline = line;
	return NuErrorFunction;
}

// PS2 MATCH 
error_func* NuDebugMsgProlog(char* file, s32 line)
{
	char* c;
	char* p;

	errfilepath = file;
	errline = line;
	c = NULL;
	p = file;
	while (*p != '\0') {
		if (*p == '\\')
			c = p;
		p++;
	}
	errfile = file;
	if (c != NULL)
		errfile = c + 1;
	return NuDebugMsgFunction;
}
