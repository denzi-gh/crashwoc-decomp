#ifndef SSMIX_H
#define SSMIX_H

#include "../../types.h"
#include "sstypes.h"
#include "sssp.h"

#define NUM_MIX_CHANNELS 64

// Mixer channels.
MIX __MIXChannel[NUM_MIX_CHANNELS];

// Current attentuation.
u32 __MIXDvdStreamAttenCurrent;

// User attenuation.
u32 __MIXDvdStreamAttenUser;

// Sound mode.
u32 __MIXSoundMode;

// Volume table (extern, defined in another unit).
extern u16 __MIXVolumeTable[965];

int OSDisableInterrupts(void);
void OSRestoreInterrupts(int level);

// Convert pan byte to pan value.
s32 __MIXPanTableL[128];

// Convert pan byte to pan value.
s32 __MIXPanTableR[128];

// Convert dB to volume value.
u16 __MIXGetVolume(s32 db);

// Get the true pan value.
s32 __MIXGetPanL(s32 pan);

// Get the true pan value.
s32 __MIXGetPanR(s32 pan);

// Reset a mix channel to defaults.
void __MIXResetChannel(MIX *channel);

// Recompute pan fields for a mix channel.
void __MIXSetPan(MIX *channel);

// Initialize all the mixing channels.
void MIXInit(void);

// Initialize a single mix channel from AXVPB params.
void MIXInitChannel(AXVPB *axvpb, u32 mode, s32 input, s32 auxA, s32 auxB, s32 pan, s32 span, s32 fader);

// Release a mix channel.
void MIXReleaseChannel(AXVPB *axvpb);

// Set the input volume for a channel.
void MIXSetInput(AXVPB *p, s32 dB);

#endif // !SSMIX_H