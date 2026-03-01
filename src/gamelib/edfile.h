#ifndef EDFILE_H
#define EDFILE_H

#include "../types.h"
#include "stdlib.h"

// Current ed file handle.
extern fileHandle edfile_handle;

// Open the ed file.
s32 EdFileOpen(char *filename, s32 mode);

// Close the ed file.
s32 EdFileClose(void);

// Read from an ed file.
void EdFileRead(void *dest, size_t size);

// Read a float from an ed file.
f32 EdFileReadFloat(void);

// Read an int from an ed file.
s32 EdFileReadInt(void);

// Read a short from an ed file.
s16 EdFileReadShort(void);

// Read a char from an ed file.
char EdFileReadChar(void);

#endif // !EDFILE_H
