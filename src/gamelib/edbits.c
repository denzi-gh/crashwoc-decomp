#include "edbits.h"

u32 edqseed = 0x3039;
u32 edbitsSfxVol = 0x64;
struct pSFX *edSfxGlobalTab;
struct pSFX *edSfxLevelTab;
u32 edSfxGlobalCount;
u32 edSfxAllCount;
struct nugscn_s *edbits_base_scene;

extern s32 gamesfx_pitch;
extern s32 gamesfx_effect_volume;
extern s32 gamesfx_edbits;
extern s32 debris_sfx;
extern char edbits_what_game;

void GameSfx(s32 sfx, struct nuvec_s *pos);

u32 edqrand(void)
{
	edqseed = (edqseed * 0x24cd + 1) & 0xffff;
	return edqseed;
}

void edbitsRegisterSfx(struct pSFX *sfxGlobalTab, struct pSFX *sfxLevelTab, u32 sfxGlobalCount, u32 sfxAllCount)
{
	edSfxGlobalTab = sfxGlobalTab;
	edSfxLevelTab = sfxLevelTab;
	edSfxGlobalCount = sfxGlobalCount;
	edSfxAllCount = sfxAllCount;
}

void edbitsRegisterBaseScene(struct nugscn_s *s)
{
	edbits_base_scene = s;
}

void edbitsSetSoundFxVolume(u32 vol)
{
	edbitsSfxVol = vol;
}


s32 edbitsLookupSoundFX(char *name)	//CHECK

{
  s32 cmp;
  s32 i;
  s32 j;
  
  if ((edSfxLevelTab != NULL) && (j = 0, 0 < (s32)edSfxAllCount - (s32)edSfxGlobalCount)) {
    i = 0;
    do {
      cmp = strncmp((char *)edSfxLevelTab + i, name, 0xf);
      if (cmp == 0) {
        return j + edSfxGlobalCount;
      }
      j = j + 1;
      i = i + 0x30;
    } while (j < (s32)edSfxAllCount - (s32)edSfxGlobalCount);
  }
  if ((edSfxGlobalTab != 0) && (j = 0, j < (s32)edSfxGlobalCount)) {
    i = 0;
    do {
      cmp = strncmp((char *)edSfxGlobalTab + i, name, 0xf);
      if (cmp == 0) {
        return j;
      }
      j = j + 1;
      i = i + 0x30;
    } while (j < (s32)edSfxGlobalCount);
  }
  return -1;
}

void edbitsSoundPlay(struct nuvec_s *pos, s32 sid)		//CHECK

{
  struct pSFX *SFXTab;
  s32 tsid;
  
  if (edbits_what_game == '\x02') {
    if (sid < (s32)edSfxGlobalCount) {
      tsid = sid * 0x30;
      SFXTab = edSfxGlobalTab;
    }
    else {
      tsid = (sid - edSfxGlobalCount) * 0x30;
      SFXTab = edSfxLevelTab;
    }
    SFXTab = (struct pSFX *)((char *)SFXTab + tsid);
    gamesfx_pitch = SFXTab->pitch;
    gamesfx_effect_volume = SFXTab->volume;
    gamesfx_edbits = 1;
    GameSfx(sid, pos);
    debris_sfx = 0;
  }
  else if (sid < (s32)edSfxGlobalCount) {
    tsid = (s32)((u32)edSfxGlobalTab[sid].volume * edbitsSfxVol) / 100;
    NuSoundPlay3d(pos, sid, tsid, tsid, (u32)edSfxGlobalTab[sid].pitch);
  }
  else {
    tsid = (s32)((u32)edSfxLevelTab[sid - (s32)edSfxGlobalCount].volume * edbitsSfxVol) / 100;
    NuSoundPlay3d(pos, sid, tsid, tsid, (u32)edSfxLevelTab[sid - (s32)edSfxGlobalCount].pitch);
  }
  return;
}


s32 edbitsLookupInstance(char *name)		//CHECK

{
  s32 cmp;
  s32 cnt;
  
  if (edbits_base_scene != NULL) {
    cnt = 0;
    if (cnt < edbits_base_scene->numspecial) {
      do {
        cmp = strncmp(edbits_base_scene->specials[cnt].name, name, 0x13);
        if (cmp == 0) {
          return cnt;
        }
        cnt = cnt + 1;
      } while (cnt < edbits_base_scene->numspecial);
    }
  }
  return -1;
}
