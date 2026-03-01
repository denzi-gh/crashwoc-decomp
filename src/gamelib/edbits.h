#ifndef EDBITS_H
#define EDBITS_H

#include "../types.h"
#include "../nu.h"

typedef struct pSFX pSFX, *PpSFX;

struct pSFX {
    char name[16];
    u16 pitch;
    u16 volume;
    u8 buzz;
    u8 rumble;
    u8 delay;
    u8 wait;
    char *path;
    u16 frequency;
    u16 stream;
    char type;
    char pad1;
    u16 id;
    struct nuvec_s Pos;
};

extern u32 edqseed;
extern u32 edbitsSfxVol;
extern struct pSFX *edSfxGlobalTab;
extern struct pSFX *edSfxLevelTab;
extern u32 edSfxGlobalCount;
extern u32 edSfxAllCount;
extern char edbits_what_game;
extern struct nugscn_s *edbits_base_scene;

u32 edqrand(void);
void edbitsRegisterSfx(struct pSFX *sfxGlobalTab, struct pSFX *sfxLevelTab, u32 sfxGlobalCount, u32 sfxAllCount);
void edbitsRegisterBaseScene(struct nugscn_s *s);
void edbitsSetSoundFxVolume(u32 vol);
s32 edbitsLookupSoundFX(char *name);
void edbitsSoundPlay(struct nuvec_s *pos, s32 sid);
s32 edbitsLookupInstance(char *name);

#endif // !EDBITS_H
