#include "nusound.h"
#include "nu3dxtypes.h"
#include "game.h"
#include "creature.h"

u32 SS_TrackPlayingFlag = 0;
s32 SS_CurrentChannel = -1;

static s32 NuSoundApply3d(struct nuvec_s *pos, s32 *volL, s32 *volR)
{
	f32 dx, dy, dz;
	f32 dist;
	f32 fade;
	f32 fVolL, fVolR;
	struct nuvec4_s viewpos;
	f32 pan;
	f32 left, right;

	dy = pos->y - global_camera.mtx._31;
	dx = pos->x - global_camera.mtx._30;
	dz = pos->z - global_camera.mtx._32;

	dist = NuFsqrt(dx * dx + dy * dy + dz * dz);

	if (dist > nusound_fade_end) {
		return 0;
	}

	if (dist > nusound_fade_start) {
		fade = (nusound_fade_end - dist) / (nusound_fade_end - nusound_fade_start);
	} else {
		fade = 1.0f;
	}

	fVolL = (f32)*volL;
	fVolR = (f32)*volR;

	viewpos.x = pos->x;
	viewpos.y = pos->y;
	viewpos.z = pos->z;
	viewpos.w = 1.0f;

	NuVec4MtxTransform(&viewpos, (struct nuvec_s *)&viewpos, NuCameraGetMtx());

	if (viewpos.w != 0.0f) {
		viewpos.x = viewpos.x / viewpos.w;
	}

	if (viewpos.w < 0.0f) {
		viewpos.x = -viewpos.x;
	}

	pan = viewpos.x;
	if (pan > 1.0f || pan < -1.0f) {
		if (pan > 1.0f) {
			pan = 1.0f;
		} else {
			pan = -1.0f;
		}
	}

	viewpos.x = pan;
	left = (1.0f - pan) * 0.5f;
	right = (1.0f + pan) * 0.5f;
	left *= fade;
	right *= fade;
	fVolL *= left;
	fVolR *= right;

	if (fVolL > 16383.0f) fVolL = 16383.0f;
	if (fVolR > 16383.0f) fVolR = 16383.0f;
	if (fVolL < -16383.0f) fVolL = -16383.0f;
	if (fVolR < -16383.0f) fVolR = -16383.0f;

	*volL = (s32)fVolL;
	*volR = (s32)fVolR;

	return 1;
}

struct NuSndLoopInfo_s* NuSoundFindLoopInfo(struct nuvec_s *id)
{
	s32 freeSlot;
	s32 i;
	struct NuSndLoopInfo_s *p;
	struct NuSndLoopInfo_s *ret;

	freeSlot = -1;
	p = NuSndLoopInfo;
	for (i = 0; i < 8; i++) {
		ret = p;
		if (p->timer == 0) {
			freeSlot = i;
		}
		if (p->pos == id) {
			return ret;
		}
		p++;
	}

	if (freeSlot == -1) {
		return 0;
	}

	ret = &NuSndLoopInfo[freeSlot];
	ret->pos = id;
	ret->playing = 0;
	return ret;
}

void NuSoundInitLoopInfo(void)
{
	s32 i;
	struct NuSndLoopInfo_s *p = NuSndLoopInfo;

	for (i = 0; i < 8; i++) {
		p->pos = 0;
		p->timer = 0;
		p->playing = 0;
		p->channel = 0;
		p++;
	}
}

void NuSoundInit(void)
{
	NuSoundInitLoopInfo();
}

void NuSoundPlay(s32 SSample, s32 VolL, s32 VolR, s32 Pitch)
{
	SS_PlaySFX(SSample, VolL, VolR, Pitch);
}

void NuSoundPlayLoop(s32 SSample, s32 VolL, s32 VolR, s32 Pitch)
{
	struct NuSndLoopInfo_s *info;

	info = NuSoundFindLoopInfo(&dummypos);
	if (info != NULL) {
		if (info->playing == 0) {
			info->playing = 1;
			info->channel = SS_PlaySFX(SSample, VolL, VolR, Pitch);
		} else {
			SS_SetChannelVolume(info->channel, VolL, VolR);
			SS_SetChannelPitch(info->channel, Pitch);
		}
		info->vol_l = VolL;
		info->timer = 12;
		info->vol_r = VolR;
	}
}

void NuSoundPlay3d(struct nuvec_s *pos, int SSample, int VolL, int VolR, int Pitch)
{
	if (NuSoundApply3d(pos, &VolL, &VolR)) {
		SS_PlaySFX(SSample, VolL, VolR, Pitch);
	}
}

void NuSoundPlay3dLoopSfx(struct nuvec_s *pos, s32 SSample, s32 VolL, s32 VolR, s32 Pitch)
{
	struct NuSndLoopInfo_s *info;
	s32 saveVolL = VolL;
	s32 saveVolR = VolR;

	if (!NuSoundApply3d(pos, &VolL, &VolR)) {
		return;
	}

	info = NuSoundFindLoopInfo(pos);
	if (info == NULL) {
		return;
	}

	if (info->playing == 0) {
		info->playing = 1;
		info->channel = SS_PlaySFX(SSample, VolL, VolR, Pitch);
	} else {
		SS_SetChannelVolume(info->channel, VolL, VolR);
		SS_SetChannelPitch(info->channel, Pitch);
	}
	info->vol_l = saveVolL;
	info->timer = 12;
	info->vol_r = saveVolR;
}

void NuSoundPlayStereo(s32 track, s32 unused, s32 vol)
{
	PlayStream(track, vol, 0);
}

void NuSoundPlayStereo2(s32 track, s32 unused, s32 vol)
{
	PlayStream(track, vol, 2);
}

s32 NuSoundStopStream(s32 SPUChannel)
{
	SS_TrackStop(SPUChannel);
	return 1;
}

void NuSoundUpdate(void)
{
	struct NuSndLoopInfo_s *p;
	struct NuSndLoopInfo_s *end;
	s32 adjVolL, adjVolR;

	p = NuSndLoopInfo;
	end = &NuSndLoopInfo[7];

	do {
		if (p->timer != 0) {
			p->timer--;
			if ((s16)p->timer == 0) {
				p->playing = 0;
			}
			if (p->timer <= 7) {
				adjVolL = (p->vol_l * p->timer) / 8;
				adjVolR = (p->vol_r * p->timer) / 8;
				if (p->pos != NULL && NuSoundApply3d(p->pos, &adjVolL, &adjVolR) == 0) {
					SS_SetChannelVolume(p->channel, 0, 0);
				} else {
					SS_SetChannelVolume(p->channel, adjVolL, adjVolR);
				}
			}
		}
		p++;
	} while (p <= end);
}

void NuXboxSoundUpdate(void)
{
	NuSoundUpdate();
	SS_Update(Game.music_volume);
}

void NuSoundLocalSet(s32 bank)
{
	SS_LoadBank(bank);
}

void NuSoundSetLevelAmbience(void)
{
	s8 iRAIL;
	s16 iALONG;

	iRAIL = (s8)player->obj.RPos.iRAIL;
	iALONG = player->obj.RPos.iALONG;

	if (Level == 18) return;
	if (Level > 18) {
		if (Level == 28) return;
		if (Level > 28) {
			if (Level == 33) return;
			if (Level <= 33) return;
		} else {
			if (Level > 26) return;
			if (Level >= 21) return;
			if (Level == 19) {
				if (iRAIL == 0) {}
			}
		}
	} else {
		if (Level == 7) return;
		if (Level > 7) {
			if (Level == 13) return;
			if (Level > 13) {
				if (Level == 15) return;
			} else {
				if (Level == 8) {
					if (iRAIL != 0) return;
					if ((u32)(iALONG - 77) <= 2) return;
				}
			}
		} else {
			if (Level == 2) return;
			if (Level > 2) {
				if (Level == 4) return;
			} else {
				if (Level == 1) return;
			}
		}
	}
}

void NuSoundKillAllAudio(void)
{
	SS_StopAllSFX();
	SS_TrackStop(-1);
}

void NuSoundPauseSfx(void)
{
	SS_PauseSFX();
}

void NuSoundResumeSfx(void)
{
	SS_ResumeSFX();
}

void NuSoundPlayChan(u32 track, u8 leftVol, u8 rightVol, u32 pitch, s32 channel)
{
	PlayStream(track, leftVol, channel);
}

u32 NuSoundKeyStatus(s32 channel)
{
	if ((SS_CurrentChannel == channel) && (SS_TrackPlayingFlag != 0)) {
		return PLAYING;
	}
	return STOPPED;
}
