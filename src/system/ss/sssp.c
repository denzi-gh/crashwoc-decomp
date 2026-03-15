#include "sssp.h"

struct DTKTrack;

extern struct DTKTrack SS_DTKTrack;
extern int SS_CurrentChannel;
extern int SS_TrackPlayingFlag;
extern volatile int flush_flag;

int OSDisableInterrupts(void);
void OSRestoreInterrupts(int level);
unsigned int DTKQueueTrack(char *fileName, struct DTKTrack *track, unsigned int eventMask, void (*callback)(unsigned int));
void DTKFlushTracks(void (*callback)(void));
void DTKSetRepeatMode(unsigned int mode);
void DTKSetState(unsigned int state);
unsigned int DTKGetState(void);
struct DTKTrack *DTKGetCurrentTrack(void);
void GC_DiskErrorPoll(void);

extern void __flush_callback_800CEAA0(void);

void SPInitSoundTable(SPSoundTable *table, u32 aramBase, u32 zeroBase) {
    u32 i;
    SPSoundEntry *sound;
    SPAdpcmEntry *adpcm;
    u32 aramBase16;
    u32 aramBase2;
    u32 zeroBase4;
    u32 zeroBase16;

    zeroBase4 = zeroBase >> 1;
    zeroBase16 = (zeroBase << 1) + 2;
    aramBase16 = aramBase << 1;
    aramBase2 = aramBase >> 1;

    sound = &table->sound[0];
    adpcm = (SPAdpcmEntry *)((u8 *)table + (table->entries * 0x1C + 4));

    for (i = 0; i < table->entries; i++) {
        switch (sound->type) {
        case 0:
            sound->adpcm = adpcm;
            sound->loopEndAddr = 0;
            sound->endAddr = aramBase16 + sound->endAddr;
            adpcm++;
            sound->currentAddr = aramBase16 + sound->currentAddr;
            sound->loopAddr = zeroBase16;
            break;
        case 1:
            sound->loopAddr = aramBase16 + sound->loopAddr;
            sound->loopEndAddr = aramBase16 + sound->loopEndAddr;
            sound->endAddr = aramBase16 + sound->endAddr;
            sound->adpcm = adpcm;
            sound->currentAddr = aramBase16 + sound->currentAddr;
            adpcm++;
            break;
        case 2:
            sound->endAddr = aramBase2 + sound->endAddr;
            sound->loopAddr = zeroBase4;
            sound->currentAddr = aramBase2 + sound->currentAddr;
            sound->loopEndAddr = 0;
            break;
        case 3:
            sound->loopAddr = aramBase2 + sound->loopAddr;
            sound->loopEndAddr = aramBase2 + sound->loopEndAddr;
            sound->endAddr = aramBase2 + sound->endAddr;
            sound->currentAddr = aramBase2 + sound->currentAddr;
            break;
        case 4:
            sound->endAddr = aramBase + sound->endAddr;
            sound->loopAddr = zeroBase;
            sound->currentAddr = aramBase + sound->currentAddr;
            sound->loopEndAddr = 0;
            break;
        case 5:
            sound->loopAddr = aramBase + sound->loopAddr;
            sound->loopEndAddr = aramBase + sound->loopEndAddr;
            sound->endAddr = aramBase + sound->endAddr;
            sound->currentAddr = aramBase + sound->currentAddr;
            break;
        }
        sound++;
    }
}

SPSoundEntry *SPGetSoundEntry(SPSoundTable *table, u32 index) {
    if (table->entries > index) return &table->sound[index];
    return 0;
}

void SPPrepareSound(SPSoundEntry *sound, AXVPB *axvpb, u32 sampleRate) {
    int old;
    u32 srcBits;
    u32 loopAddr;
    u32 endAddr;
    u32 currentAddr;
    u32 type;
    u16 zero;
    u32 sync;

    srcBits = (u32)((float)sampleRate / 32000.0f * 65536.0f);

    type = sound->type;
    switch (type) {
    case 0: {
        u16 *p;
        p = (u16 *)sound->adpcm;
        endAddr = sound->endAddr;
        currentAddr = sound->currentAddr;
        loopAddr = sound->loopAddr;
        old = OSDisableInterrupts();
        axvpb->pb.addr.loopAddressHi = (u16)(loopAddr >> 16);
        axvpb->pb.addr.currentAddressHi = (u16)(currentAddr >> 16);
        axvpb->pb.addr.loopAddressLo = (u16)loopAddr;
        axvpb->pb.addr.endAddressHi = (u16)(endAddr >> 16);
        axvpb->pb.addr.endAddressLo = (u16)endAddr;
        axvpb->pb.addr.currentAddressLo = (u16)currentAddr;
        axvpb->pb.addr.loopFlag = type;
        axvpb->pb.addr.format = type;
        axvpb->pb.adpcm.a[0][0] = *p;
        axvpb->pb.adpcm.a[0][1] = *++p;
        axvpb->pb.adpcm.a[0][2] = *++p;
        axvpb->pb.adpcm.a[0][3] = *++p;
        axvpb->pb.adpcm.a[0][4] = *++p;
        axvpb->pb.adpcm.a[0][5] = *++p;
        axvpb->pb.adpcm.a[0][6] = *++p;
        axvpb->pb.adpcm.a[0][7] = *++p;
        axvpb->pb.adpcm.a[1][0] = *++p;
        axvpb->pb.adpcm.a[1][1] = *++p;
        axvpb->pb.adpcm.a[1][2] = *++p;
        axvpb->pb.adpcm.a[1][3] = *++p;
        axvpb->pb.adpcm.a[1][4] = *++p;
        axvpb->pb.adpcm.a[1][5] = *++p;
        sync = axvpb->sync;
        axvpb->pb.adpcm.a[1][6] = *++p;
        axvpb->pb.adpcm.a[1][7] = *++p;
        axvpb->pb.adpcm.gain = *++p;
        axvpb->pb.adpcm.pred_scale = *++p;
        axvpb->pb.adpcm.yn1 = *++p;
        axvpb->pb.adpcm.yn2 = *++p;
        axvpb->pb.src.ratioLo = (u16)srcBits;
        axvpb->pb.src.last_samples[3] = type;
        axvpb->pb.src.currentAddressFrac = type;
        axvpb->pb.src.last_samples[0] = type;
        axvpb->pb.src.last_samples[1] = type;
        axvpb->pb.src.last_samples[2] = type;
        sync |= 0x00061000;
        axvpb->sync = sync;
        axvpb->pb.src.ratioHi = (u16)(srcBits >> 16);
        OSRestoreInterrupts(old);
        break;
    }
    case 1: {
        u16 *p1;
        p1 = (u16 *)sound->adpcm;
        endAddr = sound->loopEndAddr;
        currentAddr = sound->currentAddr;
        loopAddr = sound->loopAddr;
        old = OSDisableInterrupts();
        axvpb->pb.addr.loopAddressHi = (u16)(loopAddr >> 16);
        axvpb->pb.addr.currentAddressHi = (u16)(currentAddr >> 16);
        axvpb->pb.addr.loopFlag = type;
        axvpb->pb.addr.loopAddressLo = (u16)loopAddr;
        axvpb->pb.addr.endAddressHi = (u16)(endAddr >> 16);
        axvpb->pb.addr.endAddressLo = (u16)endAddr;
        axvpb->pb.addr.currentAddressLo = (u16)currentAddr;
        axvpb->pb.addr.format = 0;
        axvpb->pb.adpcm.a[0][0] = *p1;
        axvpb->pb.adpcm.a[0][1] = *++p1;
        axvpb->pb.adpcm.a[0][2] = *++p1;
        axvpb->pb.adpcm.a[0][3] = *++p1;
        axvpb->pb.adpcm.a[0][4] = *++p1;
        axvpb->pb.adpcm.a[0][5] = *++p1;
        axvpb->pb.adpcm.a[0][6] = *++p1;
        axvpb->pb.adpcm.a[0][7] = *++p1;
        axvpb->pb.adpcm.a[1][0] = *++p1;
        axvpb->pb.adpcm.a[1][1] = *++p1;
        axvpb->pb.adpcm.a[1][2] = *++p1;
        axvpb->pb.adpcm.a[1][3] = *++p1;
        axvpb->pb.adpcm.a[1][4] = *++p1;
        axvpb->pb.adpcm.a[1][5] = *++p1;
        sync = axvpb->sync;
        axvpb->pb.adpcm.a[1][6] = *++p1;
        axvpb->pb.adpcm.a[1][7] = *++p1;
        axvpb->pb.adpcm.gain = *++p1;
        axvpb->pb.adpcm.pred_scale = *++p1;
        axvpb->pb.adpcm.yn1 = *++p1;
        axvpb->pb.adpcm.yn2 = *++p1;
        axvpb->pb.src.ratioHi = (u16)(srcBits >> 16);
        axvpb->pb.src.ratioLo = (u16)srcBits;
        axvpb->pb.src.currentAddressFrac = 0;
        axvpb->pb.src.last_samples[0] = 0;
        axvpb->pb.src.last_samples[1] = 0;
        axvpb->pb.src.last_samples[2] = 0;
        axvpb->pb.src.last_samples[3] = 0;
        axvpb->pb.adpcmLoop.loop_pred_scale = *++p1;
        axvpb->pb.adpcmLoop.loop_yn1 = *++p1;
        axvpb->pb.adpcmLoop.loop_yn2 = *++p1;
        sync |= 0x00161000;
        axvpb->sync = sync;
        OSRestoreInterrupts(old);
        break;
    }
    case 2:
        endAddr = sound->endAddr;
        loopAddr = sound->loopAddr;
        currentAddr = sound->currentAddr;
        old = OSDisableInterrupts();
        zero = 0;
        axvpb->pb.addr.format = 0x0A;
        axvpb->pb.addr.loopAddressHi = (u16)(loopAddr >> 16);
        axvpb->pb.addr.loopAddressLo = (u16)loopAddr;
        axvpb->pb.addr.endAddressHi = (u16)(endAddr >> 16);
        axvpb->pb.addr.endAddressLo = (u16)endAddr;
        axvpb->pb.addr.currentAddressHi = (u16)(currentAddr >> 16);
        axvpb->pb.addr.currentAddressLo = (u16)currentAddr;
        axvpb->pb.adpcm.gain = 0x800;
        axvpb->pb.src.ratioHi = (u16)(srcBits >> 16);
        axvpb->pb.src.ratioLo = (u16)srcBits;
        axvpb->pb.addr.loopFlag = zero;
        goto zero_adpcm;
    case 3:
        endAddr = sound->loopEndAddr;
        loopAddr = sound->loopAddr;
        currentAddr = sound->currentAddr;
        old = OSDisableInterrupts();
        zero = 0;
        axvpb->pb.addr.loopFlag = 1;
        axvpb->pb.addr.format = 0x0A;
        axvpb->pb.addr.loopAddressHi = (u16)(loopAddr >> 16);
        axvpb->pb.addr.loopAddressLo = (u16)loopAddr;
        axvpb->pb.addr.endAddressHi = (u16)(endAddr >> 16);
        axvpb->pb.addr.endAddressLo = (u16)endAddr;
        axvpb->pb.addr.currentAddressHi = (u16)(currentAddr >> 16);
        axvpb->pb.addr.currentAddressLo = (u16)currentAddr;
        axvpb->pb.adpcm.gain = 0x800;
        axvpb->pb.src.ratioHi = (u16)(srcBits >> 16);
        axvpb->pb.src.ratioLo = (u16)srcBits;
        goto zero_adpcm;
    case 4:
        endAddr = sound->endAddr;
        loopAddr = sound->loopAddr;
        currentAddr = sound->currentAddr;
        old = OSDisableInterrupts();
        zero = 0;
        axvpb->pb.addr.format = 0x19;
        axvpb->pb.addr.loopAddressHi = (u16)(loopAddr >> 16);
        axvpb->pb.addr.loopAddressLo = (u16)loopAddr;
        axvpb->pb.addr.endAddressHi = (u16)(endAddr >> 16);
        axvpb->pb.addr.endAddressLo = (u16)endAddr;
        axvpb->pb.addr.currentAddressHi = (u16)(currentAddr >> 16);
        axvpb->pb.addr.currentAddressLo = (u16)currentAddr;
        axvpb->pb.adpcm.gain = 0x100;
        axvpb->pb.src.ratioHi = (u16)(srcBits >> 16);
        axvpb->pb.src.ratioLo = (u16)srcBits;
        axvpb->pb.addr.loopFlag = zero;
    zero_adpcm:
        axvpb->pb.adpcm.a[0][0] = zero;
        axvpb->pb.adpcm.a[0][1] = zero;
        axvpb->pb.adpcm.a[0][2] = zero;
        axvpb->pb.adpcm.a[0][3] = zero;
        axvpb->pb.adpcm.a[0][4] = zero;
        axvpb->pb.adpcm.a[0][5] = zero;
        axvpb->pb.adpcm.a[0][6] = zero;
        axvpb->pb.adpcm.a[0][7] = zero;
        axvpb->pb.adpcm.a[1][0] = zero;
        axvpb->pb.adpcm.a[1][1] = zero;
        axvpb->pb.adpcm.a[1][2] = zero;
        axvpb->pb.adpcm.a[1][3] = zero;
        axvpb->pb.adpcm.a[1][4] = zero;
        axvpb->pb.adpcm.a[1][5] = zero;
        axvpb->pb.adpcm.a[1][6] = zero;
        axvpb->pb.adpcm.a[1][7] = zero;
        axvpb->pb.adpcm.pred_scale = zero;
        axvpb->pb.adpcm.yn1 = zero;
        axvpb->pb.adpcm.yn2 = zero;
        axvpb->pb.src.currentAddressFrac = zero;
        axvpb->pb.src.last_samples[0] = zero;
        axvpb->pb.src.last_samples[1] = zero;
        axvpb->pb.src.last_samples[2] = zero;
        sync = axvpb->sync;
        axvpb->pb.src.last_samples[3] = zero;
        sync |= 0x00061000;
        axvpb->sync = sync;
        OSRestoreInterrupts(old);
        break;
    case 5:
        endAddr = sound->loopEndAddr;
        loopAddr = sound->loopAddr;
        currentAddr = sound->currentAddr;
        old = OSDisableInterrupts();
        zero = 0;
        axvpb->pb.addr.loopFlag = 1;
        axvpb->pb.addr.format = 0x19;
        axvpb->pb.addr.loopAddressHi = (u16)(loopAddr >> 16);
        axvpb->pb.addr.loopAddressLo = (u16)loopAddr;
        axvpb->pb.addr.endAddressHi = (u16)(endAddr >> 16);
        axvpb->pb.addr.endAddressLo = (u16)endAddr;
        axvpb->pb.addr.currentAddressHi = (u16)(currentAddr >> 16);
        axvpb->pb.addr.currentAddressLo = (u16)currentAddr;
        axvpb->pb.adpcm.gain = 0x100;
        axvpb->pb.src.ratioHi = (u16)(srcBits >> 16);
        axvpb->pb.src.ratioLo = (u16)srcBits;
        axvpb->pb.adpcm.a[0][0] = zero;
        axvpb->pb.adpcm.a[0][1] = zero;
        axvpb->pb.adpcm.a[0][2] = zero;
        axvpb->pb.adpcm.a[0][3] = zero;
        axvpb->pb.adpcm.a[0][4] = zero;
        axvpb->pb.adpcm.a[0][5] = zero;
        axvpb->pb.adpcm.a[0][6] = zero;
        axvpb->pb.adpcm.a[0][7] = zero;
        axvpb->pb.adpcm.a[1][0] = zero;
        axvpb->pb.adpcm.a[1][1] = zero;
        axvpb->pb.adpcm.a[1][2] = zero;
        axvpb->pb.adpcm.a[1][3] = zero;
        axvpb->pb.adpcm.a[1][4] = zero;
        axvpb->pb.adpcm.a[1][5] = zero;
        axvpb->pb.adpcm.a[1][6] = zero;
        axvpb->pb.adpcm.a[1][7] = zero;
        axvpb->pb.adpcm.pred_scale = zero;
        axvpb->pb.adpcm.yn1 = zero;
        axvpb->pb.adpcm.yn2 = zero;
        axvpb->pb.src.currentAddressFrac = zero;
        axvpb->pb.src.last_samples[0] = zero;
        axvpb->pb.src.last_samples[1] = zero;
        axvpb->pb.src.last_samples[2] = zero;
        sync = axvpb->sync;
        axvpb->pb.src.last_samples[3] = zero;
        sync |= 0x00061000;
        axvpb->sync = sync;
        OSRestoreInterrupts(old);
        break;
    }
}

void SPPrepareEnd(SPSoundEntry *sound, AXVPB *axvpb) {
    int old;
    u32 sync;
    old = OSDisableInterrupts();
    axvpb->pb.addr.loopFlag = 0;
    sync = axvpb->sync;
    sync |= 0xA000;
    axvpb->pb.addr.endAddressHi = ((u16 *)&sound->endAddr)[0];
    axvpb->pb.addr.endAddressLo = ((u16 *)&sound->endAddr)[1];
    axvpb->sync = sync;
    OSRestoreInterrupts(old);
}

unsigned int SS_TrackAdd(char *name, void (*cb)(unsigned int)) {
    return DTKQueueTrack(name, &SS_DTKTrack, 0x3F, cb);
}

void SS_TrackPlay(int channel) {
    SS_CurrentChannel = channel;
    if (DTKGetCurrentTrack() != 0) {
        while (DTKGetState() == 3) {}
        DTKSetState(1);
    }
    SS_TrackPlayingFlag = 1;
}

void SS_TrackStop(int channel) {
    if (channel == -1 || SS_CurrentChannel == channel) {
        if (DTKGetCurrentTrack() != 0) {
            while (DTKGetState() == 3) {
                GC_DiskErrorPoll();
            }
            DTKSetState(0);
        }
        SS_CurrentChannel = -1;
        SS_TrackPlayingFlag = 0;
    }
}

void SS_TrackSetRepeat(unsigned int repeat_mode) {
    DTKSetRepeatMode(repeat_mode);
}

void SS_TrackFlushAll(void) {
    if (DTKGetCurrentTrack() != 0) {
        flush_flag = 0;
        while (DTKGetState() == 3) {}
        DTKFlushTracks(__flush_callback_800CEAA0);
        while (flush_flag == 0) {}
    }
}