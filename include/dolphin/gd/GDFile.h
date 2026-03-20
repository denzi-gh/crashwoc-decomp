#ifndef _DOLPHIN_GD_FILE_H
#define _DOLPHIN_GD_FILE_H

#include "dolphin/types.h"

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

typedef struct _GDGList {
    void * ptr;
    u32 byteLength;
} GDGList;

s32 GDReadDLFile(char* fName, u32* numDLs, u32* numPLs,
                 GDGList** DLDescArray, GDGList** PLDescArray);

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif
