#include "edfile.h"
#include "../nu.h"

fileHandle edfile_handle = -1;

s32 EdFileOpen(char *filename, s32 mode)
{
	if (edfile_handle == -1)
	{
		edfile_handle = NuFileOpen(filename, 0);
		if (edfile_handle != -1)
		{
			return 1;
		}
	}
	return 0;
}

s32 EdFileClose(void)
{
	s32 ret;
	if (edfile_handle != -1) {
		NuFileClose(edfile_handle);
		edfile_handle = -1;
		ret = 1;
	} else {
		ret = 0;
	}
	return ret;
}

void EdFileRead(void *dest, size_t size)
{
	NuFileRead(edfile_handle, dest, size);
}

f32 EdFileReadFloat(void)
{
	f32 tmp = 0.0f;
	EdFileRead(&tmp, sizeof(f32));
	return tmp;
}

s32 EdFileReadInt(void)
{
	s32 tmp = 0;
	EdFileRead(&tmp, sizeof(s32));
	return tmp;
}

s16 EdFileReadShort(void)
{
	s16 tmp = 0;
	EdFileRead(&tmp, sizeof(s16));
	return tmp;
}

char EdFileReadChar(void)
{
	char tmp = 0;
	EdFileRead(&tmp, 1);
	return tmp;
}
