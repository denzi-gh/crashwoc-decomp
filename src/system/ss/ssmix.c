#include "ssmix.h"

s32 __MIXPanTableL[128] = {
              0,            0,           -1,           -1,
             -1,           -2,           -2,           -2,
             -3,           -3,           -4,           -4,
             -4,           -5,           -5,           -5,
             -6,           -6,           -7,           -7,
             -7,           -8,           -8,           -9,
             -9,          -10,          -10,          -10,
            -11,          -11,          -12,          -12,
            -13,          -13,          -14,          -14,
            -14,          -15,          -15,          -16,
            -16,          -17,          -17,          -18,
            -18,          -19,          -20,          -20,
            -21,          -21,          -22,          -22,
            -23,          -23,          -24,          -25,
            -25,          -26,          -26,          -27,
            -28,          -28,          -29,          -30,
            -30,          -31,          -32,          -33,
            -33,          -34,          -35,          -36,
            -36,          -37,          -38,          -39,
            -40,          -40,          -41,          -42,
            -43,          -44,          -45,          -46,
            -47,          -48,          -49,          -50,
            -51,          -52,          -54,          -55,
            -56,          -57,          -59,          -60,
            -61,          -63,          -64,          -66,
            -67,          -69,          -71,          -72,
            -74,          -76,          -78,          -80,
            -83,          -85,          -87,          -90,
            -93,          -96,          -99,         -102,
           -106,         -110,         -115,         -120,
           -126,         -133,         -140,         -150,
           -163,         -180,         -210,         -904
};

s32 __MIXPanTableR[128] = {
           -904,         -210,         -180,         -163,
           -150,         -140,         -133,         -126,
           -120,         -115,         -110,         -106,
           -102,          -99,          -96,          -93,
            -90,          -87,          -85,          -83,
            -80,          -78,          -76,          -74,
            -72,          -71,          -69,          -67,
            -66,          -64,          -63,          -61,
            -60,          -59,          -57,          -56,
            -55,          -54,          -52,          -51,
            -50,          -49,          -48,          -47,
            -46,          -45,          -44,          -43,
            -42,          -41,          -40,          -40,
            -39,          -38,          -37,          -36,
            -36,          -35,          -34,          -33,
            -33,          -32,          -31,          -30,
            -30,          -29,          -28,          -28,
            -27,          -26,          -26,          -25,
            -25,          -24,          -23,          -23,
            -22,          -22,          -21,          -21,
            -20,          -20,          -19,          -18,
            -18,          -17,          -17,          -16,
            -16,          -15,          -15,          -14,
            -14,          -14,          -13,          -13,
            -12,          -12,          -11,          -11,
            -10,          -10,          -10,           -9,
             -9,           -8,           -8,           -7,
             -7,           -7,           -6,           -6,
             -5,           -5,           -5,           -4,
             -4,           -4,           -3,           -3,
             -2,           -2,           -2,           -1,
             -1,           -1,            0,            0
};

u16 __MIXGetVolume(s32 db)
{
    if (db <= -904) {
        return 0;
    }
    if (db > 59) {
        return 0xFF64;
    }
    return __MIXVolumeTable[db + 904];
}

s32 __MIXGetPanL(s32 pan)
{
    return __MIXPanTableL[pan];
}

s32 __MIXGetPanR(s32 pan)
{
    return __MIXPanTableR[pan];
}

void __MIXResetChannel(MIX *channel)
{
    channel->mode = 0x50000000;
    channel->input = 0;
    channel->auxA = -960;
    channel->auxB = -960;
    channel->l = __MIXGetPanL(64);
    channel->r = __MIXGetPanR(64);
    channel->f = __MIXGetPanR(0);
    channel->b = __MIXGetPanL(0);
    channel->pan = 64;
    channel->span = 127;
    channel->fader = 0;
    channel->vBS = 0;
    channel->vBR = 0;
    channel->vBL = 0;
    channel->vAS = 0;
    channel->vAR = 0;
    channel->vAL = 0;
    channel->vS = 0;
    channel->vR = 0;
    channel->vL = 0;
    channel->v = 0;
}

void __MIXSetPan(MIX *channel)
{
    channel->l = __MIXGetPanL(channel->pan);
    channel->r = __MIXGetPanR(channel->pan);
    channel->f = __MIXGetPanR(channel->span);
    channel->b = __MIXGetPanL(channel->span);
}

void MIXInit(void)
{
    s32 i;

    for (i = 0; i < NUM_MIX_CHANNELS; i++)
    {
        __MIXResetChannel(&__MIXChannel[i]);
    }
    __MIXDvdStreamAttenUser = 0;
    __MIXDvdStreamAttenCurrent = 0;
    __MIXSoundMode = 1;
}

void MIXInitChannel(AXVPB *axvpb, u32 mode, s32 input, s32 auxA, s32 auxB, s32 pan, s32 span, s32 fader)
{
    MIX *c;
    s32 old;
    u16 mixerCtrl;

    c = &__MIXChannel[axvpb->index];
    c->axvpb = axvpb;
    c->span = span;
    c->mode = mode & 7;
    c->auxA = auxA;
    c->auxB = auxB;
    c->pan = pan;
    c->fader = fader;
    c->input = input;
    __MIXSetPan(c);

    if (c->mode & 4) {
        c->v = 0;
    } else {
        c->v = __MIXGetVolume(input);
    }

    if (__MIXSoundMode == 1) {
        c->vL = __MIXGetVolume(c->fader + c->l + c->f);
        c->vR = __MIXGetVolume(c->fader + c->r + c->f);
        c->vS = __MIXGetVolume(c->fader + c->b);

        if (c->mode & 1) {
            c->vAL = __MIXGetVolume(c->auxA + c->l + c->f);
            c->vAR = __MIXGetVolume(c->auxA + c->r + c->f);
            c->vAS = __MIXGetVolume(c->auxA + c->b - 60);
        } else {
            c->vAL = __MIXGetVolume(c->fader + c->auxA + c->l + c->f);
            c->vAR = __MIXGetVolume(c->fader + c->auxA + c->r + c->f);
            c->vAS = __MIXGetVolume(c->fader + c->auxA + c->b - 60);
        }

        if (c->mode & 2) {
            c->vBL = __MIXGetVolume(c->auxB + c->l + c->f);
            c->vBR = __MIXGetVolume(c->auxB + c->r + c->f);
            c->vBS = __MIXGetVolume(c->auxB + c->b - 60);
        } else {
            c->vBL = __MIXGetVolume(c->fader + c->auxB + c->l + c->f);
            c->vBR = __MIXGetVolume(c->fader + c->auxB + c->r + c->f);
            c->vBS = __MIXGetVolume(c->fader + c->auxB + c->b - 60);
        }
    } else {
        c->vL = __MIXGetVolume(c->fader + c->f);
        c->vR = __MIXGetVolume(c->fader + c->f);
        c->vS = __MIXGetVolume(c->fader + c->b);

        if (c->mode & 1) {
            c->vAL = __MIXGetVolume(c->auxA + c->f);
            c->vAR = __MIXGetVolume(c->auxA + c->f);
            c->vAS = __MIXGetVolume(c->auxA + c->b - 60);
        } else {
            c->vAL = __MIXGetVolume(c->fader + c->auxA + c->f);
            c->vAR = __MIXGetVolume(c->fader + c->auxA + c->f);
            c->vAS = __MIXGetVolume(c->fader + c->auxA + c->b - 60);
        }

        if (c->mode & 2) {
            c->vBL = __MIXGetVolume(c->auxB + c->f);
            c->vBR = __MIXGetVolume(c->auxB + c->f);
            c->vBS = __MIXGetVolume(c->auxB + c->b - 60);
        } else {
            c->vBL = __MIXGetVolume(c->fader + c->auxB + c->f);
            c->vBR = __MIXGetVolume(c->fader + c->auxB + c->f);
            c->vBS = __MIXGetVolume(c->fader + c->auxB + c->b - 60);
        }
    }

    old = OSDisableInterrupts();

    axvpb->pb.ve.currentVolume = c->v;
    axvpb->pb.ve.currentDelta = 0;
    axvpb->pb.mix.vL = c->vL;
    axvpb->pb.mix.vDeltaL = 0;
    axvpb->pb.mix.vR = c->vR;
    axvpb->pb.mix.vDeltaR = 0;
    axvpb->pb.mix.vAuxAL = c->vAL;
    axvpb->pb.mix.vDeltaAuxAL = 0;
    axvpb->pb.mix.vAuxAR = c->vAR;
    axvpb->pb.mix.vDeltaAuxAR = 0;
    axvpb->pb.mix.vAuxBL = c->vBL;
    axvpb->pb.mix.vDeltaAuxBL = 0;
    axvpb->pb.mix.vAuxBR = c->vBR;
    axvpb->pb.mix.vDeltaAuxBR = 0;
    axvpb->pb.mix.vAuxBS = c->vBS;
    axvpb->pb.mix.vDeltaAuxBS = 0;
    axvpb->pb.mix.vS = c->vS;
    axvpb->pb.mix.vDeltaS = 0;
    axvpb->pb.mix.vAuxAS = c->vAS;
    axvpb->pb.mix.vDeltaAuxAS = 0;

    mixerCtrl = 0;
    if (axvpb->pb.mix.vAuxAL || axvpb->pb.mix.vAuxAR || axvpb->pb.mix.vAuxAS) {
        mixerCtrl |= 1;
    }
    if (axvpb->pb.mix.vAuxBL || axvpb->pb.mix.vAuxBR || axvpb->pb.mix.vAuxBS) {
        mixerCtrl |= 2;
    }
    if (axvpb->pb.mix.vS || axvpb->pb.mix.vAuxAS || axvpb->pb.mix.vAuxBS) {
        mixerCtrl |= 4;
    }

    axvpb->pb.mixerCtrl = mixerCtrl;
    axvpb->sync |= 0x212;
    OSRestoreInterrupts(old);
}

void MIXReleaseChannel(AXVPB *axvpb)
{
    __MIXChannel[axvpb->index].axvpb = 0;
}

void MIXSetInput(AXVPB *p, s32 dB)
{
    MIX *channel;

    channel = &__MIXChannel[p->index];
    channel->input = dB;
    channel->mode |= 0x10000000;
}
