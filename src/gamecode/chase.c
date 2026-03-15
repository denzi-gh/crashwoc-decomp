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
void UpdateChase(struct chase_s* chase, struct obj_s* obj);
void SetLights(struct nucolour3_s *vCOL0,struct nuvec_s *vDIR0,struct nucolour3_s *vCOL1,struct nuvec_s *vDIR1, struct nucolour3_s *vCOL2,struct nuvec_s *vDIR2,struct nuvec_s *vAMB);
void SetLevelLights(void);
void AddAnimDebris(struct CharacterModel *model,struct numtx_s *mtx,s32 action,float time,struct numtx_s *rotmtx);

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
