#include <string.h>
#include <stdio.h>

/* Prevent nusound.h inclusion — sfx.c uses different prototypes */
#define NUSOUND_H

#include "gamecode/main.h"

typedef struct {
    char Filename[32];
    int Pitch;
    int LoopInfo;
    int Type;
    int ID;
} NUSOUND_FILENAME_INFO;

/* NuSound prototypes (sfx.c-specific signatures) */
void NuSoundInit(NUSOUND_FILENAME_INFO *info);
void NuSoundLocalSet(NUSOUND_FILENAME_INFO *info, s32 count, s32 offset);
void NuSoundPlayStereo(s32 track, s32 sfx2, s32 volL, s32 volR, s32 pitch);
void NuSoundPlayStereo2(s32 track, s32 sfx2, s32 volL, s32 volR, s32 pitch);
s32 NuSoundStopStream(s32 channel);
void NuSoundUpdate(void);
void NuSoundPauseSfx(void);
void NuSoundResumeSfx(void);
void NuSoundPlay3d(struct nuvec_s *pos, int SSample, int VolL, int VolR, int Pitch);
void NuSoundPlay3dLoopSfx(struct nuvec_s *pos, s32 SSample, s32 VolL, s32 VolR, s32 Pitch);
void NuSoundPlay(s32 SSample, s32 VolL, s32 VolR, s32 Pitch);
void NuSoundPlayLoop(s32 SSample, s32 VolL, s32 VolR, s32 Pitch);
void NuSoundPlayChan(s32 track, s32 leftVol, s32 rightVol, s32 pitch, s32 channel);

s32 ChaseActive(void);
void edbitsSetSoundFxVolume(u32 vol);

extern f32 nusound_fade_start;
extern f32 nusound_fade_end;

extern struct ldata_s LData[];
extern struct pSFX SfxTabGLOBAL[];

extern struct pSFX SfxTabESKIMO[];
extern struct pSFX SfxTabBURN[];
extern struct pSFX SfxTabOHNO[];
extern struct pSFX SfxTabSMOKEY[];
extern struct pSFX SfxTabREACTOR[];
extern struct pSFX SfxTabAVALANCHE[];
extern struct pSFX SfxTabCORAL[];
extern struct pSFX SfxTabARCTIC[];
extern struct pSFX SfxTabJUNGLE[];
extern struct pSFX SfxTabNATURE[];
extern struct pSFX SfxTabGAUNTLET[];
extern struct pSFX SfxTabBAMBOO[];
extern struct pSFX SfxTabBANZAI[];
extern struct pSFX SfxTabTORNADO[];
extern struct pSFX SfxTabTSUNAMI[];
extern struct pSFX SfxTabFRENZY[];
extern struct pSFX SfxTabMADNESS[];
extern struct pSFX SfxTabLIZARDS[];
extern struct pSFX SfxTabSINKING[];
extern struct pSFX SfxTabGOLD[];
extern struct pSFX SfxTabVOID[];
extern struct pSFX SfxTabROKS[];
extern struct pSFX SfxTabASHES[];
extern struct pSFX SfxTabDRAIN[];
extern struct pSFX SfxTabPRESSURE[];
extern struct pSFX SfxTabCRUNCH[];
extern struct pSFX SfxTabOIDS[];
extern struct pSFX SfxTabKNIGHT[];
extern struct pSFX SfxTabTOWN[];
extern struct pSFX SfxTabICE[];
extern struct pSFX SfxTabHEIGHTS[];
extern struct pSFX SfxTabBALLS[];
extern struct pSFX SfxTabBOWLER[];
extern struct pSFX SfxTabVORTEX[];
extern struct pSFX SfxTabSEA[];
extern struct pSFX SfxTabFRONTEND[];
extern struct pSFX SfxTabGAMEOVER[];
extern struct pSFX SfxTabSPACE[];
extern struct pSFX SfxTabCREDITS[];

extern NUSOUND_FILENAME_INFO SfxInfo[];
extern char sfxpath[];

extern s32 SFXCOUNT_ALL;
extern struct pSFX *CurSfxTabLocal;
extern s32 sfxcount_local;
extern s32 sfxcount_global;
extern s32 game_music;
extern s32 gamesfx_channel;
extern s32 gamesfx_effect_volume;
extern s32 gamesfx_volume;
extern s32 gamesfx_pitch;
extern s32 gamesfx_edbits;
extern s32 testglobalsfx;
extern s32 last_testglobalsfx;
extern s32 testsfx;
extern f32 musicvol_mul;

extern s32 Paused;
extern s32 GameMode;
extern s32 new_level;
extern s32 Bonus;
extern f32 bonus_duration;
extern f32 bonus_time;
extern s32 cutmovie;
extern s32 VEHICLECONTROL;
extern u64 LBIT;

void InitLevelSfxTables(void) {
    s32 i;

    for (i = 0; i < 44; i++) {
        LData[i].pSFX = NULL;
        LData[i].nSFX = 0;
    }

    LData[0].pSFX = SfxTabESKIMO;    LData[0].nSFX = 0xB8;
    LData[1].pSFX = SfxTabBURN;      LData[1].nSFX = 0xBE;
    LData[2].pSFX = SfxTabOHNO;      LData[2].nSFX = 0xB8;
    LData[3].pSFX = SfxTabSMOKEY;    LData[3].nSFX = 0xB5;
    LData[4].pSFX = SfxTabREACTOR;   LData[4].nSFX = 0xB6;
    LData[5].pSFX = SfxTabAVALANCHE; LData[5].nSFX = 0xBA;
    LData[6].pSFX = SfxTabCORAL;     LData[6].nSFX = 0xB7;
    LData[7].pSFX = SfxTabARCTIC;    LData[7].nSFX = 0xB8;
    LData[8].pSFX = SfxTabJUNGLE;    LData[8].nSFX = 0xB8;
    LData[9].pSFX = SfxTabNATURE;    LData[9].nSFX = 0xB8;
    LData[10].pSFX = SfxTabGAUNTLET; LData[10].nSFX = 0xB5;
    LData[11].pSFX = SfxTabBAMBOO;   LData[11].nSFX = 0xB6;
    LData[12].pSFX = SfxTabBANZAI;   LData[12].nSFX = 0xBA;
    LData[13].pSFX = SfxTabTORNADO;  LData[13].nSFX = 0xB4;
    LData[14].pSFX = SfxTabTSUNAMI;  LData[14].nSFX = 0xC1;
    LData[15].pSFX = SfxTabFRENZY;   LData[15].nSFX = 0xBB;
    LData[16].pSFX = SfxTabMADNESS;  LData[16].nSFX = 0xB4;
    LData[17].pSFX = SfxTabLIZARDS;  LData[17].nSFX = 0xBC;
    LData[18].pSFX = SfxTabSINKING;  LData[18].nSFX = 0xC6;
    LData[19].pSFX = SfxTabGOLD;     LData[19].nSFX = 0xBE;
    LData[20].pSFX = SfxTabVOID;     LData[20].nSFX = 0xB6;
    LData[21].pSFX = SfxTabROKS;     LData[21].nSFX = 0xC2;
    LData[22].pSFX = SfxTabASHES;    LData[22].nSFX = 0xC3;
    LData[23].pSFX = SfxTabDRAIN;    LData[23].nSFX = 0xBC;
    LData[24].pSFX = SfxTabPRESSURE; LData[24].nSFX = 0xB9;
    LData[25].pSFX = SfxTabCRUNCH;   LData[25].nSFX = 0xB8;
    LData[26].pSFX = SfxTabOIDS;     LData[26].nSFX = 0xC2;
    LData[27].pSFX = SfxTabKNIGHT;   LData[27].nSFX = 0xB4;
    LData[28].pSFX = SfxTabTOWN;     LData[28].nSFX = 0xB4;
    LData[29].pSFX = SfxTabICE;      LData[29].nSFX = 0xB7;
    LData[30].pSFX = SfxTabHEIGHTS;  LData[30].nSFX = 0xB3;
    LData[31].pSFX = SfxTabBALLS;    LData[31].nSFX = 0xB7;
    LData[32].pSFX = SfxTabBOWLER;   LData[32].nSFX = 0xB4;
    LData[33].pSFX = SfxTabVORTEX;   LData[33].nSFX = 0xB4;
    LData[34].pSFX = SfxTabSEA;      LData[34].nSFX = 0xB6;
    LData[36].pSFX = SfxTabSMOKEY;   LData[36].nSFX = 0xB5;
    LData[37].pSFX = SfxTabFRONTEND; LData[37].nSFX = 0xC5;
    LData[38].pSFX = SfxTabGAMEOVER; LData[38].nSFX = 0xBA;
    LData[40].pSFX = SfxTabSPACE;    LData[40].nSFX = 0xCE;
    LData[43].pSFX = SfxTabCREDITS;  LData[43].nSFX = 0xB3;
}

void InitGlobalSfx(void) {
    struct pSFX *src;
    struct pSFX *end;
    NUSOUND_FILENAME_INFO *dst;

    src = SfxTabGLOBAL;
    end = &SfxTabGLOBAL[0xB0];
    dst = SfxInfo;

    do {
        *(s32 *)dst->Filename = *(s32 *)"SFX\\";
        dst->Filename[4] = '\0';
        strcat((char *)dst, src->path);
        dst->Pitch = src->frequency;
        dst->LoopInfo = src->stream;
        dst->Type = (s32)(s8)src->type;
        dst->ID = src->id;
        src++;
        dst++;
    } while (src <= end);

    SfxInfo[0xB1].Filename[0] = '\0';
    SfxInfo[0xB1].Pitch = 0;
    SfxInfo[0xB1].LoopInfo = 4;
    SfxInfo[0xB1].Type = 0;
    SfxInfo[0xB1].ID = -1;

    NuSoundInit(SfxInfo);
    SFXCOUNT_ALL = 0xB1;
}

void InitLocalSfx(struct pSFX *sfxTab, s32 totalCount) {
    s32 local_count;
    s32 i;
    NUSOUND_FILENAME_INFO *dst;
    struct pSFX *src;

    if (sfxTab == NULL) {
        printf("No local SFX table available!\n");
        return;
    }

    if (totalCount - 0xB1 <= 0) {
        return;
    }

    if (totalCount > 0x12B) {
        totalCount = 0x12B;
    }

    local_count = totalCount - 0xB1;
    sfxcount_local = local_count;

    dst = SfxInfo;
    src = sfxTab;
    i = local_count;

    while (i != 0) {
        *(s32 *)dst->Filename = *(s32 *)"SFX\\";
        dst->Filename[4] = '\0';
        strcat((char *)dst, src->path);
        dst->Pitch = src->frequency;
        dst->LoopInfo = src->stream;
        dst->Type = (s32)(s8)src->type;
        dst->ID = src->id;
        src++;
        dst++;
        i--;
    }

    SfxInfo[local_count].Filename[0] = '\0';
    SfxInfo[local_count].Pitch = 0;
    SfxInfo[local_count].LoopInfo = 4;
    SfxInfo[local_count].Type = 0;
    SfxInfo[local_count].ID = -1;

    NuSoundLocalSet(SfxInfo, local_count, 0xB1);

    CurSfxTabLocal = sfxTab;
    SFXCOUNT_ALL = local_count + 0xB1;

    if (local_count > 0) {
        for (i = 0; i < local_count; i++) {
            CurSfxTabLocal[i].wait = 0;
        }
    }
}

void ResetGameSfx(void) {
    s32 i;

    for (i = 0xB0; i >= 0; i--) {
        SfxTabGLOBAL[i].wait = 0;
    }

    if (CurSfxTabLocal == NULL) return;
    if (SFXCOUNT_ALL <= 0xB1) return;
    for (i = 0; i < SFXCOUNT_ALL - 0xB1; i++) {
        CurSfxTabLocal[i].wait = 0;
    }
}

void UpdateGameSfx(void) {
    s32 i;
    struct pSFX *p;

    p = SfxTabGLOBAL;
    for (i = 0; i < 0xB1; i++) {
        if (p->wait != 0) {
            p->wait--;
        }
        p++;
    }

    if (CurSfxTabLocal == NULL) return;
    if (SFXCOUNT_ALL <= 0xB1) return;
    for (i = 0; i < SFXCOUNT_ALL - 0xB1; i++) {
        if (CurSfxTabLocal[i].wait != 0) {
            CurSfxTabLocal[i].wait--;
        }
    }
}

void GameMusic(s32 sfx, s32 i) {
    struct pSFX *info;
    s32 vol;

    if (sfx < 0) return;
    if (sfx >= SFXCOUNT_ALL - 1) return;

    if (sfx <= 0xB0) {
        info = &SfxTabGLOBAL[sfx];
    } else {
        info = &CurSfxTabLocal[sfx - 0xB1];
    }

    if (info->type == 1) {
        vol = Game.music_volume * 0x1BFF / 100;
    } else {
        vol = info->volume * Game.sfx_volume / 100;
    }

    if (i == 0) {
        strcpy(sfxpath, info->path);
        NuSoundStopStream(0);
        NuSoundStopStream(1);
        NuSoundPlayStereo(sfx, sfx + 1, vol, vol, info->pitch);
    } else {
        strcpy(sfxpath, info->path);
        NuSoundStopStream(2);
        NuSoundStopStream(3);
        NuSoundPlayStereo2(sfx, sfx + 1, vol, vol, info->pitch);
    }

    NuSoundUpdate();
    game_music = sfx;
}

void PauseGameAudio(s32 pause) {
    NuSoundPauseSfx();
}

void ResumeGameAudio(void) {
    NuSoundResumeSfx();
}

void GameSfx(s32 sfx, struct nuvec_s *pos) {
    struct pSFX *info;
    struct nuvec_s camPos;
    s32 vol;
    s32 pitch;
    s32 proximity;
    s32 type;

    camPos = *(struct nuvec_s *)((char *)GameCam + 0x30);

    proximity = 0;

    if (sfx < 0) goto cleanup;
    if (sfx >= SFXCOUNT_ALL) goto cleanup;

    if (sfx <= 0xB0) {
        info = &SfxTabGLOBAL[sfx];
    } else {
        info = &CurSfxTabLocal[sfx - 0xB1];
    }

    if (sfx == 0x81 || sfx == 0x53) {
        if (pos != NULL) {
            f32 dist1 = NuVecDist(&camPos, &info->Pos, NULL);
            f32 dist2 = NuVecDist(&camPos, pos, NULL);
            if (dist2 + 2.0f < dist1) {
                proximity = 1;
            }
        }
    }

    if (info->wait != 0) {
        if (proximity == 0) goto cleanup;
    }

    type = *(u8 *)&info->type;

    if (type == 1) {
        vol = info->volume * Game.music_volume / 100;
    } else {
        if (gamesfx_volume == -1) {
            vol = Game.sfx_volume;
        } else {
            vol = gamesfx_volume;
        }

        if (gamesfx_effect_volume == -1) {
            vol = info->volume * vol / 100;
        } else {
            vol = gamesfx_effect_volume * vol / 100;
        }
    }

    if (gamesfx_pitch == -1) {
        pitch = info->pitch;
    } else {
        pitch = gamesfx_pitch;
    }

    if ((unsigned)(type - 1) <= 1) {
        strcpy(sfxpath, info->path);
    }

    if (pos != NULL) {
        NuSoundPlay3d(pos, sfx, vol, vol, pitch);
    } else if (gamesfx_channel != -1) {
        NuSoundPlayChan(sfx, vol, vol, pitch, gamesfx_channel);
    } else {
        NuSoundPlay(sfx, vol, vol, pitch);
    }

    if (PLAYERCOUNT != 0 && gamesfx_edbits == 0) {
        if (info->buzz != 0) {
            NewBuzz((struct rumble_s *)((char *)player + 0xCA4), info->buzz);
        }
        if (info->rumble != 0) {
            NewRumble((struct rumble_s *)((char *)player + 0xCA4), info->rumble);
        }
    }

    info->wait = info->delay;

    if (pos != NULL) {
        info->Pos = *pos;
    } else {
        info->Pos = camPos;
    }

cleanup:
    gamesfx_pitch = -1;
    gamesfx_edbits = 0;
    gamesfx_effect_volume = -1;
    gamesfx_channel = -1;
    gamesfx_volume = -1;
}

void GameSfxLoop(s32 sfx, struct nuvec_s *pos) {
    struct pSFX *info;
    s32 vol;
    s32 pitch;

    if (sfx < 0) goto cleanup;
    if (sfx >= SFXCOUNT_ALL) goto cleanup;

    if (sfx <= 0xB0) {
        info = &SfxTabGLOBAL[sfx];
    } else {
        info = &CurSfxTabLocal[sfx - 0xB1];
    }

    if (info->wait != 0) goto cleanup;

    if (gamesfx_volume == -1) {
        vol = Game.sfx_volume;
    } else {
        vol = gamesfx_volume;
    }

    if (gamesfx_effect_volume == -1) {
        vol = info->volume * vol / 100;
    } else {
        vol = gamesfx_effect_volume * vol / 100;
    }

    if (gamesfx_pitch == -1) {
        pitch = info->pitch;
    } else {
        pitch = gamesfx_pitch;
    }

    strcpy(sfxpath, info->path);

    if (pos != NULL) {
        NuSoundPlay3dLoopSfx(pos, sfx, vol, vol, pitch);
    } else {
        NuSoundPlayLoop(sfx, vol, vol, pitch);
    }

    if (PLAYERCOUNT != 0 && gamesfx_edbits == 0) {
        if (info->buzz != 0) {
            NewBuzz((struct rumble_s *)((char *)player + 0xCA4), info->buzz);
        }
        if (info->rumble != 0) {
            NewRumble((struct rumble_s *)((char *)player + 0xCA4), info->rumble);
        }
    }

    info->wait = info->delay;

cleanup:
    gamesfx_pitch = -1;
    gamesfx_edbits = 0;
    gamesfx_effect_volume = -1;
    gamesfx_channel = -1;
    gamesfx_volume = -1;
}

void GameAudioUpdate(void) {
    s32 testMode;
    s32 paused;
    f32 targetVol;
    f32 fadeTarget;

    if (Cursor.menu == 7) {
        testMode = 1;
        if (Cursor.y == 2) {
            testMode = 2;
        }
    } else {
        testMode = 0;
    }

    paused = Paused;

    if (paused != 0) goto paused_check;

    {
        void *mask = *(void **)((char *)player + 0xC);
        if (mask != NULL) {
            if (*(s16 *)((char *)mask + 0x16E) > 2) goto check_paused;
        }
    }

    if (GameMode == 1) goto check_paused;
    if (new_level != -1) goto check_paused;
    if (Cursor.menu != 0x24) goto play_music;

check_paused:
    if (paused == 0) goto cutmovie_check;

paused_check:
    if (testMode == 2) goto play_music;

cutmovie_check:
    if (cutmovie != 0) goto mute_music;

play_music:
    targetVol = 1.0f;
    goto do_fade;

mute_music:
    targetVol = 0.0f;

do_fade:
    if (musicvol_mul > targetVol) {
        musicvol_mul -= 1.0f / 60.0f;
        if (musicvol_mul < targetVol) {
            musicvol_mul = targetVol;
        }
    } else if (musicvol_mul < targetVol) {
        musicvol_mul += 1.0f / 60.0f;
        if (musicvol_mul > targetVol) {
            musicvol_mul = targetVol;
        }
    }

    if (game_music != -1) {
        if (Bonus == 1 || Bonus == 3) {
            NuFabs(bonus_duration * 0.5f - bonus_time);
        }
    }

    fadeTarget = 12.5f;

    if (ChaseActive() != -1) goto fade_50;
    if (Level == 0x17) goto fade_50;
    if (LBIT & 0x03E00000ULL) goto fade_25;
    if (Level == 9) goto fade_50;
    if (Level == 3) goto fade_50;
    goto check_vehicle;

fade_50:
    fadeTarget = 50.0f;
    goto do_nusound_fade;

check_vehicle:
    if (VEHICLECONTROL == 2) goto fade_25;
    if (VEHICLECONTROL != 1) goto do_nusound_fade;

    {
        s16 ctype = *(s16 *)((char *)player + 0x36);
        if (ctype == 0x53) goto fade_25;
        if (ctype != 0x20) goto check_creature_b;

    fade_25:
        fadeTarget = 25.0f;
        goto do_nusound_fade;

    check_creature_b:
        if (ctype == 0x36) goto fade_100;
        if (ctype == 0x81) goto fade_100;
        if (ctype == 0x8B) goto fade_100;
        if (ctype != 0x99) goto do_nusound_fade;

    fade_100:
        fadeTarget = 100.0f;
    }

do_nusound_fade:
    if (nusound_fade_end > fadeTarget) {
        nusound_fade_end -= 5.0f / 12.0f;
        if (nusound_fade_end < fadeTarget) {
            nusound_fade_end = fadeTarget;
        }
    } else if (nusound_fade_end < fadeTarget) {
        nusound_fade_end += 5.0f / 12.0f;
        if (nusound_fade_end > fadeTarget) {
            nusound_fade_end = fadeTarget;
        }
    }

    if (testsfx != -1) {
        GameSfx(testsfx + 0xB3, NULL);
        testsfx = -1;
    }

    if (testglobalsfx != -1) {
        last_testglobalsfx = testglobalsfx;
        GameSfx(testglobalsfx, NULL);
        testglobalsfx = -1;
    }

    edbitsSetSoundFxVolume(Game.sfx_volume);
}