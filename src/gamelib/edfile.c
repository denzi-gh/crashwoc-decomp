#include "edfile.h"
#include "../nu.h"

fileHandle edfile_handle = -1;

s32 EdFileOpen(char *filename, s32 mode)
{
	if (edfile_handle == -1)
	{
		edfile_handle = NuFileOpen(filename, mode);
		if (edfile_handle != -1)
		{
			return 1;
		}
	}
	return 0;
}

s32 EdFileClose(void)
{
	s32 fileOpened = edfile_handle != -1;
	if (fileOpened) {
		NuFileClose(edfile_handle);
		edfile_handle = -1;
	}
	return fileOpened;
}

void EdFileRead(void *dest, size_t size)
{
	NuFileRead(edfile_handle, dest, size);
}

f32 EdFileReadFloat(void)
{
	return NuFileReadFloat(edfile_handle); // Makes more sense than not calling and doing the hard way.
}

s32 EdFileReadInt(void)
{
	return NuFileReadInt(edfile_handle); // Makes more sense than not calling and doing the hard way.
}

s16 EdFileReadShort(void)
{
	return NuFileReadShort(edfile_handle); // Makes more sense than not calling and doing the hard way.
}

char EdFileReadChar(void)
{
	char tmp = 0;
	NuFileRead(edfile_handle, &tmp, 1);
	return tmp;
}
