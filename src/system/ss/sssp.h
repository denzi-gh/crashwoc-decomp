#ifndef SSSP_H
#define SSSP_H

#include "../../types.h"

/*
  800ce1d0 0001a4 800ce1d0  4 SPInitSoundTable 	Global
  800ce374 000024 800ce374  4 SPGetSoundEntry 	Global
  800ce398 00055c 800ce398  4 SPPrepareSound 	Global
  800ce8f4 000058 800ce8f4  4 SPPrepareEnd 	Global
*/

typedef struct _AXPBADPCM {
    u16 a[2][8];
    u16 gain;
    u16 pred_scale;
    u16 yn1;
    u16 yn2;
} AXPBADPCM;

typedef struct _AXPBADPCMLOOP {
    u16 loop_pred_scale;
    u16 loop_yn1;
    u16 loop_yn2;
} AXPBADPCMLOOP;

typedef struct {
    AXPBADPCM adpcm;
    AXPBADPCMLOOP adpcmloop;
} SPAdpcmEntry;

typedef struct {
    u32 type;
    u32 sampleRate;
    u32 loopAddr;
    u32 loopEndAddr;
    u32 endAddr;
    u32 currentAddr;
    SPAdpcmEntry *adpcm;
} SPSoundEntry;

typedef struct {
    u32 entries;
    SPSoundEntry sound[1];
} SPSoundTable;

typedef struct _AXPBMIX {
    u16 vL;
    u16 vDeltaL;
    u16 vR;
    u16 vDeltaR;
    u16 vAuxAL;
    u16 vDeltaAuxAL;
    u16 vAuxAR;
    u16 vDeltaAuxAR;
    u16 vAuxBL;
    u16 vDeltaAuxBL;
    u16 vAuxBR;
    u16 vDeltaAuxBR;
    u16 vAuxBS;
    u16 vDeltaAuxBS;
    u16 vS;
    u16 vDeltaS;
    u16 vAuxAS;
    u16 vDeltaAuxAS;
} AXPBMIX;

typedef struct _AXPBITD {
    u16 flag;
    u16 bufferHi;
    u16 bufferLo;
    u16 shiftL;
    u16 shiftR;
    u16 targetShiftL;
    u16 targetShiftR;
} AXPBITD;

typedef struct _AXPBUPDATE {
    u16 updNum[5];
    u16 dataHi;
    u16 dataLo;
} AXPBUPDATE;

typedef struct _AXPBDPOP {
    s16 aL;
    s16 aAuxAL;
    s16 aAuxBL;
    s16 aR;
    s16 aAuxAR;
    s16 aAuxBR;
    s16 aS;
    s16 aAuxAS;
    s16 aAuxBS;
} AXPBDPOP;

typedef struct _AXPBVE {
    u16 currentVolume;
    s16 currentDelta;
} AXPBVE;

typedef struct _AXPBFIR {
    u16 numCoefs;
    u16 coefsHi;
    u16 coefsLo;
} AXPBFIR;

typedef struct _AXPBADDR {
    u16 loopFlag;
    u16 format;
    u16 loopAddressHi;
    u16 loopAddressLo;
    u16 endAddressHi;
    u16 endAddressLo;
    u16 currentAddressHi;
    u16 currentAddressLo;
} AXPBADDR;

typedef struct _AXPBSRC {
    u16 ratioHi;
    u16 ratioLo;
    u16 currentAddressFrac;
    u16 last_samples[4];
} AXPBSRC;

typedef struct _AXPB {
    u16 nextHi;
    u16 nextLo;
    u16 currHi;
    u16 currLo;
    u16 srcSelect;
    u16 coefSelect;
    u16 mixerCtrl;
    u16 state;
    u16 type;
    AXPBMIX mix;
    AXPBITD itd;
    AXPBUPDATE update;
    AXPBDPOP dpop;
    AXPBVE ve;
    AXPBFIR fir;
    AXPBADDR addr;
    AXPBADPCM adpcm;
    AXPBSRC src;
    AXPBADPCMLOOP adpcmLoop;
    u16 pad[3];
} AXPB;

typedef struct _AXVPB {
    void *next;
    void *prev;
    void *next1;
    u32 priority;
    void (*callback)(void *);
    u32 userContext;
    u32 index;
    u32 sync;
    u32 depop;
    u32 updateMS;
    u32 updateCounter;
    u32 updateTotal;
    u16 *updateWrite;
    u16 updateData[128];
    void *itdBuffer;
    AXPB pb;
} AXVPB;

void SPInitSoundTable(SPSoundTable *table, u32 aramBase, u32 zeroBase);
SPSoundEntry *SPGetSoundEntry(SPSoundTable *table, u32 index);
void SPPrepareSound(SPSoundEntry *sound, AXVPB *axvpb, u32 sampleRate);
void SPPrepareEnd(SPSoundEntry *sound, AXVPB *axvpb);

#endif