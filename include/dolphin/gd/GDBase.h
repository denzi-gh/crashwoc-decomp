#ifndef _DOLPHIN_GD_BASE_H
#define _DOLPHIN_GD_BASE_H

#include "dolphin/types.h"

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

typedef void (*GDOverflowCb)();

typedef struct _GDLObj {
	u8* start;
	u32 length;
	u8* ptr;
	u8* top;
} GDLObj;

extern GDLObj* __GDCurrentDL;

void GDInitGDLObj(GDLObj* dl, void* start, u32 length);
void GDFlushCurrToMem(void);
void GDPadCurr32(void);
void GDOverflowed(void);
void GDSetOverflowCallback(GDOverflowCb callback);
GDOverflowCb GDGetOverflowCallback(void);

inline static void GDOverflowCheck(u32 size)
{
	if (__GDCurrentDL->ptr + size > __GDCurrentDL->top)
		GDOverflowed();
}

inline static void __GDWrite(u8 data) { *__GDCurrentDL->ptr++ = data; }

inline static void GDWrite_u8(u8 data)
{
	GDOverflowCheck(sizeof(u8));
	__GDWrite(data);
}

inline static void GDWrite_s8(s8 data)
{
	GDWrite_u8((u8) data);
}

inline static void GDWrite_u16(u16 data)
{
	GDOverflowCheck(sizeof(u16));
	__GDWrite((u8)(data >> 8));
	__GDWrite((u8)(data & 0xFF));
}

inline static void GDWrite_s16(s16 data)
{
	GDWrite_u16((u16) data);
}

inline static void GDWrite_u24(u32 data)
{
  GDOverflowCheck(3);
  __GDWrite((u8)((data >> 16) & 0xff));
  __GDWrite((u8)((data >> 8) & 0xff));
  __GDWrite((u8)((data >> 0) & 0xff));
}

inline static void GDWrite_u32(u32 data)
{
	GDOverflowCheck(sizeof(u32));
	__GDWrite((u8)((data >> 24) & 0xFF));
	__GDWrite((u8)((data >> 16) & 0xFF));
	__GDWrite((u8)((data >> 8) & 0xFF));
	__GDWrite((u8)((data >> 0) & 0xFF));
}

inline static void GDWrite_s32(s32 data)
{
	GDWrite_u32((u32) data);
}

inline static void GDWrite_f32(float data)
{
	union {
		float f;
		u32 u;
	} fid;
	fid.f = data;
	GDWrite_u32(fid.u);
}

inline static void GDWriteXFCmdHdr(u16 addr, u8 len)
{
	GDWrite_u8(16);
	GDWrite_u16(len - 1);
	GDWrite_u16(addr);
}

inline static void GDWriteXFCmd(u16 addr, u32 val)
{
	GDWrite_u8(0x10);
	GDWrite_u16(0);
	GDWrite_u16(addr);
	GDWrite_u32(val);
}

inline static void GDWriteXFIndxDCmd(u16 addr, u8 len, u16 index)
{
  GDWrite_u8(0x38);
  GDWrite_u16(index);
  GDWrite_u16((len - 1) << 12 | addr);
}

inline static void GDWriteXFIndxACmd(u16 addr, u8 len, u16 index)
{
  GDWrite_u8(0x20);
  GDWrite_u16(index);
  GDWrite_u16(((len - 1) << 12) | addr);
}

inline static void GDWriteXFIndxBCmd(u16 addr, u8 len, u16 index)
{
  GDWrite_u8(0x28);
  GDWrite_u16(index);
  GDWrite_u16(((len - 1) << 12) | addr);
}

inline static void GDWriteXFIndxCCmd(u16 addr, u8 len, u16 index)
{
  GDWrite_u8(0x30);
  GDWrite_u16(index);
  GDWrite_u16(((len - 1) << 12) | addr);
}

inline static void GDWriteCPCmd(u8 addr, u32 val)
{
	GDWrite_u8(8);
	GDWrite_u8(addr);
	GDWrite_u32(val);
}

inline static void GDWriteBPCmd(u32 regval)
{
	GDWrite_u8(0x61);
	GDWrite_u32(regval);
}

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif
