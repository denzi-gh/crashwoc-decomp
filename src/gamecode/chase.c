#include "main.h"
#include <stddef.h>
#include <string.h>

typedef struct anim_s AnimPacket;
int strncasecmp(const char* s1, const char* s2, size_t n);
int sprintf(char *str, const char *format, ...);
char *strcat(char *dest, const char *src);

extern f32 temp_fALONG;
extern u16 temp_xrot;
extern u16 temp_zrot;
extern struct nuvec_s ShadNorm;

s32 qrand(void);
s32 StartHGobjAnim(struct nuhspecial_s *obj);
s32 ResetHGobjAnim(struct nuhspecial_s *obj);
void PointAlongSpline(struct nugspline_s *spl, float ratio, struct nuvec_s *dst, u16 *angle, u16 *tilt);
float NewShadowMask(struct nuvec_s *ppos, float size, int extramask);
void JudderGameCamera(struct cammtx_s *cam, float time, struct nuvec_s *pos);
struct nugspline_s *NuSplineFind(struct nugscn_s *scene, char *name);

struct chaseevent_s {
    struct nugspline_s* spl;
    struct nuhspecial_s obj[24];
};

struct chase_s {
    struct nugspline_s* spl_START;
    float time;
    float duration;
    struct nugspline_s* spl_CHASER[6];
    struct nuvec_s pos[6];
    struct RPos_s RPos[6];
    struct anim_s anim[6];
    struct nuhspecial_s obj[6];
    short character[6];
    short action[6];
    u16 xrot[6];
    u16 yrot[6];
    u16 zrot[6];
    struct chaseevent_s event[6][24];
    struct nugspline_s* spl_MISC[6][4];
    struct Nearest_Light_s lights[6];
    float scale[6];
    u8 misc_phase[6];
    char ok[6];
    u8 cuboid;
    char i;
    char i_last;
    char i_next;
    char status;
    char iRAIL;
    short iALONG;
    float fALONG;
};

typedef struct chase_s CHASE;

struct objtab_s {
    struct nuhspecial_s obj;
    struct nugscn_s** scene;
    char visible;
    char font3d_letter;
    char pad1;
    char pad2;
    char* name;
    char unk[4];
    u64 levbits;
};

CHASE Chase[3];
s32 LIGHTCHASECHARACTERS;

extern struct RPos_s* best_cRPos;
extern struct objtab_s ObjTab[201];
void EvalModelAnim(struct CharacterModel *model, struct anim_s *anim, struct numtx_s *m, struct numtx_s *tmtx, float ***dwa, struct numtx_s *mLOCATOR);
s32 FurtherALONG(s32 iRAIL0, s32 iALONG0, float fALONG0, s32 iRAIL1, s32 iALONG1, float fALONG1);
s32 FurtherBEHIND(s32 iRAIL0, s32 iALONG0, float fALONG0, s32 iRAIL1, s32 iALONG1, float fALONG1);
void InitChase(CHASE *chase) {
    s32 j;

    for (j = 0; j <= 5; j++) {
        if (chase->ok[j] == 0) continue;

        if ((s8)chase->i_last == -1) goto init_scratch;
        if (chase->character[j] == -1) goto start_anim;
        if (chase->character[j] != Chase[(s8)chase->i_last].character[j]) goto init_scratch;

        chase->anim[j] = Chase[(s8)chase->i_last].anim[j];
        chase->lights[j] = Chase[(s8)chase->i_last].lights[j];
        goto start_anim;

    init_scratch:
        if (chase->character[j] == -1) goto start_anim;

        {
            struct CharacterModel *model = &CModel[CRemap[chase->character[j]]];
            if (chase->action[j] != -1 && model->anmdata[chase->action[j]] != NULL) {
                s32 r = qrand();
                f32 rf = (f32)r * (1.0f / 65536.0f);
                f32 model_scale = *(f32 *)model->anmdata[chase->action[j]];
                chase->anim[j].anim_time = rf * (model_scale - 1.0f) + 1.0f;
            } else {
                chase->anim[j].anim_time = 1.0f;
            }

            chase->anim[j].blend = 0;
            chase->anim[j].action = chase->action[j];
            chase->anim[j].newaction = chase->action[j];
            chase->anim[j].oldaction = chase->action[j];
        }

    start_anim:
        if (chase->obj[j].special != NULL) {
            StartHGobjAnim(&chase->obj[j]);
        }

        chase->xrot[j] = 0;
        chase->zrot[j] = 0;
        PointAlongSpline(chase->spl_CHASER[j], 0.0f, &chase->pos[j], &chase->yrot[j], NULL);

        {
            f32 shadow = NewShadowMask(&chase->pos[j], 0.0f, -1);
            if (shadow != 2000000.0f) {
                FindAnglesZX(&ShadNorm);
                if (Level != 0x11 && Level != 0x0E && Level != 0x1F && Level != 0x05) {
                    chase->pos[j].y = shadow;
                }
            } else {
                temp_xrot = 0;
                temp_zrot = 0;
            }
        }

        chase->xrot[j] = temp_xrot;
        chase->zrot[j] = temp_zrot;

        GetALONG(&chase->pos[j], &chase->RPos[j], -1, -1, 2);
        ResetLights(&chase->lights[j]);
    }

    chase->time = 0.0f;
    chase->status = 2;
    JudderGameCamera(GameCam, 0.5f, NULL);
    GameSfx(0x3b, NULL);
    NewBuzz(&player->rumble, 0x1E);
}

void ResetChases(void) {
    CHASE *chase;
    s32 i;
    s32 j;
    s32 k;
    s32 l;

    chase = Chase;
    for (i = 0; i <= 2; i++, chase = chase + 1) {
        if (chase->status != 0) {
            if (AheadOfCheckpoint((s8)chase->iRAIL, chase->iALONG, chase->fALONG) != 0) {
                for (j = 0; j <= 5; j++) {
                    if (chase->ok[j] != 0) {
                        if (chase->obj[j].special != NULL) {
                            ResetHGobjAnim(&chase->obj[j]);
                        }
                        for (k = 0; k <= 23; k++) {
                            if (chase->event[j][k].spl != NULL) {
                                for (l = 0; l < 24; l++) {
                                    ResetHGobjAnim(&chase->event[j][k].obj[l]);
                                }
                            }
                        }
                        chase->misc_phase[j] = 0;
                    }
                }
                chase->status = 1;
            }
        }
    }
}

s32 LineCrossed(float xold, float zold, float xnew, float znew, float x0, float z0, float x1, float z1);
s32 NuHGobjRndrMtxDwa(struct NUHGOBJ_s *hgobj, struct numtx_s *wm, int nlayers, short *layers, struct numtx_s *mtx_array, float **dwa);
s32 NuRndrGScnObj(struct nugobj_s *gobj, struct numtx_s *wm);
s32 NuSpecialFind(struct nugscn_s *scene, struct nuhspecial_s *special, char *name);
void SetLights(struct nucolour3_s *vCOL0,struct nuvec_s *vDIR0,struct nucolour3_s *vCOL1,struct nuvec_s *vDIR1, struct nucolour3_s *vCOL2,struct nuvec_s *vDIR2,struct nuvec_s *vAMB);
void SetLevelLights(void);
void AddAnimDebris(struct CharacterModel *model,struct numtx_s *mtx,s32 action,float time,struct numtx_s *rotmtx);

struct gdeb_s {
    s32 i;
    char* name;
    u64 levbits;
};

extern struct nuvec_s v000;
extern s32 LivesLost;
extern s32 jonframe1;
extern struct gdeb_s GDeb[170];
extern struct nuvec_s chase_midpos[3];

float NewShadowMaskPlat(struct nuvec_s *ppos, float size, s32 extramask);
s32 HitCreatures(struct obj_s *obj, s32 destroy, s32 type);
s32 WipeCreatures(struct RPos_s *rpos);
s32 WipeCrates(s32 iRAIL0, s32 iALONG0, float fALONG0, s32 iRAIL1, s32 iALONG1, float fALONG1, s32 destroy);
s32 HitWumpa(struct obj_s *obj, s32 destroy);
s32 WipeWumpa(struct RPos_s *rpos);
s32 CylinderCuboidOverlapXZ(struct nuvec_s *pos, float radius, struct obj_s *cub, struct nuvec_s *cub_pos);
s32 CylinderCylinderOverlapXZ(struct nuvec_s *p0, float r0, struct nuvec_s *p1, float r1);
void UpdateAnimPacket(struct CharacterModel *mod, struct anim_s *anim, float dt, float xz_distance);
void LoseMask(struct obj_s *obj);

float NearestChaserDistance(struct chase_s *chase, struct obj_s *obj);

void UpdateChase(struct chase_s* chase, struct obj_s* obj) {
    struct nuvec_s oldpos;
    struct nuvec_s pos;
    struct obj_s c_obj;
    struct RPos_s best_RPos;
    struct nuvec_s delta;
    s32 holding;
    s32 kill;
    s32 count;
    s32 sfx;
    CharacterData *cdata;
    float d;
    float tmul;
    float y;
    s32 j;
    s32 k;

    d = -1.0f;
    holding = 0;

    /* Check if i_last chase has finished */
    if ((s8)chase->i_last != -1) {
        if (Chase[(s8)chase->i_last].status != 3) {
            holding = 1;
        }
    }

    /* If obj is dead and level 0x11, skip time advance */
    if (obj->dead != 0) {
        if (Level == 0x11) {
            goto after_time_advance;
        }
    }

    if (holding != 0) goto after_time_advance;

    /* Compute time step */
    {
        float t = 1.0f / 60.0f;
        if (LivesLost != 0) {
            s32 ll = LivesLost;
            if (ll > 5) ll = 5;
            t = t - (f32)ll * t * 0.05f;
        }

        d = NearestChaserDistance(chase, obj);
        if (chase->time < chase->duration) {
            if (Level == 0x11) {
                if (d == 0.0f) goto after_time_advance;
            }
            if (d >= 3.0f) {
                t = t * (d / 3.0f);
            }
            {
                float newtime = chase->time + t;
                chase->time = newtime;
                if (newtime >= chase->duration) {
                    chase->time = chase->duration;
                    chase->status = 3;
                    PauseGameAudio(0);
                    GameSfx(0x3b, NULL);
                    JudderGameCamera(GameCam, 0.5f, NULL);
                }
            }
        }
    }

after_time_advance:
    /* Set chase_midpos for this chase */
    chase_midpos[(s8)chase->i] = v000;
    d = chase->time / chase->duration;
    tmul = 0.5f;
    best_RPos.iRAIL = -1;
    count = 0;
    kill = 0;

    /* Per-chaser loop */
    for (j = 0; j <= 5; j++) {
        if (chase->ok[j] == 0) goto next_chaser;
        if (chase->status != 2) goto after_chaser_update;

        /* Save old position and interpolate along spline */
        oldpos = chase->pos[j];
        PointAlongSpline(chase->spl_CHASER[j], d, &pos, &chase->yrot[j], NULL);
        chase->pos[j].x = pos.x;
        y = NewShadowMaskPlat(&pos, 0.0f, -1);
        chase->pos[j].z = pos.z;

        if (y != 2000000.0f) {
            FindAnglesZX(&ShadNorm);
            if (Level == 0x8) {
                pos.y = y;
            }
        } else {
            temp_xrot = 0;
            temp_zrot = 0;
        }

        /* Level 8: interpolate Y */
        if (Level == 0x8) {
            float curY = chase->pos[j].y;
            if (curY < pos.y) {
                pos.y = (pos.y - curY) * 0.1f + curY;
            }
        } else {
            pos.y = pos.y; /* reload from stack */
        }
        chase->pos[j].y = pos.y;

        /* Rotate xrot/zrot unless level 0xe */
        if (Level != 0x0E) {
            chase->xrot[j] = SeekRot(chase->xrot[j], temp_xrot, 4);
            chase->zrot[j] = SeekRot(chase->zrot[j], temp_zrot, 4);
        } else {
            chase->zrot[j] = 0;
            chase->xrot[j] = 0;
        }

        /* Update midpos and ALONG */
        {
            struct nuvec_s *midp = &chase_midpos[(s8)chase->i];
            count++;
            NuVecAdd(midp, midp, &chase->pos[j]);
            GetALONG(&chase->pos[j], &chase->RPos[j], (s8)chase->RPos[j].iRAIL, chase->RPos[j].iALONG, 2);
        }

        /* Track furthest behind chaser */
        if ((s8)best_RPos.iRAIL != -1) {
            if (FurtherBEHIND((s8)chase->RPos[j].iRAIL, chase->RPos[j].iALONG, chase->RPos[j].fALONG,
                              (s8)best_RPos.iRAIL, best_RPos.iALONG, best_RPos.fALONG) == 0) {
                goto after_best_update;
            }
        }
        best_RPos.iRAIL = chase->RPos[j].iRAIL;
        best_RPos.iALONG = chase->RPos[j].iALONG;
        best_RPos.fALONG = chase->RPos[j].fALONG;
after_best_update:

        /* Check events */
        for (k = 0; k < 24; k++) {
            struct nugspline_s *espl = chase->event[j][k].spl;
            if (espl == NULL) continue;

            {
                struct nuvec_s *p0 = (struct nuvec_s *)espl->pts;
                struct nuvec_s *p1 = (struct nuvec_s *)(espl->pts + (s32)espl->ptsize);

                if (LineCrossed(oldpos.x, oldpos.z,
                                chase->pos[j].x, chase->pos[j].z,
                                p0->x, p0->z, p1->x, p1->z) != 2) continue;
            }

            /* Trigger event objects */
            {
                s32 started = 0;
                s32 count2 = 24;
                struct nuhspecial_s *eobj = &chase->event[j][k].obj[0];
                do {
                    if (StartHGobjAnim(eobj) != 0) {
                        started++;
                    }
                    eobj++;
                    count2--;
                } while (count2 != 0);

                if (started == 0) continue;
            }

            /* Play SFX for event */
            sfx = -1;
            if (Level == 0x11) {
                sfx = 0xB4;
                if (qrand() <= 0x7FFF) {
                    sfx = 0xB3;
                }
            } else if (Level == 0x8) {
                if (j == 0) goto set_event_sfx;
            } else {
            set_event_sfx:
                sfx = 0x3B;
            }

            if (sfx != -1) {
                GameSfx(sfx, &chase->pos[j]);
            }
            JudderGameCamera(GameCam, 0.3f, NULL);
        }

        /* Check misc spline triggers */
        for (k = 0; k <= 3; k++) {
            struct nugspline_s *mspl = chase->spl_MISC[j][k];
            if (mspl == NULL) continue;
            {
                struct nuvec_s *p0 = (struct nuvec_s *)mspl->pts;
                struct nuvec_s *p1 = (struct nuvec_s *)(mspl->pts + (s32)mspl->ptsize);
                if (LineCrossed(oldpos.x, oldpos.z,
                                chase->pos[j].x, chase->pos[j].z,
                                p0->x, p0->z, p1->x, p1->z) == 2) {
                    chase->misc_phase[j] = (u8)(k + 1);
                }
            }
        }

        /* Hit/Wipe and collision for characters */
        if (chase->character[j] != -1 && chase->character[j] <= 0x3E6) {
            cdata = &CData[chase->character[j]];

            c_obj.pos = chase->pos[j];
            c_obj.min = cdata->min;
            c_obj.max = cdata->max;
            c_obj.bot = cdata->min.y;
            c_obj.top = cdata->max.y;
            c_obj.SCALE = cdata->scale * chase->scale[j];
            c_obj.RADIUS = cdata->radius * c_obj.SCALE;
            c_obj.flags = 0;
            c_obj.pLOCATOR = NULL;
            c_obj.attack = 0x200;

            HitCreatures(&c_obj, 1, 3);
            HitCrates(&c_obj, 2);
            HitWumpa(&c_obj, 0);

            if (USELIGHTS != 0 && LIGHTCHASECHARACTERS != 0) {
                pos.x = c_obj.pos.x;
                pos.y = (c_obj.bot + c_obj.top) * c_obj.SCALE * tmul + c_obj.pos.y;
                pos.z = c_obj.pos.z;
                GetLights(&pos, &chase->lights[j], 1);
            }
        } else {
            /* Non-character chaser: wipe instead */
            WipeCreatures(&chase->RPos[j]);
            WipeCrates((s8)chase->iRAIL, chase->iALONG, chase->fALONG,
                       (s8)chase->RPos[j].iRAIL, chase->RPos[j].iALONG, chase->RPos[j].fALONG, 2);
            WipeWumpa(&chase->RPos[j]);
        }

        /* Level 8: particle debris */
        if (Level == 0x8) {
            if ((jonframe1 & 3) == 0) {
                AddVariableShotDebrisEffect(GDeb[131].i, &chase->pos[j], 1, 0, 0);
            }
        }

    after_chaser_update:
        /* Player collision with character chasers */
        if (obj->dead != 0) goto next_chaser;
        if (holding != 0) goto next_chaser;
        if (kill != 0) goto next_chaser;
        if (chase->character[j] == -1) goto next_chaser;
        if (chase->character[j] > 0x3E6) goto next_chaser;

        {
            cdata = &CData[chase->character[j]];

            c_obj.pos = chase->pos[j];
            c_obj.min = cdata->min;
            c_obj.max = cdata->max;
            c_obj.bot = cdata->min.y;
            c_obj.top = cdata->max.y;
            c_obj.SCALE = cdata->scale * chase->scale[j];
            {
                float absrad = NuFabs(cdata->radius);
                c_obj.attack = 0x200;
                c_obj.hdg = chase->yrot[j];
                c_obj.RADIUS = absrad * c_obj.SCALE;
            }

            if (d >= 0.0f) {
                if ((chase->cuboid & (1 << j)) != 0) {
                    if (CylinderCuboidOverlapXZ(&obj->pos, obj->RADIUS, &c_obj, &c_obj.pos) != 0) {
                        goto set_kill;
                    }
                    goto next_chaser;
                } else {
                    if (CylinderCylinderOverlapXZ(&obj->pos, obj->RADIUS, &c_obj.pos, c_obj.RADIUS) == 0) {
                        goto next_chaser;
                    }
                }
            set_kill:
                kill = 1;
            }
        }

    next_chaser:
        /* Update anim if character */
        if (chase->character[j] == -1) continue;
        {
            struct CharacterModel *model = &CModel[CRemap[(s32)chase->character[j]]];
            chase->anim[j].oldaction = chase->anim[j].newaction;
            k = chase->character[j];

            if (k == 0x42) {
                if (chase->status == 3) {
                    k = 0x33;
                    if (chase->i == 2) goto set_action_75;
                } else {
                    if (obj->dead != 0) goto set_action_75;
                    k = 0x3A;
                    if (chase->misc_phase[j] == 1) {
                        k = 0x40;
                    }
                }
            } else {
                k = chase->action[j];
            }
            goto do_update_anim;

        set_action_75:
            k = 0x75;

        do_update_anim:
            chase->anim[j].newaction = k;
            UpdateAnimPacket(model, &chase->anim[j], 0.5f, 0.0f);
        }
    }

    /* Normalize chase_midpos */
    if (count > 1) {
        float fc = (float)count;
        chase_midpos[(s8)chase->i].x /= fc;
        chase_midpos[(s8)chase->i].y /= fc;
        chase_midpos[(s8)chase->i].z /= fc;
    }

    /* SFX and rumble while active */
    if (chase->status == 2 && count != 0) {
        sfx = -1;
        switch (Level) {
        case 0x05:
            sfx = 0xB3;
            break;
        case 0x08:
            {
                sfx = 0xB6;
                NuVecSub(&delta, &obj->pos, &chase_midpos[(s8)chase->i]);
                delta.x *= 0.9f;
                delta.y *= 0.9f;
                delta.z *= 0.9f;
                NuVecAdd(&chase_midpos[(s8)chase->i], &chase_midpos[(s8)chase->i], &delta);
            }
            break;
        case 0x1F:
            sfx = 0xB6;
            break;
        case 0x0E:
            sfx = 0xBD;
            break;
        }

        if (sfx != -1) {
            GameSfxLoop(sfx, &chase_midpos[(s8)chase->i]);
        }

        if (qrand() <= 0x7FF) {
            s32 rv = qrand();
            if (rv < 0) rv += 0xFF;
            rv >>= 8;
            NewRumble(&player->rumble, rv);
            JudderGameCamera(GameCam, (float)rv / 255.0f * 0.3f, NULL);
        }
    }

    /* Kill player if hit */
    if (obj->dead != 0) goto done;
    if (holding != 0) goto done;
    if (kill != 0) goto do_kill;
    /* Check if player is ahead of closest chaser */
    if ((s8)best_RPos.iRAIL == -1) goto done;
    if (best_cRPos == NULL) goto done;
    if (FurtherALONG((s8)best_RPos.iRAIL, best_RPos.iALONG, best_RPos.fALONG,
                     (s8)best_cRPos->iRAIL, best_cRPos->iALONG, best_cRPos->fALONG) == 0) {
        goto done;
    }
    kill = 1;

do_kill:
    {
        s32 die = GetDieAnim(obj, -1);
        if (KillGameObject(obj, die) != 0) {
            if (Level == 0x11) {
                GameSfx(0xB7, NULL);
            }
        }

        if (obj->mask != NULL) {
            if (obj->mask->active != 0) {
                if ((LDATA->flags & 0xE00) == 0) {
                    LoseMask(obj);
                    obj->mask->active = 0;
                }
            }
        }
        obj->invincible = 0;
    }

done:
    ;
}

//NGC MATCH
struct nugspline_s * NuSplineFindPartial(struct nugscn_s *scene,char *name,char *txt) {
  struct nugspline_s *spl;
  s32 i;
  
  spl = scene->splines;
  for(i = 0; i < scene->numsplines; i++) {
      if (strncasecmp(name,spl->name,strlen(name)) == 0) {
        strcpy(txt,spl->name);
        return spl;
      }
      spl++;
  }
  return NULL;
}

void InitChases(void) {
    struct pCHASE *pc;
    CHASE *chase;
    char buf[64];
    char name[64];
    char chname[64];
    char txt[256];
    char parsebuf[32];
    struct nuvec_s pos;
    struct nugspline_s *spl;
    s32 j, k, l;
    s32 count;
    s32 base_len;
    s32 name_len;
    s32 char_idx;

    chase = Chase;
    for (j = 0; j < 3; j++) {
        chase->status = 0;
        chase++;
    }

    pc = LDATA->pChase;
    if (pc == NULL) return;
    if (world_scene[0] == NULL) return;
    if ((s8)pc->i == -1) return;

    do {
        sprintf(buf, "chase_%.2i_", (s8)pc->i);
        strcpy(chname, buf);

        chase = &Chase[(s8)pc->i];

        strcpy(tbuf, chname);
        strcat(tbuf, "trigger");

        spl = NuSplineFind(world_scene[0], tbuf);
        chase->spl_START = spl;
        if (spl == NULL) goto next_pc;

        if (spl->len != 2) {
            chase->spl_START = NULL;
        }
        if (chase->spl_START == NULL) goto next_pc;

        count = 0;

        for (j = 0; j <= 5; j++) {
            chase->ok[j] = 0;
            chase->character[j] = -1;
            chase->obj[j].special = NULL;

            if (pc->character[j] == -1) continue;

            sprintf(tbuf, "%s%.2i", buf, j);
            spl = NuSplineFind(world_scene[0], tbuf);
            chase->spl_CHASER[j] = spl;
            if (spl == NULL) continue;

            {
                u16 c_raw = pc->character[j];
                s16 c = (s16)c_raw;
                if (c > 0x3E6) {
                    s32 idx = c - 0x3E7;
                    if ((u32)idx > 0xC8) continue;
                    if (ObjTab[idx].obj.special == NULL) continue;
                    chase->obj[j].scene = ObjTab[idx].obj.scene;
                    chase->obj[j].special = ObjTab[idx].obj.special;
                } else {
                    if ((s8)CRemap[c] == -1) continue;
                    chase->character[j] = c;
                    chase->action[j] = pc->action[j];
                }
            }

            chase->scale[j] = pc->scale[j];

            sprintf(name, "%s%.2i_", buf, j);
            strcat(name, "trigger_event_");

            for (k = 0; k <= 23; k++) {
                sprintf(tbuf, "%s%.2i_", name, k);
                base_len = strlen(tbuf);
                spl = NuSplineFindPartial(world_scene[0], tbuf, txt);
                chase->event[j][k].spl = spl;
                if (spl == NULL) goto next_event;
                if (spl->len != 2) {
                    chase->event[j][k].spl = NULL;
                }
                if (chase->event[j][k].spl == NULL) goto next_event;

                for (l = 0; l < 24; l++) {
                    chase->event[j][k].obj[l].special = NULL;
                }

                count = 0;
                name_len = strlen(txt);
                char_idx = 0;

                if (base_len < name_len) {
                    for (l = base_len; l < name_len; l++) {
                        s8 c = txt[l];
                        if (c == 'X' || c == 'x' || l == name_len - 1) {
                            if (l == name_len - 1) {
                                parsebuf[char_idx] = c;
                                char_idx++;
                            }
                            parsebuf[char_idx] = '\0';
                            if (NuSpecialFind(world_scene[0], &chase->event[j][k].obj[count], parsebuf) != 0) {
                                count++;
                                if (count == 24) goto next_event;
                            }
                            char_idx = 0;
                        } else {
                            parsebuf[char_idx] = c;
                            char_idx++;
                        }
                    }
                }

            next_event:
                ;
            }

            sprintf(name, "%s%.2i_", buf, j);
            count = count + 1;
            strcat(name, "trigger_misc_");

            for (l = 0; l <= 3; l++) {
                sprintf(tbuf, "%s%.2i", name, l);
                spl = NuSplineFind(world_scene[0], tbuf);
                chase->spl_MISC[j][l] = spl;
                if (spl != NULL && spl->len != 2) {
                    chase->spl_MISC[j][l] = NULL;
                }
            }

            chase->ok[j] = 1;
        }

        if (count == 0) goto next_pc;

        {
            struct nugspline_s *ts = chase->spl_START;
            struct nuvec_s *p0 = (struct nuvec_s *)ts->pts;
            struct nuvec_s *p1 = (struct nuvec_s *)(ts->pts + (s32)ts->ptsize);
            pos.x = (p0->x + p1->x) * 0.5f;
            pos.y = (p0->y + p1->y) * 0.5f;
            pos.z = (p0->z + p1->z) * 0.5f;
        }

        GetALONG(&pos, NULL, -1, -1, 1);

        chase->iRAIL = temp_iRAIL;
        chase->iALONG = temp_iALONG;
        chase->fALONG = temp_fALONG;
        chase->duration = pc->duration;
        chase->i = pc->i;
        chase->i_last = pc->i_last;
        chase->i_next = pc->i_next;
        chase->status = 1;
        chase->cuboid = pc->cuboid;

    next_pc:
        pc++;
    } while ((s8)pc->i != -1);
}

//NGC MATCH
void UpdateCrateBallsOfFireDoors(void) {
    struct nuhspecial_s obj[8];
    char txt[9];
    s32 i;

    if ((best_cRPos != NULL) && (Rail[best_cRPos->iRAIL].type == 0)) {
        NuSpecialFind(world_scene[0], obj + 0, "door1");
        NuSpecialFind(world_scene[0], obj + 1, "door2");
        NuSpecialFind(world_scene[0], obj + 2, "door3");
        NuSpecialFind(world_scene[0], obj + 3, "door4");
        NuSpecialFind(world_scene[0], obj + 4, "door25");
        NuSpecialFind(world_scene[0], obj + 5, "door26");
        NuSpecialFind(world_scene[0], obj + 6, "door27");
        NuSpecialFind(world_scene[0], obj + 7, "door28");

        if (FurtherBEHIND(best_cRPos->iRAIL, best_cRPos->iALONG, best_cRPos->fALONG, 2, 0x14, 0.5f) != 0) {
            strcpy(txt, "00000000");
        } else if (FurtherBEHIND(best_cRPos->iRAIL, best_cRPos->iALONG, best_cRPos->fALONG, 2, 0x21, 0.5f) != 0) {
            strcpy(txt, "00001111");
        } else {
            strcpy(txt, "11110000");
        }

        for (i = 0; i < 8; i++) {
            if (obj[i].special != NULL) {
                obj[i].special->instance->flags.visible = txt[i] == '1';
            }
        }

        NuSpecialFind(world_scene[0], obj + 0, "door5");
        NuSpecialFind(world_scene[0], obj + 1, "door6");
        NuSpecialFind(world_scene[0], obj + 2, "door7");
        NuSpecialFind(world_scene[0], obj + 3, "door8");
        NuSpecialFind(world_scene[0], obj + 4, "door29");
        NuSpecialFind(world_scene[0], obj + 5, "door30");
        NuSpecialFind(world_scene[0], obj + 6, "door31");
        NuSpecialFind(world_scene[0], obj + 7, "door32");

        if (FurtherBEHIND(best_cRPos->iRAIL, best_cRPos->iALONG, best_cRPos->fALONG, 2, 0x38, 0.5f) != 0) {
            strcpy(txt, "00000000");
        } else if (FurtherBEHIND(best_cRPos->iRAIL, best_cRPos->iALONG, best_cRPos->fALONG, 2, 0x41, 0.5f) != 0) {
            strcpy(txt, "11110000");
        } else {
            strcpy(txt, "00001111");
        }

        for (i = 0; i < 8; i++) {
            if (obj[i].special != NULL) {
                obj[i].special->instance->flags.visible = txt[i] == '1';
            }
        }
    }
    return;
}

//NGC MATCH
s32 ChaseActive(void) {
  s32 i;

  for(i = 0; i < 3; i++) {
    if (Chase[i].status == 2) {
      return i;
    }
  }
  return -1;
}

//NGC MATCH
float NearestChaserDistance(struct chase_s *chase,struct obj_s *obj) {
  s32 iVar1;
  s32 iVar2;
  float d0;
  float d;
  
  d0 = 100.0f;
  for(iVar2 = 0; iVar2 < 6; iVar2++) {
    if (chase->ok[iVar2] != 0) {
      if ((best_cRPos != NULL) && (FurtherALONG((s32)chase->RPos[iVar2].iRAIL,(s32)chase->RPos[iVar2].iALONG,
                 chase->RPos[iVar2].fALONG,(s32)best_cRPos->iRAIL,(s32)best_cRPos->iALONG,best_cRPos->fALONG) != 0)) {
        return 0.0f;
      }
      d = NuVecXZDistSqr(&obj->pos,&chase->pos[iVar2],NULL);
      if (d < d0) {
        d0 = d;
      }
    }
  }
    return NuFsqrt(d0);
}

//NGC MATCH
void DrawChases(s32 render) {
  CHASE *chase;
  struct numtx_s m;
  struct CharacterModel *model;
  struct nuvec_s s;
  s32 i;
  s32 j;
  s32 local_64;
  struct numtx_s mtxLOCATOR [16];
  float **dwa;
  AnimPacket* anim;
  
  chase = Chase;
  local_64 = 0;
  for(i = 0; i < 3; i++, chase++) {
    if ((chase->status == 1) && (Level == 0x1f)) {
      local_64 = 1;
    }
    if (((local_64 != 0) ||
        ((chase->status == 2 && ((chase->i_last == -1 || (Chase[chase->i_last].status == 3))))))
       || ((chase->status == 3 &&
           ((chase->i_next != -1 &&
            (((chase->i_next > 2 || (Chase[chase->i_next].status == 1)) || (Chase[chase->i_next].status == 0)))))) ))
    {
      for(j = 0; j < 6; j++) {
        if (chase->ok[j] != 0) {
          if (chase->obj[j].special != NULL) {
            if (local_64 == 0) {
              s.x = s.y = s.z = 1.0f;
              NuMtxSetScale(&m,&s);
              NuMtxRotateY(&m,chase->yrot[j] + 0x8000);
              NuMtxRotateZ(&m,chase->zrot[j]);
              NuMtxRotateX(&m,chase->xrot[j]);
              NuMtxTranslate(&m,&chase->pos[j]);
            }
            if (render != 0) {
              if (Level != 0x1f) {
                NuRndrGScnObj((chase->obj[j].scene)->gobjs
                              [(chase->obj[j].special)->instance->objid],&m);
              }
              chase->obj[j].special->instance->mtx = m;
              if ((Level == 0x1f) && (ObjTab[0x4e].obj.special != NULL)) {
                NuRndrGScnObj((ObjTab[0x4e].obj.scene)->gobjs
                              [(ObjTab[0x4e].obj.special)->instance->objid],&m);
              }
            }
          }
          else {
            if ((s32)chase->character[j] != -1) {
              model = &CModel[CRemap[(s32)chase->character[j]]];
              s.x = s.y = s.z = CData[(s32)chase->character[j]].scale * chase->scale[j];
              NuMtxSetScale(&m,&s);
              NuMtxRotateY(&m,chase->yrot[j] + 0x8000);
              NuMtxRotateZ(&m,chase->zrot[j]);
              NuMtxRotateX(&m,chase->xrot[j]);
              NuMtxTranslate(&m,&chase->pos[j]);
              anim = &chase->anim[j];
              EvalModelAnim(model,anim,&m,tmtx,&dwa,mtxLOCATOR);
              if ((temp_action != -1) && (Paused == 0)) {
                AddAnimDebris(model,mtxLOCATOR,temp_action,temp_time,NULL);
              }
              if (render != 0) {
                if ((USELIGHTS != 0) && (LIGHTCHASECHARACTERS != 0)) {
                  SetLights(&chase->lights[j].pDir1st->Colour,&chase->lights[j].pDir1st->Direction,&chase->lights[j].pDir2nd->Colour,&chase->lights[j].pDir2nd->Direction,
                            &chase->lights[j].pDir3rd->Colour,&chase->lights[j].pDir3rd->Direction,&chase->lights[j].AmbCol);
                }
                NuHGobjRndrMtxDwa(model->hobj,&m,1,NULL,tmtx,dwa);
              }
            }
          }
        }
      }
    }
  }
  if (((render != 0) && (USELIGHTS != 0)) && (LIGHTCHASECHARACTERS != 0)) {
    SetLevelLights();
  }
}

/* UPDATECHASE LOCAL VAR
    nuvec_s oldpos; // 0x8(r1)
    nuvec_s* p0; // r9
    nuvec_s* p1; // r11
    nuvec_s pos; // 0x18(r1)
    CharacterModel* model; // r3
    float y; // f31
    float t; // f31
    float tmul; // f28
    float d; // f29
    float f; // f30
    int i; // r21
    int j; // r30
    int k; // r28
    int l; // r31
    int holding; // 0x1F0(r1)
    int kill; // r9
    int cuboid; // 
    int count; // 0x1F4(r1)
    int sfx; // 0x1F8(r1)
    // Size: 0x188
    struct obj_s  c_obj;
    struct CData_s * cdata; // r28
*/

//NGC MATCH
void UpdateChases(void) {
    struct chase_s* chase;
    struct nuvec_s* p0;
    struct nuvec_s* p1;
    struct obj_s* obj;
    s32 i;

    if (Level == 0x1f) {
        UpdateCrateBallsOfFireDoors();
    }

    chase = Chase;
    obj = &player->obj;
    for (i = 0; i < 3; i++, chase++) {
        if (chase->status == 1) {
            if (player->obj.transporting == 0) {
                p0 = (struct nuvec_s*)(chase->spl_START->pts);
                p1 = (struct nuvec_s*)(chase->spl_START->pts + (s32)chase->spl_START->ptsize);
                if (LineCrossed(obj->oldpos.x, obj->oldpos.z, obj->pos.x, obj->pos.z, p0->x, p0->z, p1->x, p1->z) == 2)
                {
                    InitChase(chase);
                }
            }
        } else if (chase->status > 1) {
            if (chase->status < 4) {
                UpdateChase(chase, obj);
            }
        }
    }
    return;
}
