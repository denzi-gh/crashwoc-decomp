#ifndef NUSOUNDTYPES_H
#define NUSOUNDTYPES_H

#include "../types.h"

enum PlayingStatus
{
	STOPPED,
	PLAYING
};

// Size: 0x10
struct NuSndLoopInfo_s {
	struct nuvec_s *pos;    // 0x00
	s16 playing;            // 0x04
	s16 channel;            // 0x06
	s16 timer;              // 0x08
	s16 pad;                // 0x0A
	s16 vol_l;              // 0x0C
	s16 vol_r;              // 0x0E
};

#endif // !NUSOUNDTYPES_H