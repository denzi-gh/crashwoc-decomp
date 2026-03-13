#ifndef SSTYPES_H
#define SSTYPES_H

#include "../../types.h"

typedef struct MIXChannel {
	struct _AXVPB *axvpb; // 0x00
	u32 mode;             // 0x04
	s32 input;            // 0x08
	s32 auxA;             // 0x0C
	s32 auxB;             // 0x10
	s32 pan;              // 0x14
	s32 span;             // 0x18
	s32 fader;            // 0x1C
	s32 l;                // 0x20
	s32 r;                // 0x24
	s32 f;                // 0x28
	s32 b;                // 0x2C
	u16 v;                // 0x30
	u16 v1;               // 0x32
	u16 vL;               // 0x34
	u16 vL1;              // 0x36
	u16 vR;               // 0x38
	u16 vR1;              // 0x3A
	u16 vS;               // 0x3C
	u16 vS1;              // 0x3E
	u16 vAL;              // 0x40
	u16 vAL1;             // 0x42
	u16 vAR;              // 0x44
	u16 vAR1;             // 0x46
	u16 vAS;              // 0x48
	u16 vAS1;             // 0x4A
	u16 vBL;              // 0x4C
	u16 vBL1;             // 0x4E
	u16 vBR;              // 0x50
	u16 vBR1;             // 0x52
	u16 vBS;              // 0x54
	u16 vBS1;             // 0x56
} MIX; // size: 0x58

#endif // !SSTYPES_H
