#include "types.h"

typedef int BOOL;
#define TRUE 1
#define FALSE 0

typedef void (*OSResetCallback)(void);

typedef struct OSResetFunctionInfo {
    int (*func)(int);
    u32 priority;
    struct OSResetFunctionInfo* next;
    struct OSResetFunctionInfo* prev;
} OSResetFunctionInfo;

typedef struct OSThread OSThread;
struct OSThread {
    char context[0x2C8];
    u16 state;
    u16 attr;
    s32 suspend;
    s32 priority;
    s32 base;
    void* val;
    void* queue;
    struct { OSThread* next; OSThread* prev; } link;
    struct { OSThread* head; OSThread* tail; } queueJoin;
    void* mutex;
    struct { void* head; void* tail; } queueMutex;
    struct { OSThread* next; OSThread* prev; } linkActive;
};

s32 OSDisableInterrupts(void);
s32 OSRestoreInterrupts(s32);
void __OSStopAudioSystem(void);
void OSDisableScheduler(void);
void OSEnableScheduler(void);
BOOL __PADDisableRecalibration(BOOL);
void ICFlashInvalidate(void);
void LCDisable(void);
BOOL __OSSyncSram(void);
void* __OSLockSram(void);
BOOL __OSUnlockSram(BOOL);
void __OSMaskInterrupts(u32);
void OSCancelThread(OSThread*);
void __OSReboot(u32, u32);
s64 __OSGetSystemTime(void);
s64 __div2i(s64, s64);

#define OS_BUS_CLOCK (*(volatile u32*)0x800000F8)
#define OS_TIMER_CLOCK (OS_BUS_CLOCK / 4)

extern s32 Down;
extern OSResetCallback ResetCallback;
extern s32 LastState;
extern s64 HoldUp;
extern s64 HoldDown;
extern s64 __OSStartTime;

static OSResetFunctionInfo* ResetFunctionQueue;

static void Reset(u32 resetCode) {
    /* Cannot be implemented in pure C - requires mfspr/mtspr/isync/sync/mftb */
}

void __OSDoHotReset(u32 resetCode) {
    OSDisableInterrupts();
    *(volatile u16*)0xCC002002 = 0;
    ICFlashInvalidate();
    Reset(resetCode << 3);
}

void OSResetSystem(int reset, u32 resetCode, BOOL forceMenu) {
    BOOL padRecal;
    OSResetFunctionInfo* info;
    BOOL retVal;
    BOOL complete;
    OSThread* thread;
    OSThread* next;
    u32* base;

    OSDisableScheduler();
    __OSStopAudioSystem();

    if (reset == 2) {
        padRecal = __PADDisableRecalibration(TRUE);
    }

    while (TRUE) {
        while (TRUE) {
            info = ResetFunctionQueue;
            retVal = FALSE;

            while (TRUE) {
                while (info != NULL) {
                    if (!info->func(FALSE)) {
                        retVal = TRUE;
                    }
                    info = info->next;
                }

                if (!__OSSyncSram()) {
                    retVal = TRUE;
                }

                if (retVal == FALSE) {
                    complete = TRUE;
                } else {
                    complete = FALSE;
                }

                if (complete) break;
                break;
            }
            if (complete) break;
        }
        break;
    }

    if (reset == 1 && forceMenu != 0) {
        u8* sram = (u8*)__OSLockSram();
        sram[0x13] |= 0x40;
        __OSUnlockSram(TRUE);

        while (TRUE) {
            while (!__OSSyncSram())
                ;
            break;
        }
    }

    OSDisableInterrupts();
    info = (OSResetFunctionInfo*)ResetFunctionQueue;
    retVal = FALSE;

    while (TRUE) {
        while (info != NULL) {
            if (!info->func(TRUE)) {
                retVal = TRUE;
            }
            info = info->next;
        }
        break;
    }

    __OSSyncSram();
    LCDisable();

    if (reset == 1) {
        OSDisableInterrupts();
        *(volatile u16*)0xCC002002 = 0;
        ICFlashInvalidate();
        Reset(resetCode << 3);
    } else if (reset == 0) {
        thread = *(OSThread* volatile*)0x800000DC;

        while (TRUE) {
            while (thread != NULL) {
                u16 state = thread->state;
                next = thread->linkActive.next;
                if (state == 4 || state == 1) {
                    OSCancelThread(thread);
                }
                thread = next;
            }
            break;
        }

        OSEnableScheduler();
        __OSReboot(resetCode, forceMenu);
    }

    thread = *(OSThread* volatile*)0x800000DC;

    while (TRUE) {
        while (thread != NULL) {
            u16 state = thread->state;
            next = thread->linkActive.next;
            if (state == 4 || state == 1) {
                OSCancelThread(thread);
            }
            thread = next;
        }
        break;
    }

    base = (u32*)0x80000000;
    memset(base + 0x10, 0, 0x8C);
    memset(base + 0x35, 0, 0x14);
    memset(base + 0x3D, 0, 0x4);
    memset((void*)(base + 0xC00), 0, 0xC0);
    memset((void*)(base + 0xC32), 0, 0xC);

    __PADDisableRecalibration(padRecal);
}

u32 OSGetResetCode(void) {
    if (*(volatile u8*)0x800030E2 != 0) {
        return (u32)0x80000000;
    }
    return (*(volatile u32*)0xCC003024 & ~0x7) >> 3;
}

void __OSResetSWInterruptHandler(void) {
    s64 now;
    u32 threshold;

    now = __OSGetSystemTime();
    HoldDown = now;

    threshold = OS_TIMER_CLOCK / 10000;

    while (TRUE) {
        s64 elapsed;
        now = __OSGetSystemTime();
        elapsed = now - HoldDown;

        if (elapsed >= (s64)threshold) break;
        if (*(volatile u32*)0xCC003000 & 0x00010000) break;
    }

    if (!(*(volatile u32*)0xCC003000 & 0x00010000)) {
        Down = TRUE;
        LastState = TRUE;
        __OSMaskInterrupts(0x200);

        if (ResetCallback != NULL) {
            OSResetCallback cb = ResetCallback;
            ResetCallback = NULL;
            cb();
        }
    }

    *(volatile u32*)0xCC003000 = 2;
}

BOOL OSGetResetButtonState(void) {
    BOOL result;
    s32 old;
    s64 now;

    old = OSDisableInterrupts();
    now = __OSGetSystemTime();

    if (!(*(volatile u32*)0xCC003000 & 0x00010000)) {
        if (Down == 0) {
            BOOL valid = TRUE;
            if (HoldUp != 0) {
                valid = FALSE;
            }
            Down = TRUE;
            result = valid;
            HoldDown = now;
        } else {
            BOOL valid = TRUE;
            if (HoldUp == 0) {
                u32 busClock = OS_BUS_CLOCK;
                u32 timerClock = busClock >> 2;
                u32 ticks100 = timerClock / 125000;
                s64 elapsed = now - HoldDown;
                ticks100 *= 100;
                ticks100 >>= 3;
                if (elapsed < (s64)ticks100) {
                    valid = FALSE;
                }
            }
            if (valid) {
                result = TRUE;
            } else {
                result = FALSE;
            }
        }
    } else {
        if (Down != 0) {
            s32 last = LastState;
            Down = 0;
            result = last;
            if (last) {
                HoldUp = now;
            } else {
                HoldUp = 0;
            }
        } else {
            if (HoldUp != 0) {
                u32 busClock = *(volatile u32*)0x800000F8;
                u32 timerClock = busClock >> 2;
                u32 holdThreshold = timerClock / 1000 * 40;
                s64 elapsed = now - HoldUp;

                if (elapsed >= (s64)holdThreshold) {
                    result = TRUE;
                } else {
                    HoldUp = 0;
                    result = FALSE;
                }
            } else {
                HoldUp = 0;
                result = FALSE;
            }
        }
    }

    LastState = result;

    {
        u8 bootByte = *(volatile u8*)0x800030E3;
        if (bootByte & 0x3F) {
            u32 timerClock = OS_TIMER_CLOCK;
            s64 deadline = __OSStartTime + (s64)timerClock * (s64)(bootByte & 0x3F) * 60;
            if (now >= deadline) {
                s64 diff = now - deadline;
                s64 halved = diff / timerClock / 2;
                if ((halved & 1) == 0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }
            }
        }
    }

    OSRestoreInterrupts(old);
    return result;
}
