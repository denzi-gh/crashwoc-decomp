#include "types.h"

extern s32 __OSCurrHeap;

void* OSGetArenaLo(void);
void* OSGetArenaHi(void);
void  OSSetArenaLo(void* lo);
void* OSAllocFromHeap(s32 heap, u32 size);

#define ROUND_UP_32(x)   (((u32)(x) + 31) & ~31)
#define ROUND_DOWN_32(x)  ((u32)(x) & ~31)

void* sbrk(s32 incr) {
    void* arenaLo;
    void* arenaHi;

    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();

    arenaLo = (void*)ROUND_UP_32(arenaLo);
    arenaHi = (void*)ROUND_DOWN_32(arenaHi);
    incr = (s32)ROUND_UP_32(incr);

    if (incr <= (s32)((u32)arenaHi - (u32)arenaLo)) {
        OSSetArenaLo((void*)((u32)arenaLo + incr));
        return arenaLo;
    }
    return OSAllocFromHeap(__OSCurrHeap, (u32)incr);
}
