#ifndef NUSOUND_H
#define NUSOUND_H

#include "types.h"
#include "nusoundtypes.h"

/*
  800c20d0 000250 800c20d0  4 NuSoundApply3d 	Global
  800c2320 000080 800c2320  4 NuSoundFindLoopInfo 	Global
  800c23a0 000030 800c23a0  4 NuSoundInitLoopInfo 	Global
  800c23d0 000020 800c23d0  4 NuSoundInit 	Global
  800c23f0 000024 800c23f0  4 NuSoundPlay 	Global
  800c2414 0000b0 800c2414  4 NuSoundPlayLoop 	Global
  800c24c4 000060 800c24c4  4 NuSoundPlay3d 	Global
  800c2524 0000cc 800c2524  4 NuSoundPlay3dLoopSfx 	Global
  800c25f0 00002c 800c25f0  4 NuSoundPlayStereo 	Global
  800c261c 00002c 800c261c  4 NuSoundPlayStereo2 	Global
  800c2648 000028 800c2648  4 NuSoundStopStream 	Global
  800c2670 0000e8 800c2670  4 NuSoundUpdate 	Global
  800c2758 000030 800c2758  4 NuXboxSoundUpdate 	Global
  800c2788 000020 800c2788  4 NuSoundLocalSet 	Global
  800c27a8 0000cc 800c27a8  4 NuSoundSetLevelAmbience 	Global
  800c2874 00002c 800c2874  4 NuSoundKillAllAudio 	Global
  800c28a0 000020 800c28a0  4 NuSoundPauseSfx 	Global
*/

extern u32 SS_TrackPlayingFlag;
extern s32 SS_CurrentChannel;

extern struct NuSndLoopInfo_s NuSndLoopInfo[8];
extern struct nuvec_s dummypos;
extern f32 nusound_fade_start;
extern f32 nusound_fade_end;

extern struct nucamera_s global_camera;
extern struct creature_s *player;
extern s32 Level;
extern struct game_s Game;

f32 NuFsqrt(f32 x);

s16 SS_PlaySFX(s32 SSample, s32 VolL, s32 VolR, s32 Pitch, ...);
void SS_SetChannelVolume(s32 channel, s32 VolL, s32 VolR, ...);
void SS_SetChannelPitch(s32 channel, s32 Pitch, ...);
void SS_TrackStop(s32 channel, ...);
void PlayStream(s32 track, s32 vol, s32 channel, ...);
void SS_StopAllSFX(void);
void SS_PauseSFX(void);
void SS_ResumeSFX(void);
void SS_LoadBank(s32 bank);
void SS_Update(u8 val, ...);

struct NuSndLoopInfo_s* NuSoundFindLoopInfo(struct nuvec_s *id);
void NuSoundInitLoopInfo(void);
void NuSoundInit(void);
void NuSoundPlay(s32 SSample, s32 VolL, s32 VolR, s32 Pitch);
void NuSoundPlayLoop(s32 SSample, s32 VolL, s32 VolR, s32 Pitch);
void NuSoundPlay3d(struct nuvec_s *pos, int SSample, int VolL, int VolR, int Pitch);
void NuSoundPlay3dLoopSfx(struct nuvec_s *pos, s32 SSample, s32 VolL, s32 VolR, s32 Pitch);
void NuSoundPlayStereo(s32 track, s32 unused, s32 vol);
void NuSoundPlayStereo2(s32 track, s32 unused, s32 vol);
s32 NuSoundStopStream(s32 SPUChannel);
void NuSoundUpdate(void);
void NuXboxSoundUpdate(void);
void NuSoundLocalSet(s32 bank);
void NuSoundSetLevelAmbience(void);
void NuSoundKillAllAudio(void);
void NuSoundPauseSfx(void);
void NuSoundResumeSfx(void);
void NuSoundPlayChan(u32 track, u8 leftVol, u8 rightVol, u32 pitch, s32 channel);
u32 NuSoundKeyStatus(s32 channel);

#endif // !NUSOUND_H
