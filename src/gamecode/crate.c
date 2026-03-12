#include "main.h"
#include <stddef.h>

typedef struct crate_s CRATE;

struct crate_s {
  s32 id;
  char type[4];
  struct nuvec_s pos;
  struct crate_s* linked;
  struct crate_s* trigger;
  u16 orientation;
  s16 offx;
  s16 offy;
  s16 offz;
  s16 ccindex;
  s8 draw;
  s8 cpad1;
};

struct crate_type_s {
  struct nuhspecial_s obj;
  s32 id;
  char* name;
  s32 character;
};

typedef struct {
  CRATE* model;
  struct nuvec_s pos0;
  struct nuvec_s pos;
  f32 oldy;
  f32 shadow;
  f32 mom;
  f32 timer;
  f32 duration;
  s8 on;
  s8 iRAIL;
  s16 iALONG;
  f32 fALONG;
  u16 flags;
  s8 type1;
  s8 type2;
  s8 type3;
  s8 type4;
  s8 newtype;
  s8 subtype;
  s8 i;
  s8 metal_count;
  s8 appeared;
  s8 in_range;
  s16 dx;
  s16 dy;
  s16 dz;
  s16 iU;
  s16 iD;
  s16 iN;
  s16 iS;
  s16 iE;
  s16 iW;
  s16 trigger;
  s8 counter;
  s8 anim_cycle;
  s16 index;
  f32 anim_time;
  f32 anim_duration;
  f32 anim_speed;
  u16 xrot0;
  u16 zrot0;
  u16 xrot;
  u16 zrot;
  u16 surface_xrot;
  u16 surface_zrot;
  s16 character;
  s16 action;
  struct nuvec_s colbox[2];
} CrateCube;

typedef struct {
  struct nuvec_s origin;
  f32 radius;
  s16 iCrate;
  s16 nCrates;
  u16 angle;
  s8 pad1;
  s8 pad2;
  struct nuvec_s minclip;
  struct nuvec_s maxclip;
} CrateCubeGroup;

typedef struct {
  CrateCube* crate;
  s8 type1;
  s8 type2;
  s8 type3;
  s8 type4;
} CRATETYPEDATA;

typedef struct {
  struct nuangvec_s ang;
  struct nuangvec_s angmom;
  struct nuvec_s pos;
  struct nuvec_s mom;
  s32 rndfade;
} BoxPolType;

typedef struct {
  s16 time;
  s16 type;
  struct nuvec_s colbox[2];
  BoxPolType BoxPol[36];
} BoxExpType;

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

struct tersurface_s {
  f32 friction;
  u32 flags;
};

static struct nulsthdr_s* crates;
s32 num_crates_used;

extern struct nugscn_s* crate_scene;
extern volatile struct crate_type_s crate_list[29];
extern s32 CRATECOUNT;
extern CrateCube Crate[];
extern s32 CRATEGROUPCOUNT;
extern CrateCubeGroup CrateGroup[];
extern CRATE* marker_crate;
extern CRATE* flash_crate;
extern CRATE* lock_crate;
extern CRATE* locked_crate;
extern CRATE* triggerorigin_crate;
extern CRATE* triggerdest_crate;
extern CRATE* highlight_crate;
extern CRATE* highlighted_crate;
extern CRATE MarkerCrate;
extern CRATE FlashCrate;
extern CRATE LockCrate;
extern CRATE HighlightCrate;
extern s32 current_selected_crate;
extern char Chase[];
extern CrateCubeGroup* temp_pGroup;
extern CrateCube* temp_pCrate;
extern CRATETYPEDATA CrateTypeData[32];
extern s32 i_cratetypedata;
extern s32 TimeTrial;
extern s32 mask_crates;
extern struct nuvec_s vNEWMASK;
extern s32 newmask_advise;
extern f32 TimeTrialWait;
extern f32 plr_invisibility_time;
extern s32 last_questionmark_extralife;
extern s32 VEHICLECONTROL;
extern f32 glass_mix;
extern f32 glass_col_mix;
extern s32 glass_col_enabled;
extern s32 glass_enabled;
extern f32 WATERBOSSGLASSMIX;
extern BoxExpType BoxExpList[16];
extern s32 iBOXEXP;
extern s32 CRATEEXPLOSIONFRAMES;
extern f32 CRATEHOPSPEED;
extern f32 CRATEGRAVITY;
extern f32 METALCRATEBOUNCESPEED;
extern s32 level_part_2;
extern s32 temp_crate_type;
extern f32 CRATEBALLOONRADIUS;
extern f32 CRATEBALLOONOFFSET;
extern f32 EShadY;
extern f32 temp_ratio;
extern f32 temp_fALONG;
extern s32 temp_face;
extern struct nuvec_s vTEMP;
extern struct nuvec_s ShadNorm;
extern s32 cp_goto;
extern struct nuvec_s cpGOTO;
extern struct RPos_s* best_cRPos;
extern s32 temp_crate_bounce;
extern u16 temp_xrot;
extern u16 temp_zrot;
extern s32 Bonus;
extern s32 bonus_restart;
extern s32 cp_iALONG;
extern s32 cp_iRAIL;
extern s32 DESTRUCTIBLEBONUSCRATECOUNT;
extern s32 DESTRUCTIBLECRATECOUNT;
extern s32 DRAWCRATESHADOWS;
extern struct RPos_s gempath_RPos;
extern struct cammtx_s* pCam;
extern struct objtab_s ObjTab[201];
extern struct tersurface_s TerSurface[];
extern f32 tntsfxtime[7];
extern s32 SFX_CHANGER;
extern struct numtl_s* CrateMat;
extern struct numtl_s* CrateMat2;
extern float uvs[48];

void AddKaboom(s32 type, struct nuvec_s *pos, float radius);
void AddCrateExplosion(struct nuvec_s *pos, s32 type, s32 ang, struct nuvec_s *colbox);
void AddPanelDebris(float x, float y, int type, float scale, int count);
void JudderGameCamera(struct cammtx_s *cam, float time, struct nuvec_s *pos);
s32 EShadowInfo(void);
s32 FurtherALONG(s32 iRAIL0, s32 iALONG0, float fALONG0, s32 iRAIL1, s32 iALONG1, float fALONG1);
s32 FurtherBEHIND(s32 iRAIL0, s32 iALONG0, float fALONG0, s32 iRAIL1, s32 iALONG1, float fALONG1);
CrateCube* CrateInSlot(CrateCubeGroup *group, s32 x, s32 y, s32 z);
float NewShadowMaskPlat(struct nuvec_s *ppos, float size, s32 extramask);
void NewScanInit(void);
short* NewScanHandel(struct nuvec_s *vpos, struct nuvec_s *vvel, float size, s32 plats, short *handel);
s32 NewRayCastSetHandel(struct nuvec_s *vpos, struct nuvec_s *vvel, float size, float timeadj, float impactadj,
                        short *Handel);
void ResetCheckpoint(s32 iRAIL, s32 iALONG, float fALONG, struct nuvec_s *pos);
void ResetKabooms(void);
void AddScreenWumpa(float x, float y, float z, s32 count);
void AddTempWumpa(float x, float y, float z, CrateCube *crate, s32 n);
void AddQuad3DrotXYZ(struct nuvec_s *pos,struct nuvec_s *shape,struct numtl_s *mtl,struct nuangvec_s *a,float *uvs,u32 colour);
void HopCratesAbove(float speed, CrateCubeGroup *group, CrateCube *crate);
CrateCube* InCrate(float x, float z, float top, float bot, float radius);
s32 CrateOnTop(CrateCubeGroup *group, CrateCube *crate);
void UpdateCrates(void);
void HeightSortCrateData(void);
void ConvertCrateData(void);
void BreakCrate(CrateCubeGroup *group, CrateCube *crate, s32 type, s32 attack);
void DestroyAllNitroCrates(CrateCubeGroup *group, CrateCube *crate);
void AddGliderHitPoints(s32 points);
float **NuHGobjEvalDwa(s32 layer, void *bollox, struct nuanimdata_s *vtxanim, float vtxtime);
void NuHGobjEvalAnim(struct NUHGOBJ_s *hgobj, struct nuanimdata_s *animdata, float time, s32 njanims,
                     struct NUJOINTANIM_s *janim, struct numtx_s *mtx_array);

//NGC MATCH
void ResetCrateType2(CrateCube *crt) {
  crt->type2 = crt->type1;
  if (crt->model == NULL) {
    return;
  }
  crt->model->type[1] = crt->type1;
  return;
}

//NGC MATCH
struct crate_s * NextCrate(struct crate_s *a) {
  return (struct crate_s *)NuLstGetNext(crates,(struct nulnkhdr_s *)a);
}

//NGC MATCH
void InitCrates(void) {
  s32 iVar2;
  s32 iVar4;
  
  crates = NuLstCreate(0x100,0x28);
  CRATECOUNT = 0;
  CRATEGROUPCOUNT = 0;
  if (crate_scene != NULL) {
    for(iVar2 = 0; iVar2 < 29; iVar2++) {
      NuSpecialFind(crate_scene,(struct nuhspecial_s* )&crate_list[iVar2].obj,crate_list[iVar2].name);
    }
  }
  else {
    for(iVar2 = 0; iVar2 < 0x1d; iVar2++) {
      crate_list[iVar2].obj.special = 0;
    }
  }
  MarkerCrate.type[0] = 0x1a;
  marker_crate = &MarkerCrate;
  LockCrate.type[0] = 0x1b;
  lock_crate = &LockCrate;
  HighlightCrate.type[0] = 0x1c;
  highlight_crate = &HighlightCrate;
  FlashCrate.type[0] = (char)current_selected_crate;
  flash_crate = &FlashCrate;
  locked_crate = 0;
  triggerorigin_crate = 0;
  triggerdest_crate = 0;
  highlighted_crate = 0;
  return;
}

//NGC MATCH
void CloseCrates(void) {
  if (crates != NULL) {
    NuLstDestroy(crates);
    crates = NULL;
  }
  return;
}

//NGC MATCH
void ConvertCrateData(void) {
  struct crate_s *cr;
  struct crate_s *cr2;
  CrateCube *cc;
  CrateCubeGroup *crg;
  s32 i;
  s32 j;
  s32 k;
  struct nuvec_s tvec;
  s32 ddx;
  s32 ddy;
  s32 ddz;
  
  CRATECOUNT = 0;
  CRATEGROUPCOUNT = 0;
  cr2 = NextCrate(0);
  if (cr2 != 0) {
    s32 *pGroupCount;
    
    pGroupCount = &CRATEGROUPCOUNT;
    do {
      if (cr2->linked == cr2) {
        tvec.z = -0.25f;
        tvec.y = -0.25f;
        tvec.x = -0.25f;
        crg = &CrateGroup[(*pGroupCount)++];
        NuVecRotateY(&tvec,&tvec,(uint)cr2->orientation);
        crg->origin.x = cr2->pos.x + tvec.x;
        crg->origin.y = cr2->pos.y + tvec.y;
        crg->origin.z = cr2->pos.z + tvec.z;
        crg->radius = 0.0f;
        crg->iCrate = (u16)CRATECOUNT;
        cr2->ccindex = crg->iCrate;
        crg->nCrates = 1;
        crg->angle = cr2->orientation;
        cr = NextCrate(0);
        crg->maxclip = cr2->pos;
        crg->minclip = crg->maxclip;
        CRATECOUNT++;
        if (cr != 0) {
          do {
            if (cr->linked == cr2) {
              if (cr2 == cr) {
                cc = &Crate[(s32)crg->iCrate];
              }
              else {
                cc = &Crate[CRATECOUNT];
                cr->ccindex = CRATECOUNT++;
                crg->nCrates++;
              }
              if (cr->pos.x < crg->minclip.x) {
                crg->minclip.x = cr->pos.x;
              }
              if (cr->pos.y < crg->minclip.y) {
                crg->minclip.y = cr->pos.y;
              }
              if (cr->pos.z < crg->minclip.z) {
                crg->minclip.z = cr->pos.z;
              }
              if (cr->pos.x > crg->maxclip.x) {
                crg->maxclip.x = cr->pos.x;
              }
              if (cr->pos.y > crg->maxclip.y) {
                crg->maxclip.y = cr->pos.y;
              }
              if (cr->pos.z > crg->maxclip.z) {
                crg->maxclip.z = cr->pos.z;
              }
              cc->pos0.x = cr->pos.x + tvec.x;
              cc->pos0.y = cr->pos.y + tvec.y;
              cc->pos0.z = cr->pos.z + tvec.z;
              cc->shadow = 0.0f;
              cc->dx = cr->offx;
              cc->dy = cr->offy;
              cc->dz = cr->offz;
              cc->type1 = cr->type[0];
              cc->type2 = cr->type[1];
              cc->type3 = cr->type[2];
              cc->type4 = (signed char)cr->type[3];
              cc->trigger = -1;
              cc->on = 1;
              cc->timer = 0.0f;
              cc->iU = -1;
              cc->iD = -1;
              cc->iN = -1;
              cc->iS = -1;
              cc->iE = -1;
              cc->iW = -1;
              cc->model = cr;
            }
            cr = NextCrate(cr);
          } while (cr != 0);
        }
        crg->minclip.x -= 0.5f;
        crg->minclip.y -= 0.5f;
        crg->minclip.z -= 0.5f;
        crg->maxclip.x += 0.5f;
        crg->maxclip.y += 0.5f;
        crg->maxclip.z += 0.5f;
      }
      cr2 = NextCrate(cr2);
    } while (cr2 != 0);
  }
  for(i = 0; i < CRATEGROUPCOUNT; i++) {
    crg = &CrateGroup[i];
    for(j = crg->iCrate; j < crg->iCrate + crg->nCrates; j++) {
      for(k = crg->iCrate; k < crg->iCrate + crg->nCrates; k++) {
        if (j != k) {
          cc = &Crate[k];
          ddx = cc->dx - Crate[j].dx;
          ddy = cc->dy - Crate[j].dy;
          ddz = cc->dz - Crate[j].dz;
          if (((ddx == 1) && (ddy == 0)) && (ddz == 0)) {
            Crate[j].iE = k;
          }
          if (((ddx == -1) && (ddy == 0)) && (ddz == 0)) {
            Crate[j].iW = k;
          }
          if (ddx == 0) {
            if ((ddy == 1) && (ddz == 0)) {
              Crate[j].iU = k;
            }
          }
          if (ddx == 0) {
            if ((ddy == -1) && (ddz == 0)) {
              Crate[j].iD = k;
            }
          }
          if (ddx == 0) {
            if ((ddy == 0) && (ddz == 1)) {
              Crate[j].iN = k;
              Crate[j].dz += 0;
            }
          }
          if (((ddx == 0) && (ddy == 0)) && (ddz == -1)) {
            Crate[j].iS = k;
          }
        }
      }
    }
  }
  for(cr2 = NextCrate(0); cr2 != 0; cr2 = NextCrate(cr2)) {
    if (cr2->trigger != 0) {
      Crate[cr2->ccindex].trigger = cr2->trigger->ccindex;
    }
  }
  HeightSortCrateData();
  return;
}

//NGC MATCH
s32 ReadCrateData(void) {
  s32 handle;
  s32 i;
  s32 iVar7;
  s32 version;
  void * fbuff;
  
  CRATECOUNT = 0;
  sprintf(tbuf,"%s.crt",LevelFileName);
  if (NuFileExists(tbuf) != 0) {
    fbuff = Chase;
    handle = NuMemFileOpen(fbuff,NuFileLoadBuffer(tbuf,fbuff,0x7fffffff),NUFILE_READ);
    if (handle != 0) {
      version = NuFileReadInt(handle);
      if (version > 4) {
          NuFileClose(handle);
          return 0;
      }
      CRATEGROUPCOUNT = (s32)NuFileReadShort(handle);
      for(i = 0; i < CRATEGROUPCOUNT; i++) {
            CrateGroup[i].origin.x = NuFileReadFloat(handle);
            CrateGroup[i].origin.y = NuFileReadFloat(handle);
            CrateGroup[i].origin.z = NuFileReadFloat(handle);
            CrateGroup[i].radius = 0.0f;
            CrateGroup[i].iCrate = NuFileReadShort(handle);
            CrateGroup[i].nCrates = NuFileReadShort(handle);
            CRATECOUNT = CRATECOUNT + CrateGroup[i].nCrates;
            CrateGroup[i].angle = NuFileReadShort(handle);
            for (iVar7 = (s32)CrateGroup[i].iCrate; iVar7 < (s32)CrateGroup[i].iCrate + (s32)CrateGroup[i].nCrates; iVar7++) {
                Crate[iVar7].pos0.x = NuFileReadFloat(handle);
                Crate[iVar7].pos0.y = NuFileReadFloat(handle);
                Crate[iVar7].pos0.z = NuFileReadFloat(handle);
                Crate[iVar7].shadow = NuFileReadFloat(handle);
                Crate[iVar7].dx = NuFileReadShort(handle);
                Crate[iVar7].dy = NuFileReadShort(handle);
                Crate[iVar7].dz = NuFileReadShort(handle);
                Crate[iVar7].type1 = NuFileReadChar(handle);
                if (version > 2) {
                  Crate[iVar7].type2 = NuFileReadChar(handle);
                  Crate[iVar7].type3 = NuFileReadChar(handle);
                  Crate[iVar7].type4 = NuFileReadChar(handle);
                }
                else {
                  Crate[iVar7].type2 = -1;
                  Crate[iVar7].type3 = -1;
                  Crate[iVar7].type4 = -1;
                }
                Crate[iVar7].timer = 0.0f;
                Crate[iVar7].on = 1;
                Crate[iVar7].iU = NuFileReadShort(handle);
                Crate[iVar7].iD = NuFileReadShort(handle);
                Crate[iVar7].iN = NuFileReadShort(handle);
                Crate[iVar7].iS = NuFileReadShort(handle);
                Crate[iVar7].iE = NuFileReadShort(handle);
                Crate[iVar7].iW = NuFileReadShort(handle);
                if (version > 3) {
                  Crate[iVar7].trigger = NuFileReadShort(handle);
                }
                else {
                  Crate[iVar7].trigger = -1;
                }
            }
      }
      NuFileClose(handle);
      return 1;        
    }
  }
  return 0;
}

//NGC MATCH
s32 NewCrateAnimation(CrateCube* crate, s32 type, s32 action, s32 random) {
    struct CharacterModel* model;
    s32 i;
    s32 character;

    if ((u32)type > 0x14) {
        return 0;
    }
    
    character = crate_list[type].character;
    crate->character = -1;
    crate->action = -1;
    
    if (((u32)character > 0xbe) || ((u32)action > 0x75)) {
        return 0;
    }

    i = CRemap[character];
    if (i == -1) {
        return 0;
    }
    
    model = &CModel[i];
    if (model->anmdata[action] != NULL) {
        i = 1;
        crate->anim_duration = model->anmdata[action]->time;
        crate->anim_cycle = model->animlist[action]->flags & 1;
        crate->anim_speed = model->animlist[action]->speed;
    } else if (model->fanmdata[action] != NULL) {
        i = 1;
        crate->anim_duration = model->fanmdata[action]->time;
        crate->anim_cycle = model->fanimlist[action]->flags & 1;
        crate->anim_speed = model->fanimlist[action]->speed;
    } else {
        i = 0;
    }

    if (i != 0) {
        crate->anim_time = 1.0f;
        if (random != 0) {
            crate->anim_time = (s32)qrand() * 0.000015259022f * (crate->anim_duration - 1.0f) + crate->anim_time;
            if (crate->anim_time >= crate->anim_duration) {
                crate->anim_time = 1.0f;
            }
        }
        crate->character = character;
        crate->action = action;
    }
    return i;
}

//NGC MATCH
void OpenPreviousCheckpoints(s32 iRAIL,s32 iALONG,float fALONG) {
  CrateCubeGroup *group;
  CrateCube *crate;
  s32 i;
  s32 j;

  group = CrateGroup;
  for(i = 0; i < CRATEGROUPCOUNT; i++, group++) {
      crate = &Crate[group->iCrate];
      for(j = 0; j < group->nCrates; j++, crate++) {
          if ((((crate->on != 0) && (crate->type1 == 7)) && (crate->iRAIL != -1)) &&
             (FurtherBEHIND(crate->iRAIL,crate->iALONG,crate->fALONG,iRAIL,iALONG,fALONG) != 0)) {
            crate->on = 0;
            if (crate->model != NULL) {
              crate->model->draw = 0;
            }
            NewCrateAnimation(crate,7,0x34,0);
          }
      }
  }
  return;
}

//NGC MATCH
void ReadInCrateData(void) {
  int i;
  int j;
  struct crate_s * new_crate;
  CrateCubeGroup* crg;
  CrateCube* cc;
  struct crate_s *first_crate;
  struct nuvec_s tvec;
  struct crate_s *cr;
  struct crate_s *cr2;
  
  HeightSortCrateData();
  num_crates_used = 0;
  for(i = 0; i < CRATEGROUPCOUNT; i++) {
      tvec.z = 0.25f;
      tvec.y = 0.25f;
      tvec.x = 0.25f;
      crg = &CrateGroup[i];
      NuVecRotateY(&tvec,&tvec,(uint)crg->angle);
      for(j = 0; j < crg->nCrates; j++) {
          cc = &Crate[crg->iCrate + j];
          cr = (struct crate_s *)NuLstAlloc(crates);
          if (cr != NULL) {
            cr->id = cc->type1;
            cr->pos.x = cc->pos0.x + tvec.x;
            cr->pos.y = cc->pos0.y + tvec.y;
            cr->pos.z = cc->pos0.z + tvec.z;
            if (j == 0) {
              crg->minclip = crg->maxclip = cr->pos;
              first_crate = cr;
            }
            else {
              if (cr->pos.x < crg->minclip.x) {
                crg->minclip.x = cr->pos.x;
              }
              if (cr->pos.y < crg->minclip.y) {
                crg->minclip.y = cr->pos.y;
              }
              if (cr->pos.z < crg->minclip.z) {
                crg->minclip.z = cr->pos.z;
              }
              if (cr->pos.x > crg->maxclip.x) {
                crg->maxclip.x = cr->pos.x;
              }
              if (cr->pos.y > crg->maxclip.y) {
                crg->maxclip.y = cr->pos.y;
              }
              if (cr->pos.z > crg->maxclip.z) {
                crg->maxclip.z = cr->pos.z;
              }
            }
            cr->orientation = crg->angle;
            cr->trigger = NULL;
            cr->linked = first_crate;
            cr->type[0] = cc->type1;
            cr->type[1] = cc->type2;
            cr->type[2] = cc->type3;
            cr->type[3] = cc->type4;
            cr->ccindex = crg->iCrate + (short)j;
            cr->offx = cc->dx - Crate[crg->iCrate].dx;
            cr->offy = cc->dy - Crate[crg->iCrate].dy;
            cr->offz = cc->dz - Crate[crg->iCrate].dz;
            cr->draw = 1;
            num_crates_used++;
            cc->model = cr;
          }
      }
      crg->minclip.x -= 0.5f;
      crg->minclip.y -= 0.5f;
      crg->minclip.z -= 0.5f;
      crg->maxclip.x += 0.5f;
      crg->maxclip.y += 0.5f;
      crg->maxclip.z += 0.5f;
  }
  
    for (cr2 = NextCrate(NULL); cr2 != NULL; cr2 = NextCrate(cr2)) {
        if (Crate[cr2->ccindex].trigger != -1) {
            new_crate = NULL;
LAB_1:
                new_crate = NextCrate(new_crate);
                if (new_crate != NULL) {
                    if (new_crate->ccindex != Crate[cr2->ccindex].trigger) {
                        goto LAB_1;
                    }
                    cr2->trigger = new_crate;
                }
        }
    }
    ConvertCrateData();
}

//NGC MATCH
void HeightSortCrateData(void) {
  s32 i;
  s32 j;
  s32 l;
  s32 k;
  CrateCube tcr;
  CrateCubeGroup* crg;

  for(i = 0; i < CRATEGROUPCOUNT; i++) {
   crg = &CrateGroup[i];
      for(j = crg->iCrate; j < (s32)crg->iCrate + (s32)crg->nCrates - 1; j++) {
          for(k = j + 1; k < (crg->iCrate + crg->nCrates); k++) {
              if (Crate[j].dy > Crate[k].dy) {
                tcr = Crate[k];
                Crate[k] = Crate[j];
                Crate[j] = tcr;
                for(l = 0; l < CRATECOUNT; l++) {
                    if (Crate[l].trigger == j) {
                      Crate[l].trigger = k;
                    }
                    else if (Crate[l].trigger == k) {
                      Crate[l].trigger = j;
                    }
                }
              }
          }
      }
  }
  return;
}

//NGC MATCH
void StartExclamationCrateSequence(CrateCubeGroup *group,CrateCube *crate) {
  struct nuvec_s pos;
  
  pos.x = (crate->pos).x;
  pos.y = (crate->pos).y + 0.25f;
  pos.z = (crate->pos).z;
  temp_pGroup = group;
  temp_pCrate = crate;
  AddKaboom(0x20,&pos,0.0f);
  crate->newtype = 0xf;
  crate->metal_count = 1;
  GameSfx(0x35,&temp_pCrate->pos);
  return;
}

//NGC MATCH
void SaveCrateTypeData(CrateCube *crate) {
  CRATETYPEDATA *data;
  
  if (i_cratetypedata > 0x1f) {
    return;
  }
  data = &CrateTypeData[i_cratetypedata];
  data->crate = crate;
  data->type1 = crate->type1;
  data->type2 = crate->type2;
  data->type3 = crate->type3;
  data->type4 = crate->type4;
  i_cratetypedata++;
  return;
}

//NGC MATCH
void RestoreCrateTypeData(void) {
  s32 i;
  CRATETYPEDATA *data;
  
  data = CrateTypeData;
  for(i = 0; i < i_cratetypedata; i++, data++) {
      data->crate->type1 = data->type1;
      data->crate->type2 = data->type2;
      data->crate->type3 = data->type3;
      data->crate->type4 = data->type4;
  }
  i_cratetypedata = 0;
  return;
}

//NGC MATCH
void AddExtraLife(struct nuvec_s *pos,int pdeb) {
  struct nuvec_s scr_pos;
  struct nuvec_s cV[2];
  float scale;
  
  NuCameraTransformScreenClip(&scr_pos,pos,1,NULL);
  cV[0].x = GameCam[0].vX.x * 0.1f + pos->x;
  cV[0].y = GameCam[0].vX.y * 0.1f + pos->y;
  cV[0].z = GameCam[0].vX.z * 0.1f + pos->z;
  NuCameraTransformScreenClip(&cV[1],&cV[0],1,NULL);
  scale = NuFabs((scr_pos.x - cV[1].x));
  AddPanelDebris(scr_pos.x,scr_pos.y,pdeb,(scale * 3.6363637f),1);
  return;
}

//NGC MATCH
s32 GetCrateType(CrateCube* crate, s32 flags) {
    u32 type;

    if (TimeTrial != 0) {
        type = crate->type2;
    } else {
        type = crate->type1;
    }
    
    if (type == 0 && crate->newtype != -1) {
        type = crate->newtype;
    }
    
    if ((flags & 2) && type == 8) {
        if (crate->newtype != -1) {
            goto block_12;
        }
    }
    else if (crate->newtype != -1) {
block_12:
        type = (TimeTrial == 0 || type == 9) ? (crate->subtype != -1 ? crate->subtype : crate->newtype) : crate->newtype;
    } else if (TimeTrial == 0 || type == 9) {
        type = (crate->subtype != -1) ? crate->subtype : type;
    }

    if (!(flags & 1)) {
        if (type == 22 || type == 23 || type == 24) {
            type = 9;
        }
    }
        
    else if (type == 2) {
        if (LDATA->character == 1) {
            type = 25;
        }
    }
    
    return type;
}

//NGC MATCH
void HopCratesAbove(float speed,CrateCubeGroup *group,CrateCube *crate) {
  CrateCube *crate2;
  s32 i;
  
Loop:
  crate2 = &Crate[group->iCrate];
  for(i = 0; i < group->nCrates; i++, crate2++) {
      if (((crate2->on != 0) && (crate2->dx == crate->dx) && (crate2->dz == crate->dz)) &&
         (crate->pos.y + 0.5f == crate2->pos.y)) {
        crate2->mom = speed;
        crate = crate2;
        goto Loop;
      }
  }
  return;
}

//NGC MATCH
s32 CrateAbove(struct obj_s *obj,CrateCubeGroup *group,CrateCube *crate) {
  CrateCube *crate2;
  s32 i;
  
  crate2 = Crate + group->iCrate;
  for(i = 0; i < group->nCrates; i++, crate2++) {
        if ( ( ((crate2 != crate) && (crate2->on != 0)) && (GetCrateType(crate2,0) != 0) ) &&
              ((crate2->dx == crate->dx) && (crate2->dz == crate->dz)) ) {
            if (crate2->pos.y > crate->pos.y && ( !(obj->objtop < crate2->pos.y) 
                && !(obj->objbot > (crate2->pos.y + 0.5f)) )) {
                 return 1;
              }
        }
  }
  return 0;
}

//NGC MATCH
s32 CrateBelow(struct obj_s *obj,CrateCubeGroup *group,CrateCube *crate) {
  s32 i;
  CrateCube *crate2;
  
  crate2 = Crate + group->iCrate;
  for(i = 0; i < group->nCrates; i++, crate2++) {
      if (((((crate2 != crate) && (crate2->on != 0)) && (GetCrateType(crate2,0) != 0)) &&
          ((crate2->dx == crate->dx && (crate2->dz == crate->dz)))) && ((crate2->pos.y < crate->pos.y &&
          (!(obj->objtop < crate2->pos.y || (obj->objbot > (crate2->pos.y + 0.5f) ) ) ) )) ) {
        return 1;
      }
  }
  return 0;
}

//NGC MATCH
s32 CrateOff(CrateCubeGroup *group,CrateCube *crate,s32 kaboom,s32 chase) {
  struct nuvec_s pos;
  s32 type;
  s32 i;
  s32 sfx;
  float fVar2;
  
  type = GetCrateType(crate,0);
  if ((((type == -1) || (type - 0xdU < 2)) || (type == 0)) || ((type == 0xf || (type == 0x11) ))) {
    return 0;
  }
  crate->on = 0;
  if (crate->model != NULL) {
    crate->model->draw = 0;
  }
  pos.x = (crate->pos).x;
  pos.y = (crate->pos).y + 0.25f;
  pos.z = (crate->pos).z;
  if ((type != 7) && (crate->in_range != 0)) {
    AddCrateExplosion(&crate->pos,type,group->angle,crate->colbox);
  }
  sfx = 0x25;
  switch (type) {
      case 2:
          if ((((kaboom & 3U) == 0) && ((player->obj).dead == 0)) && (chase == 0)) {
            i = 2;
            if ((crate->flags & 0x40) != 0) {
              i = 3;
            }
            AddExtraLife(&pos,i);
            if ((TimeTrial == 0) && ((crate->flags & 0x40) == 0)) {
              if ((crate->type1 == '\b') && (0 < crate->i)) {
                SaveCrateTypeData(crate);
                if (crate->i == 1) {
                  crate->type3 = crate->type4;
                }
                crate->type4 = -1;
              }
              else if (crate->type1 == 2) {
                SaveCrateTypeData(crate);
                crate->type1 = 5;
              }
              else if ((crate->type1 == 0) && (crate->type3 == 2)) {
                SaveCrateTypeData(crate);
                crate->type3 = 5;
              }
            }
          }
      break;
      case 7:
          sfx = 0x17;
          ResetCheckpoint((s32)crate->iRAIL,(s32)crate->iALONG,crate->fALONG,&crate->pos);
          NewCrateAnimation(crate,7,0x34,0);
      break;
      case 3:
          if ((((kaboom & 3U) == 0) && ((player->obj).dead == 0)) && (chase == 0)) {
            mask_crates++;
            vNEWMASK = crate->model->pos;
            newmask_advise = 0;
          }
      break;
      case 6:
      break;
      case 8:
      break;
      case 0x10:
          AddKaboom(2,&pos,0.0f);
          i = 6;
          sfx = 0x3b;
          AddGameDebris(i,&pos);
      break;
      case 9:
            AddKaboom(1,&pos,0.0f);
            i = 5;
            sfx = 0x3b;
            AddGameDebris(i,&pos);
            
      break;
      case 0xc:
          if (chase != 0) break;
          fVar2 = 3.0f;
            TimeTrialWait += fVar2;
      break;
      case 0xb:
        if (chase != 0) break;
        fVar2 = 2.0f;
        TimeTrialWait += fVar2;
      break;
      case 10:
        if (chase != 0) break;
        fVar2 = 1.0f;
        TimeTrialWait += fVar2;
      break;
      case 0x14:
          if ((chase == 0) && ((kaboom == 0 || ((kaboom & 0xcU) != 0)))) {
            plr_invisibility_time = 0.0f;
            GameSfx(0x1e,NULL);
          } 
      break;
      default:
          if ((((kaboom & 3U) == 0) && (chase == 0)) &&
             ((TimeTrial == 0 && (((player->obj).dead == 0 && ((crate->flags & 0x400) == 0)))))) {
              if (type == 5) {
                if (last_questionmark_extralife == 0) {
                    i = last_questionmark_extralife;
                  if (((Game.lives < 10) && ((crate->flags & 0x40) == 0)) &&
                     (qrand() < (s32)(0x4000 - (Game.lives << 0xe) / 10))) {
                    AddExtraLife(&pos,2);
                    last_questionmark_extralife = 1;
                    break;
                  }
                }
              }
              if (last_questionmark_extralife != 0) last_questionmark_extralife--;
              if (CrateOnTop(group,crate) != 0) {
                i = 1;
                AddScreenWumpa(pos.x,pos.y,pos.z,i);
              }
              else {
                if (type != 0x13) {
                  i = qrand() / 0x3334 + 1;
                }
                else {
                  i = 1;
                }
                if (((crate->flags & 0x1000) != 0) || ((VEHICLECONTROL == 1 && ((player->obj).vehicle != -1))) ) {
                  AddScreenWumpa(pos.x,pos.y,pos.z,i);
                } else {
                    AddTempWumpa(pos.x,pos.y,pos.z,crate,i);
                }
              }
        }
      break;
  }
  GameSfx(sfx,((crate->flags & 0x400) == 0) ? &pos : NULL);
  return 1;
}

//NGC MATCH
CrateCube * CrateInSlot(CrateCubeGroup *group,s32 x,s32 y,s32 z) {
  CrateCube *crate;
  s32 i;
  
  crate = &Crate[group->iCrate];
  for(i = 0; i < group->nCrates; i++, crate++) {
      if (((crate->dx == x) && (crate->dy == y)) && (crate->dz == z)) {
        return crate;
      }
  }
  return NULL;
}

//NGC MATCH
void ResetInvisibility(void) {
  plr_invisibility_time = 5.0f;
  glass_mix = (Level != 0x17) ? 0.0f : WATERBOSSGLASSMIX;
  glass_col_mix = 0;
  glass_col_enabled = 0;
  glass_enabled = 0;
  return;
}

//NGC MATCH
void ResetCrate(CrateCube *crt) {
  crt->mom = 0.0f;
  crt->oldy = crt->pos0.y;
  crt->pos.y = crt->pos0.y;
  crt->newtype = -1;
  crt->subtype = -1;
  if (((crt->type1 == 6) || (crt->type2 == 6)) || ((crt->type1 == 0 && (crt->type3 == 6)))) {
    crt->counter = 0xa;
  }
  else {
    crt->counter = 0;
  }
  crt->metal_count = 0;
  crt->action = -1;
  crt->appeared = 0;
  return;
}

//NGC MATCH
void ResetCrates(void) {
  CrateCubeGroup *group;
  CrateCube *crate;
  CrateCube *crate2;
  CrateCube *crate3;
  struct nuvec_s pos;
  struct nuvec_s v;
  struct nuvec_s vSN;
  struct nuvec_s tv;
  struct nuvec_s vROT = {0.25f, 0.0f, 0.25f};
  s32 i;
  s32 j;
  s32 k;
  s32 x;
  s32 y;
  s32 z;
  s32 iVar9;
  s32 iVar7;
  s32 iVar8;
  s32 below;
  s32 index;
  s32 uVar8;
  s32 gempath_reset;
  u16 save_flags;
  f32 dy;
  short *Handle;

  DESTRUCTIBLECRATECOUNT = 0;
  DESTRUCTIBLEBONUSCRATECOUNT = 0;
  index = 0;
  gempath_reset = 0;
  if ((Rail[7].type == 3) &&
      (AheadOfCheckpoint((s32)gempath_RPos.iRAIL, (s32)gempath_RPos.iALONG, gempath_RPos.fALONG) != 0)) {
    gempath_reset = 1;
  }
  group = CrateGroup;
  for(i = 0; i < CRATEGROUPCOUNT; i++, group++) {
    NuVecRotateY(&v, &vROT, (u32)group->angle);
    crate = &Crate[group->iCrate];
    for(j = 0; j < group->nCrates; j++, crate++) {
      if (crate->type2 == -1) {
        ResetCrateType2(crate);
      }
      crate->index = index;
      crate->pos.x = crate->pos0.x + v.x;
      index++;
      save_flags = 0;
      crate->pos.z = crate->pos0.z + v.z;
      GetALONG(&crate->pos0, NULL, -1, -1, 1);
      crate->iRAIL = (char)temp_iRAIL;
      crate->iALONG = temp_iALONG;
      crate->fALONG = temp_fALONG;
      crate->timer = 0.0f;
      if (crate->iRAIL != -1) {
        if (Rail[crate->iRAIL].type == 1) {
          save_flags = 0x40;
        } else if (Rail[crate->iRAIL].type == 2) {
          save_flags = 0x80;
        } else if (Rail[crate->iRAIL].type == 3) {
          save_flags = 0x100;
        }
      }
      if (crate->type1 == 8) {
        crate->i = 0;
        crate->duration = 1.0f;
      }
      if ((((LDATA->flags & 0x200) != 0) || (Level == 0x1d)) && (Level != 0x1e)) {
        save_flags |= 0xc00;
      }
      switch (crate->type1) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0xe:
          if (crate->iRAIL == -1) {
            if (save_flags & 0x400) {
              save_flags |= 0x10;
            }
          } else {
            save_flags |= 0x10;
          }
          break;
        case 0:
          if ((crate->iRAIL != -1) || (save_flags & 0x400)) {
            if ((crate->type3 != -1) && (crate->type3 != 0 && (crate->type3 != 13)) && (crate->type3 != 0xF)) {
              if ((crate->trigger != -1) && (Crate[crate->trigger].type1 == 0xe)) {
                save_flags |= 0x10;
              }
            }
          }
          break;
      }
      if ((save_flags & 0x10) != 0) {
        DESTRUCTIBLECRATECOUNT++;
      }
      if ((save_flags & 0x40) != 0) {
        if ((save_flags & 0x10) != 0) {
          DESTRUCTIBLEBONUSCRATECOUNT++;
        }
        if (Bonus != 4) goto Reset;
      } else if ((save_flags & 0x100) != 0) {
        if (gempath_reset != 0) goto Reset;
      } else if (((((cp_iRAIL == -1) || (cp_iALONG == -1)) || (crate->type1 != 7)) || (TimeTrial != 0)) &&
                 (bonus_restart == 0)) {
        if (AheadOfCheckpoint((s32)crate->iRAIL, (s32)crate->iALONG, crate->fALONG) != 0) {
Reset:
          crate->on = 1;
          ResetCrate(crate);
        }
      }
      if (crate->type1 == 3) {
        NewCrateAnimation(crate, 3, 0x22, 1);
      }
      pos.x = crate->pos.x;
      pos.y = crate->pos0.y;
      pos.z = crate->pos.z;
      crate->shadow = NewShadowMaskPlat(&pos, 0.0f, -1);
      if ((Level == 0x12) && (crate->shadow < 0.0f)) {
        crate->shadow = 0.0f;
      }
      if (crate->shadow != 2000000.0f) {
        vSN = ShadNorm;
        FindAnglesZX(&vSN);
        crate->surface_xrot = temp_xrot;
        crate->surface_zrot = temp_zrot;
        iVar8 = ShadowInfo();
        if ((TerSurface[iVar8].flags & 1) == 0) {
          save_flags |= 0x2000;
        }
        iVar9 = (s32)EShadY;
        if (iVar9 != 2000000.0f) {
          uVar8 = EShadowInfo();
          if (uVar8 < 0 || uVar8 > 10) {
            uVar8 = 0;
          }
        } else {
          uVar8 = -1;
        }
        NewScanInit();
        crate->colbox[0].y = crate->shadow;
        crate->colbox[1].y = crate->shadow + 4.0f;
        tv.x = (pos.x - 1.5f);
        tv.y = (crate->shadow + 0.2f);
        tv.z = (pos.z - 1.5f);
        vSN.x = 3.0f;
        vSN.y = 0.0f;
        vSN.z = 3.0f;
        Handle = NewScanHandel(&tv, &vSN, 0.0f, 1, NULL);
        pos.y = (crate->shadow + 0.2f);
        tv.x = -1.5f;
        tv.y = 0.0f;
        tv.z = 0.0f;
        NewRayCastSetHandel(&pos, &tv, 0.0f, 0.0f, 0.0f, Handle);
        crate->colbox[0].x = tv.x + pos.x;
        tv.x = 0.0f;
        tv.y = 0.0f;
        tv.z = -1.5f;
        NewRayCastSetHandel(&pos, &tv, 0.0f, 0.0f, 0.0f, Handle);
        crate->colbox[0].z = tv.z + pos.z;
        tv.x = 1.5f;
        tv.y = 0.0f;
        tv.z = 0.0f;
        NewRayCastSetHandel(&pos, &tv, 0.0f, 0.0f, 0.0f, Handle);
        crate->colbox[1].x = tv.x + pos.x;
        tv.x = 0.0f;
        tv.y = 0.0f;
        tv.z = 1.5f;
        NewRayCastSetHandel(&pos, &tv, 0.0f, 0.0f, 0.0f, Handle);
        crate->colbox[1].z = tv.z + pos.z;
      } else {
        crate->surface_xrot = 0;
        crate->surface_zrot = 0;
        iVar8 = 0;
        uVar8 = -1;
      }
      if ((TerSurface[iVar8].flags & 4) != 0) {
        save_flags |= 0x200;
      }
      if ((LowestCrate(group, crate) != 0) &&
          (((crate->shadow == 2000000.0f || (crate->pos.y - crate->shadow > 2.0f)) ||
            (((TerSurface[iVar8].flags & 1) != 0 || ((uVar8 != -1 && (crate->pos.y > iVar9)))))))) {
        save_flags |= 0x1000;
      }
      crate->flags = save_flags;
      if (crate->shadow != 2000000.0f) {
        dy = crate->pos0.y - crate->shadow;
        if (dy < -0.1f) {
          crate->flags = save_flags | 4;
        } else if (dy < 0.1f) {
          crate->flags = save_flags | 2;
          if (((crate->type1 == 0x10) || (crate->type1 == 3)) || ((crate->type2 == 0x10) || (crate->type2 == 3))) {
            crate->flags |= 1;
          }
        }
      }
    }
    crate = &Crate[group->iCrate];
    for(j = 0; j < group->nCrates; j++, crate++) {
      if ((crate->flags & 6) == 0) {
        crate2 = NULL;
        x = crate->dx;
        y = crate->dy;
        z = crate->dz;
        crate3 = NULL;
LinkLoop:
        y--;
        crate3 = CrateInSlot(group, x, y, z);
        if (crate3 != NULL) {
          crate2 = crate3;
          goto LinkLoop;
        }
        if ((crate2 != NULL) && ((crate2->flags & 6) != 0)) {
          crate->flags |= 1;
        }
      }
      if ((crate->type1 == 7) && (crate->shadow != 2000000.0f)) {
        crate->xrot0 = crate->surface_xrot;
        crate->zrot0 = crate->surface_zrot;
      } else if (((crate->flags & 4) != 0) && (CrateInSlot(group, crate->dx, crate->dy + 1, crate->dz) == NULL)) {
        crate->xrot0 = (qrand() - 0x8000) / 16;
        crate->zrot0 = (qrand() - 0x8000) / 16;
      } else if (((crate->flags & 2) != 0) && (CrateInSlot(group, crate->dx, crate->dy + 1, crate->dz) == NULL)) {
        crate->xrot0 = crate->surface_xrot;
        crate->zrot0 = crate->surface_zrot;
      } else {
        crate->xrot0 = 0;
        crate->zrot0 = 0;
      }
      crate->xrot = crate->xrot0;
      crate->zrot = crate->zrot0;
      if (crate->type1 == 6) {
        iVar7 = 1;
      } else if (crate->type2 == 6) {
        iVar7 = 2;
      } else if ((crate->type1 == 0) && (crate->type3 == 6)) {
        iVar7 = 3;
      } else {
        iVar7 = 0;
      }
      if (iVar7 != 0) {
        crate3 = &Crate[group->iCrate];
        for(k = 0; k < group->nCrates; k++, crate3++) {
          below = (crate3->dy < crate->dy) ? 1 : 0;
          if ((((crate3 != crate) && (crate3->dx == crate->dx)) && (crate3->dz == crate->dz)) &&
              ((((iVar7 == 1 &&
                 ((crate3->type1 == 6 || ((below && ((crate3->type1 == 4 || (crate3->type1 == 13)))))))) ||
                ((iVar7 == 2 && ((crate3->type2 == 6 ||
                  ((below && ((crate3->type2 == 4 || (crate3->type2 == 13)))))))))) || ((iVar7 == 3 &&
                (((crate3->type1 == 0 && (crate3->trigger == crate->trigger)) && ((crate3->type3 == 6 ||
                  ((below && ((crate3->type3 == 4 || (crate3->type3 == 13)))))))))))))) {
            crate->flags |= 0x20;
            break;
          }
        }
Twin:
        crate->counter = crate->counter / 2;
      }
    }
  }
  ResetKabooms();
  UpdateCrates();
  ResetInvisibility();
  return;
}

void UpdateCrates(void) {
  CrateCubeGroup *group;
  CrateCube *crate;
  CrateCube *crate2;
  struct nuvec_s v;
  s32 i;
  s32 j;
  s32 k;
  s32 contact;
  s32 type;
  s32 old;
  s32 sfx;
  f32 speed;
  f32 y_adjust;
  f32 old_time;
  u16 angle;

  if (plr_invisibility_time < 5.0f) {
    old_time = plr_invisibility_time;
    plr_invisibility_time += 0.016666668f;
    if ((old_time < 4.0f) && (plr_invisibility_time >= 4.0f)) {
      GameSfx(0x21, NULL);
    }
    if (plr_invisibility_time > 5.0f) {
      plr_invisibility_time = 5.0f;
    }
  }
  glass_enabled = plr_invisibility_time < 4.0f;
  angle = (u16)(((GameTimer.frame % 60) << 0x10) / 60);
  group = CrateGroup;
  for(i = 0; i < CRATEGROUPCOUNT; i++, group++) {
    crate = &Crate[group->iCrate];
    for(j = 0; j < group->nCrates; j++, crate++) {
      if ((((LDATA->flags & 0x200) != 0) || (Level == 0x1d)) && (VEHICLECONTROL == 1)) {
        y_adjust = NuTrigTable[angle] * 0.25f;
        angle += 0x1000;
      }
      sfx = -1;
      if ((crate->on == 0) && ((TimeTrial != 0) || (crate->type1 != 7))) {
        goto SkipContact;
      }
      if (crate->action != -1) {
        if (crate->anim_time < crate->anim_duration) {
          crate->anim_time += crate->anim_speed * 0.5f;
          if (crate->anim_time >= crate->anim_duration) {
            if (crate->anim_cycle != 0) {
              crate->anim_time -= crate->anim_duration - 1.0f;
            } else {
              crate->anim_time = crate->anim_duration;
            }
          }
        } else {
          if (TimeTrial != 0) {
            if (crate->type2 == 0xe) {
              StartExclamationCrateSequence(group, crate);
              goto SetActionDone;
            }
          } else if (crate->type1 != 7) {
            if ((crate->type1 == 0xe) || ((crate->type1 == 0) && (crate->type3 == 0xe))) {
              StartExclamationCrateSequence(group, crate);
              goto SetActionDone;
            }
          }
          if (TimeTrial != 0) {
            if (crate->type2 == 0x11) {
              DestroyAllNitroCrates(group, crate);
            }
          } else if (crate->type1 != 7) {
            if ((crate->type1 == 0x11) || ((crate->type1 == 0) && (crate->type3 == 0x11))) {
              DestroyAllNitroCrates(group, crate);
            }
          }
SetActionDone:
          crate->action = -1;
        }
        if ((TimeTrial == 0) && (crate->type1 == 7) && (crate->on == 0)) {
          goto EndLoop;
        }
      }
      crate->oldy = crate->pos.y;
      type = GetCrateType(crate, 2);
      switch (type) {
        case 9:
          if (crate->timer != 0.0f) {
            crate->timer += 0.016666668f;
            for(k = 0; k < 7; k++) {
              if ((crate->timer >= tntsfxtime[k]) && (crate->timer - 0.016666668f < tntsfxtime[k])) {
                GameSfx(0x4f, &crate->pos);
              }
            }
            if (crate->timer > 3.0f) {
              crate->timer = 3.0f;
            }
            if (crate->timer == 3.0f) {
              CrateOff(group, crate, 0, 0);
              HopCratesAbove(CRATEHOPSPEED, group, crate);
            } else if (crate->timer < 1.0f) {
              crate->subtype = 0x16;
            } else if (crate->timer < 2.0f) {
              crate->subtype = 0x17;
            } else {
              crate->subtype = 0x18;
            }
          } else {
            crate->subtype = type;
          }
          break;
        case 8:
          old = crate->subtype;
          crate->timer += 0.016666668f;
          if (crate->timer >= crate->duration) {
            crate->i++;
            switch (crate->i) {
              case 1:
                type = crate->type3;
                if (type == -1) {
                  crate->i = 0;
                }
                break;
              case 2:
                type = crate->type4;
                if (type == -1) {
                  crate->i = 0;
                }
                break;
              default:
                crate->i = 0;
                break;
            }
            crate->timer = 0.0f;
            if ((crate->flags & 8) != 0) {
              crate->duration *= 0.9f;
              if (crate->duration < 0.016666668f) {
                crate->newtype = 0xf;
                sfx = 0x82;
                crate->subtype = -1;
                NewBuzz(&player->rumble, 6);
              }
            } else if (player->used != 0) {
              NuVecSub(&v, &crate->pos, &player->obj.pos);
              if ((v.x * v.x + v.y * v.y + v.z * v.z) < 25.0f) {
                crate->flags |= 8;
              }
            }
          }
          if (crate->newtype == -1) {
            switch (crate->i) {
              case 1:
                crate->subtype = crate->type3;
                break;
              case 2:
                crate->subtype = crate->type4;
                break;
              default:
                crate->subtype = 8;
                break;
            }
            if (crate->subtype != old) {
              GameSfx(SFX_CHANGER, &crate->pos);
            }
          }
          break;
        case 6:
          if (crate->timer > 0.0f) {
            crate->timer += 0.016666668f;
            if ((((crate->flags & 0x20) != 0) && (crate->timer >= 2.5f)) ||
                (((crate->flags & 0x20) == 0) && (crate->timer >= 5.0f))) {
              crate->timer = ((crate->flags & 0x20) != 0) ? 2.5f : 5.0f;
              crate->counter = 0;
            }
          }
          break;
        case 0x10:
          if (qrand() <= 0xff) {
            sfx = 0x34;
          }
          break;
        case 3:
          if (qrand() <= 0x7ff) {
            sfx = 0;
          }
          break;
        default:
SkipContact:
          if (crate->timer != 0.0f) {
            crate->timer = 0.0f;
          }
          break;
      }
      if (crate->model != NULL) {
        crate->model->draw = 0;
        if (crate->on != 0) {
          crate->model->draw = 1;
        }
      }
      if (crate->on == 0) {
        goto EndCrate;
      }
      if ((crate->flags & 0x400) != 0) {
        crate->pos.y = crate->pos0.y + y_adjust;
        crate->mom = crate->pos.y - crate->oldy;
        goto EndCrate;
      }
      if (((crate->flags & 1) != 0) || ((crate->flags & 0x800) != 0)) {
        contact = 0;
        crate->pos.y += crate->mom;
        if (crate->pos.y <= crate->shadow) {
          if (crate->mom < 0.0f) {
            contact = 1;
            if ((crate->flags & 0x800) != 0) {
              type = GetCrateType(crate, 0);
              BreakCrate(group, crate, type, 0);
              if ((type == 5) && (Level != 0x1d)) {
                AddGliderHitPoints(0x19);
              }
              goto EndLoop;
            }
            crate->mom = 0.0f;
            crate->pos.y = crate->shadow;
            goto DoHop;
          }
        } else {
          crate2 = &Crate[group->iCrate];
          for(k = 0; k < group->nCrates; k++, crate2++) {
            if ((((crate2 != crate) && (crate2->on != 0)) && (crate2->dx == crate->dx)) && (crate2->dz == crate->dz)) {
              if (crate2->dy < crate->dy) {
                if (crate->pos.y < crate2->pos.y + 0.5f) {
                  crate->pos.y = crate2->pos.y + 0.5f;
                }
                if (crate2->pos.y + 0.5f == crate->pos.y) {
                  contact = 1;
                  break;
                }
              } else if (type == 0xf) {
                if (crate->pos.y + 0.5f > crate2->pos.y) {
                  crate->pos.y = crate2->pos.y - 0.5f;
                }
                if (crate2->pos.y == crate->pos.y + 0.5f) {
                  crate->mom = -0.016666668f;
                  if (GetCrateType(crate2, 0) == 0x10) {
                    CrateOff(group, crate2, 0, 0);
                  }
                  goto PostMotion;
                }
              }
            }
          }
          if (contact != 0) {
            if ((type == 0xf) && (GetCrateType(crate2, 0) == 0xd)) {
              crate->mom = METALCRATEBOUNCESPEED;
              NewCrateAnimation(crate2, 0xd, 0x58, 0);
            } else {
              crate->mom = (crate->mom + crate2->mom) * 0.5f;
            }
            goto DoHop;
          } else {
            crate->mom += CRATEGRAVITY;
          }
        }
        goto PostMotion;
DoHop:
        if (qrand() <= 0xfff) {
          type = GetCrateType(crate, 0);
          if ((type == 0x10) || (type == 3)) {
            speed = qrand() * 0.000015259022f * CRATEHOPSPEED;
            crate->mom = speed;
            HopCratesAbove(speed, group, crate);
          }
        }
PostMotion:
        if (contact != 0) {
          type = GetCrateType(crate, 0);
          if ((type == 9) && (crate->timer == 0.0f) && ((crate->flags & 2) == 0)) {
            crate->timer = 0.016666668f;
          } else if ((type == 8) && (crate->newtype == -1) && (crate->subtype == 9)) {
            crate->timer = 0.016666668f;
            crate->newtype = 9;
          }
        }
      }
EndCrate:
      if (sfx != -1) {
        GameSfx(sfx, &crate->pos);
      }
      if (crate->mom > 0.2f) {
        crate->mom = 0.2f;
      } else if (crate->mom < -0.2f) {
        crate->mom = -0.2f;
      }
EndLoop:
      ;
    }
  }
  return;
}

//NGC MATCH
void InitCrateExplosions(void) {
  s32 i;
  
  iBOXEXP = 0;
  for(i = 0; i < 0x10; i++) {
    BoxExpList[i].time = 0;
  }
  return;
}

void AddCrateExplosion(struct nuvec_s *pos,s32 type,s32 ang,struct nuvec_s *colbox) {
  BoxExpType *box;
  s32 i;
  s32 j;
  BoxPolType *BoxFace;
  f32 tang;

  box = &BoxExpList[iBOXEXP];
  BoxFace = box->BoxPol;
  box->type = type;
  box->time = CRATEEXPLOSIONFRAMES;
  box->colbox[0] = colbox[0];
  box->colbox[1] = colbox[1];
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 6; j++, BoxFace++) {
      BoxFace->rndfade = qrand() / 0x1000;
      BoxFace->ang.x = 0;
      BoxFace->ang.y = ang + i * 0x4000;
      BoxFace->ang.z = 0;
      BoxFace->angmom.x = qrand() / 0x40;
      BoxFace->angmom.y = qrand() / 0x40;
      BoxFace->angmom.z = qrand() / 0x40;
      BoxFace->pos = *pos;
      BoxFace->pos.y += 0.25f;
      BoxFace->pos.x -= NuTrigTable[BoxFace->ang.y & 0xffff] * 0.25f;
      BoxFace->pos.z -= NuTrigTable[(BoxFace->ang.y + 0x4000) & 0xffff] * 0.25f;
      tang = (f32)(qrand() / 4 - 0x2000);
      tang += BoxFace->ang.y;
      BoxFace->mom.x = NuTrigTable[(s32)tang & 0xffff] * 0.25f * -0.12f;
      BoxFace->mom.y = (f32)qrand() / 1966080.0f + 0.035f;
      BoxFace->mom.z = NuTrigTable[(s32)(tang + 16384.0f) & 0xffff] * 0.25f * -0.12f;
    }
  }
  for(j = 0; j < 6; j++, BoxFace++) {
    BoxFace->rndfade = qrand() / 0x1000;
    BoxFace->ang.x = 0x4000;
    BoxFace->ang.y = ang;
    BoxFace->ang.z = 0;
    BoxFace->angmom.x = qrand() / 0x40;
    BoxFace->angmom.y = qrand() / 0x40;
    BoxFace->angmom.z = qrand() / 0x40;
    BoxFace->pos = *pos;
    BoxFace->pos.y += 0.5f;
    tang = (f32)qrand();
    BoxFace->mom.x = NuTrigTable[(s32)tang & 0xffff] * 0.25f * 0.5f * -0.1f;
    BoxFace->mom.y = (f32)qrand() / 3276800.0f + 0.035f;
    BoxFace->mom.z = NuTrigTable[(s32)(tang + 16384.0f) & 0xffff] * 0.25f * 0.5f * -0.1f;
  }
  iBOXEXP++;
  if (iBOXEXP == 0x10) {
    iBOXEXP = 0;
  }
  return;
}

//NGC MATCH
void UpdateCrateExplosions(void) {
  BoxExpType *box;
  BoxPolType *face;
  s32 i;
  s32 j;
  s32 k;
  s32 bVar6;
  
  box = BoxExpList;
  for(i = 0; i < 0x10; i++, box++) {
    if (box->time != 0) {
      box->time--;
      if (box->time != 0) {
          face = BoxExpList[i].BoxPol;
          for(j = 0; j < 5; j++) {
                for(k = 0; k < 6; k++, face++) {
                  bVar6 = 0;
                  face->pos.x = face->pos.x + face->mom.x;
                  face->pos.y = face->pos.y + face->mom.y;
                  face->mom.y = face->mom.y - 0.004f;
                  face->pos.z = face->pos.z + face->mom.z;
                  (face->ang).x = (face->ang).x + face->angmom.x;
                  (face->ang).y = (face->ang).y + face->angmom.y;
                  (face->ang).z = (face->ang).z + face->angmom.z;
                  if ((face->pos.x < box->colbox[0].x) && (face->mom.x < 0.0f)) {
                    face->pos.x = box->colbox[0].x;
                    bVar6 = 1;
                    face->mom.x = -face->mom.x * 0.8f;
                  }
                  if ((face->pos.y < box->colbox[0].y) && (face->mom.y < 0.0f)) {
                    face->pos.y = box->colbox[0].y;
                    bVar6 = 1;
                    face->mom.y = -face->mom.y * 0.8f;
                  }
                  if ((face->pos.z < box->colbox[0].z) && (face->mom.z < 0.0f)) {
                    face->pos.z = box->colbox[0].z;
                    bVar6 = 1;
                    face->mom.z = -face->mom.z * 0.8f;
                  }
                  if (face->pos.x > (box->colbox[1].x) && (0.0f < face->mom.x)) {
                    face->pos.x = box->colbox[1].x;
                    bVar6 = 1;
                    face->mom.x = -face->mom.x * 0.8f;
                  }
                  if (face->pos.z > (box->colbox[1].z) && (0.0f < face->mom.z)) {
                    face->pos.z = box->colbox[1].z;
                    bVar6 = 1;
                    face->mom.z = -face->mom.z * 0.8f;
                  }
                  if (bVar6) {
                    face->angmom.x = -face->angmom.x;
                    face->angmom.y = -face->angmom.y;
                    face->angmom.z = -face->angmom.z;
                  }
                }
          }
      }
    }
  }
  return;
}

void DrawCrateExplosions(void) {
  static struct nuvec_s shape[4] = {
    { -0.25f,  0.25f, 0.0f },
    {  0.25f,  0.25f, 0.0f },
    { -0.25f, -0.25f, 0.0f },
    {  0.25f, -0.25f, 0.0f },
  };
  BoxPolType *BoxFace;
  float *uvs_regular;
  float *uvs_mat2;
  float *uvs_fade_regular;
  float *uvs_fade_mat2;
  s32 lp;
  s32 lpo;
  s32 lp2;
  s32 fade;
  s32 col;
  s32 col2;
  s32 a;
  s32 r;
  s32 g;
  s32 b;

  for (lp = 0; lp < 0x10; lp++) {
    if (BoxExpList[lp].time != 0) {
      BoxFace = BoxExpList[lp].BoxPol;
      for (lpo = 0; lpo < 5; lpo++) {
        if (BoxExpList[lp].type == 0x10) {
          if (lpo == 4) {
            col = 0x10C020;
          }
          else {
            col = 0x20FF20;
          }
        }
        else if (BoxExpList[lp].type == 9) {
          if (lpo == 4) {
            col = 0xFF0000;
          }
          else {
            col = 0x800000;
          }
        }
        else {
          if (lpo == 4) {
            col = 0x808080;
          }
          else {
            col = 0x606060;
          }
        }
        uvs_regular = uvs;
        uvs_mat2 = uvs;
        uvs_fade_regular = uvs;
        uvs_fade_mat2 = uvs;
        for (lp2 = 6; lp2 != 0; lp2--) {
          fade = BoxExpList[lp].time - BoxFace->rndfade;
          if (fade > 0) {
            if (fade <= 6) {
              col2 = col + (fade << 0x1c);
              if ((u16)(BoxExpList[lp].type - 10) <= 2) {
                a = (col2 >> 0x17) & 0x1fe;
                if (a > 0xff) {
                  a = 0xff;
                }
                r = (col2 & 0xff) << 1;
                if (r > 0xff) {
                  r = 0xff;
                }
                g = (col2 >> 7) & 0x1fe;
                if (g > 0xff) {
                  g = 0xff;
                }
                b = (col2 >> 0xf) & 0x1fe;
                if (b > 0xff) {
                  b = 0xff;
                }
                col2 = a << 0x18;
                col2 += r << 0x10;
                col2 += g << 8;
                col2 += b;
                AddQuad3DrotXYZ(&BoxFace->pos,shape,CrateMat2,&BoxFace->ang,uvs_fade_mat2,col2);
              }
              else {
                a = (col2 >> 0x17) & 0x1fe;
                if (a > 0xff) {
                  a = 0xff;
                }
                r = (col2 & 0xff) << 1;
                if (r > 0xff) {
                  r = 0xff;
                }
                g = (col2 >> 7) & 0x1fe;
                if (g > 0xff) {
                  g = 0xff;
                }
                b = (col2 >> 0xf) & 0x1fe;
                if (b > 0xff) {
                  b = 0xff;
                }
                col2 = a << 0x18;
                col2 += r << 0x10;
                col2 += g << 8;
                col2 += b;
                AddQuad3DrotXYZ(&BoxFace->pos,shape,CrateMat,&BoxFace->ang,uvs_fade_regular,col2);
              }
            }
            else {
              col2 = col + 0x80000000;
              if ((u16)(BoxExpList[lp].type - 10) <= 2) {
                a = (col2 >> 0x17) & 0x1fe;
                if (a > 0xff) {
                  a = 0xff;
                }
                r = (col2 & 0xff) << 1;
                if (r > 0xff) {
                  r = 0xff;
                }
                g = (col2 >> 7) & 0x1fe;
                if (g > 0xff) {
                  g = 0xff;
                }
                b = (col2 >> 0xf) & 0x1fe;
                if (b > 0xff) {
                  b = 0xff;
                }
                col2 = a << 0x18;
                col2 += r << 0x10;
                col2 += g << 8;
                col2 += b;
                AddQuad3DrotXYZ(&BoxFace->pos,shape,CrateMat2,&BoxFace->ang,uvs_mat2,col2);
              }
              else {
                a = (col2 >> 0x17) & 0x1fe;
                if (a > 0xff) {
                  a = 0xff;
                }
                r = (col2 & 0xff) << 1;
                if (r > 0xff) {
                  r = 0xff;
                }
                g = (col2 >> 7) & 0x1fe;
                if (g > 0xff) {
                  g = 0xff;
                }
                b = (col2 >> 0xf) & 0x1fe;
                if (b > 0xff) {
                  b = 0xff;
                }
                col2 = a << 0x18;
                col2 += r << 0x10;
                col2 += g << 8;
                col2 += b;
                AddQuad3DrotXYZ(&BoxFace->pos,shape,CrateMat,&BoxFace->ang,uvs_regular,col2);
              }
            }
          }
          BoxFace++;
          uvs_regular += 8;
          uvs_mat2 += 8;
          uvs_fade_regular += 8;
          uvs_fade_mat2 += 8;
        }
      }
    }
  }
  return;
}

//NGC MATCH
float CrateTopBelow(struct nuvec_s *pos) {
  CrateCubeGroup *group;
  CrateCube *crate;
  s32 i;
  s32 j;
  float top;
  float y;
  struct nuvec_s vNew;
  
  y = 2000000.0f;
  group = CrateGroup;
  for(i = 0; i < CRATEGROUPCOUNT; i++, group++) {
      if (((!(pos->x < group->minclip.x) && !(pos->x > (group->maxclip).x)) &&
          !(pos->z < group->minclip.z)) && !(pos->z > (group->maxclip).z)) {
        vNew.x = pos->x - (group->origin).x;
        vNew.z = pos->z - (group->origin).z;
        NuVecRotateY(&vNew,&vNew,-(u32)group->angle);
        crate = &Crate[group->iCrate];
        for (j = 0; j < group->nCrates; j++, crate++) {
            if (((((crate->on != 0) && ( GetCrateType(crate,0) + 1U > 1)) &&
                 ((top = (crate->pos.y + 0.5f), !(pos->y < top) &&
                  ( !(vNew.x < (crate->dx * 0.5f) || (vNew.x > (crate->dx * 0.5f) + 0.5f))))))
                && !(vNew.z < crate->dz * 0.5f)) &&
               (!(vNew.z > (crate->dz * 0.5f) + 0.5f || ((y != 2000000.0f && !(top > y)))))) {
              y = top;
            }
        }
      }
  }
  return y;
}

//NGC MATCH
void DestroyAllNitroCrates(CrateCubeGroup *group,CrateCube *crate) {
  CrateCubeGroup *group2;
  CrateCube *crate2;
  struct nuvec_s pos;
  s32 i;
  s32 j;
  
  group2 = CrateGroup;
  for(i = 0; i < CRATEGROUPCOUNT; i++, group2++) {
      crate2 = Crate + group2->iCrate;
      for(j = 0; j < group2->nCrates; j++, crate2++) {
          if ((crate2->on != 0) && (GetCrateType(crate2,0) == 0x10)) {
            CrateOff(group2,crate2,0,0);
          }
      }
  }
  pos.x = (crate->pos).x;
  pos.y = (crate->pos).y + 0.25f;
  pos.z = (crate->pos).z;
  temp_pGroup = group;
  temp_pCrate = crate;
  AddKaboom(0x20,&pos,0.0f);
  crate->newtype = 0xf;
  crate->metal_count = 1;
  JudderGameCamera(GameCam,0.5f,NULL);
  GameSfx(0x33,&crate->pos);
  return;
}

//NGC MATCH
s32 AttackCrate(struct obj_s *obj,CrateCubeGroup *group,CrateCube *crate) {
  s32 type;
  
  type = GetCrateType(crate,0);
  if (type == -1) {
    return 1;
  }
    if (type == 0xe) {
      if ((crate->action == -1) && (NewCrateAnimation(crate,0xe,0x35,0) == 0)) {
        StartExclamationCrateSequence(group,crate);
      }
      return 2;
    }
    if (type == 0x11) {
        if ((crate->action == -1) && (NewCrateAnimation(crate,0x11,0x35,0) == 0)) {
            DestroyAllNitroCrates(group,crate);
        }
        return 2;
    }
    if (CrateOff(group,crate,0,obj->attack >> 9 & 1) != 0) {
        if ((type == 9) || (type == 0x10)) {
                KillPlayer(obj,GetDieAnim(obj,6));   
        }
        return 1;
    }
  return 0;
}

//NGC MATCH
s32 LowestCrate(CrateCubeGroup *group,CrateCube *crate) {
  CrateCube *crate2;
  s32 i;
  s32 dx;
  s32 dz;
  
  crate2 = &Crate[group->iCrate];
  dx = crate->dx;
  dz = crate->dz;
  for(i = 0; i < group->nCrates; i++, crate2++) {
      if ((((crate2 != crate) && (crate2->dx == dx)) && (crate2->dz == dz)) &&
         ((crate2->pos0).y < (crate->pos0).y)) {
        return 0;
      }
  }
  return 1;
}

//NGC MATCH
s32 LowestActiveCrate(CrateCubeGroup *group,CrateCube *crate) {
  CrateCube *crate2;
  s32 i;
  s32 dx;
  s32 dz;
  
  crate2 = &Crate[group->iCrate];
  dx = crate->dx;
  dz = crate->dz;
  for(i = 0; i < group->nCrates; i++, crate2++) {
      if ((((crate2 != crate) && (crate2->on != 0)) && (crate2->dx == dx)) &&
         ((crate2->dz == dz && ((crate2->pos).y < (crate->pos).y)))) {
        return 0;
      }
  }
  return 1;
}

//NGC MATCH
s32 CrateInTheWay(struct obj_s *obj,CrateCubeGroup *group,CrateCube *crate,s32 dx,s32 dz,char *hit) {
  CrateCube *crate2;
  s32 i;
  
  crate2 = &Crate[group->iCrate];
  for(i = 0; i < group->nCrates; i++, crate2++) {
      if ((((crate2 != crate) && (hit[i] == 1)) && (crate2->dx == dx)) &&
         (crate2->dz == dz)) {
        return 1;
      }
  }
  return 0;
}

//NGC MATCH
s32 CrateOnTop(CrateCubeGroup *group,CrateCube *crate) {
  s32 i;
  CrateCube *crate2;
  
  crate2 = &Crate[group->iCrate];
  for(i = 0; i < group->nCrates; i++, crate2++) {
      if ((((crate2 != crate) && (crate2->on != 0)) && (GetCrateType(crate2,0) != 0)) &&
         (((crate2->dx == crate->dx && (crate2->dz == crate->dz)) &&
          (crate2->pos.y == crate->pos.y + 0.5f)))) {
        return 1;
      }
  }
  return 0;
}

CrateCube* InCrate(float x,float z,float ytop,float ybot,float radius) {
  CrateCubeGroup *group;
  CrateCube *crate;
  s32 i;
  s32 j;
  s32 type;
  struct nuvec_s vNew;
  float ymid;
  float d;
  float dbest;
  float dx;
  float dy;
  float dz;
  float temp;
  
  ymid = (ytop + ybot) * 0.5f;
  temp_pGroup = NULL;
  temp_pCrate = NULL;
  group = CrateGroup;
  for(i = 0; i < CRATEGROUPCOUNT; i++, group++) {
      if (((!(x < (group->minclip.x - radius)) && !(x > (group->maxclip.x + radius))) &&
          !(z < (group->minclip.z - radius))) && !(z > (group->maxclip.z + radius))) {
        vNew.x = x - group->origin.x;
        vNew.z = z - group->origin.z;
        NuVecRotateY(&vNew,&vNew,-(u32)group->angle);
        crate = &Crate[group->iCrate];
        for(j = 0; j < group->nCrates; j++, crate++) {
            if (crate->on != 0) {
              type = GetCrateType(crate,0);
              if (!(type == -1 || type == 0)) {
                temp = (s32)crate->dx * 0.5f;
                if (!(vNew.x < (temp - radius)) && !(vNew.x > ((temp + 0.5f) + radius))) {
                  temp = (s32)crate->dz * 0.5f;
                  if (!(vNew.z < (temp - radius)) && !(vNew.z > ((temp + 0.5f) + radius))) {
                    if (!(ytop < crate->pos.y) && !(ybot > (crate->pos.y + 0.5f))) {
                      dy = crate->pos.y + 0.25f;
                      dx = crate->pos.x;
                      dz = crate->pos.z;
                      dx -= x;
                      dy -= ymid;
                      dz -= z;
                      d = dx * dx + dy * dy + dz * dz;
                      if ((temp_pCrate == NULL) || (d < dbest)) {
                        dbest = d;
                        temp_crate_type = type;
                        temp_pGroup = group;
                        temp_pCrate = crate;
                      }
                    }
                  }
                }
              }
            }
        }
      }
  }
  return temp_pCrate;
}

//NGC MATCH
void BreakCrate(CrateCubeGroup *group,CrateCube *crate,s32 type,s32 attack) {
  if ((type == 0x13) && ((attack & 0x200U) == 0)) {
    if (crate->action == -1) {
      NewCrateAnimation(crate,0x13,0x58,0);
      GameSfx(0x39,&crate->pos);
    }
  }
  else if (type == 0xe) {
    if ((crate->action == -1) && (NewCrateAnimation(crate,0xe,0x35,0) == 0)) {
      StartExclamationCrateSequence(group,crate);
    }
  }
  else if (type == 0x11) {
    if ((crate->action == -1) && (NewCrateAnimation(crate,0x11,0x35,0) == 0)) {
      DestroyAllNitroCrates(group,crate);
    }
  }
  else {
    if (CrateOff(group,crate,0,(attack >> 9) & 1) != 0) {
      HopCratesAbove(CRATEHOPSPEED,group,crate);
    }
  }
  return;
}

//NGC MATCH
s32 HitCrates(struct obj_s *obj,s32 destroy) {
  if ((level_part_2 == 0) &&
     (InCrate(obj->pos.x,obj->pos.z,(obj->top * obj->SCALE + obj->pos.y),(obj->bot * obj->SCALE + obj->pos.y),obj->RADIUS) != 0)) {
        if ((destroy == 1) || ((((destroy == 2 && (temp_crate_type != 7)) && (temp_crate_type != 0xe)) && (temp_crate_type != 0x11)))) {
          BreakCrate(temp_pGroup,temp_pCrate,temp_crate_type,(uint)obj->attack);
        }
        return 1;
  }
  else {
    return 0;
  }
}

//NGC MATCH
s32 WipeCrates(s32 iRAIL0,s32 iALONG0,float fALONG0,s32 iRAIL1,s32 iALONG1,float fALONG1,s32 destroy) {
  CrateCubeGroup *group;
  CrateCube *crate;
  s32 i;
  s32 j;
  s32 type;

  group = CrateGroup;
  for(i = 0; i < CRATEGROUPCOUNT; i++, group++) {
      crate = &Crate[group->iCrate];
      for(j = 0; j < group->nCrates; j++, crate++) {
        if (crate->on != 0) {
        type = GetCrateType(crate,0);
            if((u32)(type + 1) > 1 && 
                (FurtherALONG(crate->iRAIL,crate->iALONG,crate->fALONG,iRAIL0,iALONG0,fALONG0) != 0) &&
                  (FurtherALONG(iRAIL1,iALONG1,fALONG1,crate->iRAIL,crate->iALONG,crate->fALONG) != 0) &&
                 ((destroy == 1) || ((destroy == 2 && (type != 7)) && (type != 0xe && (type != 0x11))))) {
                BreakCrate(group,crate,type,0x200);
            }
        }
      }
  }
  return 0;
}

//NGC MATCH
CrateCube * HitCrateBalloons(struct nuvec_s *pos,float radius) {
  CrateCubeGroup *group;
  CrateCube *crate;
  float r2;
  s32 iVar1;
  s32 iVar3;
  struct nuvec_s v;
  struct nuvec_s d;
  
  temp_pGroup = NULL;
  temp_pCrate = NULL;
  if (level_part_2 != 0) {
    return temp_pCrate;
  }
    group = CrateGroup;
    r2 = (radius + CRATEBALLOONRADIUS);
    r2 *= r2;
    for(iVar1 = 0; iVar1 < CRATEGROUPCOUNT; iVar1++, group++) {
        crate = Crate + group->iCrate;
        for(iVar3 = 0; iVar3 < group->nCrates; iVar3++, crate++) {
            if ((crate->on != 0) && ((crate->flags & 0x400) != 0)) {
              v.x = (crate->pos).x;
              v.y = (crate->pos).y + CRATEBALLOONOFFSET;
              v.z = (crate->pos).z;
              NuVecSub(&d,pos,&v);
              if ((d.x * d.x + d.y * d.y + d.z * d.z) < r2) {
                temp_crate_type = GetCrateType(crate,0);
                temp_pGroup = group;
                temp_pCrate = crate;
                GameSfx(0x50,&v);
                return temp_pCrate;
              }
            }
        }
    }
  return temp_pCrate;
}

//NGC MATCH
s32 RayIntersectCuboid(struct nuvec_s *p0,struct nuvec_s *p1,struct nuvec_s *min,struct nuvec_s *max) {
  float new_ratio;
  float dx;
  float dy;
  float dz;
  s32 face;
  struct nuvec_s v;
  
  dx = (p1->x - p0->x);
  dy = (p1->y - p0->y);
  dz = (p1->z - p0->z);
  if ((p0->x <= min->x) && (p1->x >= min->x)) {
    new_ratio = ((min->x - p0->x) / dx);
    face = 8;
    v.x = min->x;
  }
  else {
    if ((p0->x >= max->x) && (p1->x <= max->x)) {
      new_ratio = ((max->x - p0->x) / dx);
      face = 4;
      v.x = max->x;
    }
    else {
      face = 0;
    }
  }
  if (face != 0) {
    v.z = (dz * new_ratio + p0->z);
    if ((v.z >= min->z) && (v.z <= max->z)) {
        v.y = (dy * new_ratio + p0->y);
         if ((v.y >= min->y) && (v.y <= max->y)) {
            temp_ratio = new_ratio;
            temp_face = face;
            vTEMP = v;
            return 1;
        }
    }
  }
    if ((p0->z <= min->z) && (p1->z >= min->z)) {
      new_ratio = ((min->z - p0->z) / dz);
      face = 2;
      v.z = min->z;
    }
    else {
      if ((p0->z >= max->z) && (p1->z <= max->z)) {
        new_ratio = ((max->z - p0->z) / dz);
        face = 1;
        v.z = max->z;
      }
      else {
        face = 0;
      }
    }
    if (face != 0) {
      v.x = (dx * new_ratio + p0->x);
      if ((v.x >= min->x) && (v.x <= max->x)) {
        v.y = (dy * new_ratio + p0->y);
        if ((v.y >= min->y) && (v.y <= max->y)) {
            temp_ratio = new_ratio;
            temp_face = face;
            vTEMP = v;
            return 1;
        }
      }
    }
    if ((p0->y <= min->y) && (p1->y >= min->y)) {
      new_ratio = ((min->y - p0->y) / dy);
      face = 0x20;
      v.y = min->y;
    }
    else {
      if ((p0->y >= max->y) && (p1->y <= max->y)) {
        new_ratio = ((max->y - p0->y) / dy);
        face = 0x10;
        v.y = max->y;
      }
      else {
        face = 0;
      }
    }
    if (face != 0) {
      v.x = (dx * new_ratio + p0->x);
      if ((v.x >= min->x) && (v.x <= max->x)) {
        v.z = (dz * new_ratio + p0->z);
        if ((v.z >= min->z) && (v.z <= max->z)) {
            temp_ratio = new_ratio;
            temp_face = face;
            vTEMP = v;
            return 1;
        }
      }
    }
  return 0;
}

//NGC MATCH
s32 CrateRayCast(struct nuvec_s *p0,struct nuvec_s *p1) {
  struct nuvec_s vMIN;
  struct nuvec_s vMAX;
  struct nuvec_s v0;
  struct nuvec_s v1;
  struct nuvec_s min;
  struct nuvec_s max;
  s32 i;
  s32 j;
  s32 face;
  s32 type;
  float ratio;
  CrateCubeGroup *group;
  CrateCube *crate;
  
  ratio = 1.0f;
  vMIN.x = (p0->x < p1->x) ? p0->x : p1->x;
  vMIN.z = (p0->z < p1->z) ? p0->z : p1->z;
  vMAX.x = (p0->x > p1->x) ? p0->x : p1->x;
  vMAX.z = (p0->z > p1->z) ? p0->z : p1->z;
  group = CrateGroup;
  for(i = 0; i < CRATEGROUPCOUNT; i++, group++) {
      if ((((vMAX.x >= group->minclip.x) && (vMIN.x <= group->maxclip.x)) &&
          (vMAX.z >= group->minclip.z )) && (vMIN.z <= group->maxclip.z)) {
        v0.x = p0->x - (group->origin).x;
        v0.y = p0->y;
        v0.z = p0->z - (group->origin).z;
        NuVecRotateY(&v0,&v0,-(uint)group->angle);
        v1.x = p1->x - (group->origin).x;
        v1.y = p1->y;
        v1.z = p1->z - (group->origin).z;
        NuVecRotateY(&v1,&v1,-(uint)group->angle);
        crate = &Crate[group->iCrate];
        for(j = 0; j < group->nCrates; j++, crate++) {
           if ((crate->on != 0) && (crate->in_range != 0)) {
                type = GetCrateType(crate,0) + 1;
                 if ((u32)type > 1) {
                  min.x = ((s32)crate->dx * 0.5f);
                  min.y = crate->pos.y;
                  min.z = (crate->dz * 0.5f);
                  max.x = (min.x + 0.5f);
                  max.y = min.y + 0.5f;
                  max.z = (min.z + 0.5f);
                  if ((RayIntersectCuboid(&v0,&v1,&min,&max) != 0) && (temp_ratio < ratio)) {
                    face = temp_face;
                    ratio = temp_ratio;
                  }
                }
            }
        }
      }
  }
  temp_face = face;
  temp_ratio = (float)ratio;
  if (ratio < 1.0f) {
      return 1;
  }
  return 0;
}

//NGC MATCH
s32 GotoCheckpoint(struct nuvec_s *pos,s32 direction) {
  CrateCubeGroup *group;
  CrateCube *crate;
  s32 i;
  s32 j;
  s32 iRAIL;
  s32 iALONG;
  float fALONG;
  
  cp_goto = -1;
  if ((best_cRPos == NULL) || ((u32)direction > 1)) {
      return 0;
  }
    group = CrateGroup;
    iRAIL = -1;
    for(i = 0; i < CRATEGROUPCOUNT; i++, group++) {
        crate = Crate + group->iCrate;
        for(j = 0; j < group->nCrates; j++, crate++) {
            if ((((crate->type1 == 7) && (NuVecDist(pos,&crate->pos0,NULL) > 5.0f)) && ((((direction == 0) &&
                  (FurtherALONG(crate->iRAIL,crate->iALONG,crate->fALONG,
                                        best_cRPos->iRAIL,best_cRPos->iALONG,
                                        best_cRPos->fALONG) != 0)) || ((direction == 1 &&
                  (FurtherBEHIND(crate->iRAIL,crate->iALONG,crate->fALONG,
                                         best_cRPos->iRAIL,best_cRPos->iALONG,
                                         best_cRPos->fALONG) != 0))))))
                && ((iRAIL == -1 || (((direction == 0 &&
                  (FurtherBEHIND(crate->iRAIL,crate->iALONG,crate->fALONG,iRAIL,iALONG,fALONG) != 0)) ||
                 ((direction == 1 &&
                   (FurtherALONG(crate->iRAIL,crate->iALONG,crate->fALONG,iRAIL,iALONG,fALONG) != 0)))))))) {
              cpGOTO.x = crate->pos.x;
              cpGOTO.y = (crate->pos0.y + 0.5f) + 1.0f;
              cpGOTO.z = crate->pos.z;
              iRAIL = crate->iRAIL;
              iALONG = crate->iALONG;
              fALONG = crate->fALONG;
            }
        }
    }
    if (iRAIL != -1) {
        cp_goto = direction;
        return 1;
    }
    return 0;
}

//NGC MATCH (temp var)
s32 CrateBounceReaction(CrateCubeGroup *group, CrateCube *crate, s32 type, s32 hit) {
    s32 gone;
    s32 sfx;
    s32 bounce;

    bounce = 0;
    gone = 0;
    sfx = -1;

    if (type != 0xf) {
        if (type == 9) {
            if ((crate->timer > 0.0f) && (crate->newtype != -1)) {
                goto LAB_80013a4c;
            }
            if (((TimeTrial == 0) ? crate->type1 : crate->type2) == 8) {
                if ((crate->newtype == -1) && (crate->subtype == 0x09)) {
                    sfx = 2;
                    bounce |= 1;
                    crate->newtype = 9;
                    crate->timer = 0.01666667f;
                    goto LAB_80013a4c;
                }
            }
            if (type == 9) {
                if (crate->timer == 0.0f) {
                    sfx = 2;
                    bounce |= 1;
                    crate->timer += 0.01666667f;
                }
                goto LAB_80013a4c;
            }
        }
        
        if (type == 0xe) {
            if (crate->action == -1) {
                sfx = 0xe;
                bounce |= 1;
                if (NewCrateAnimation(crate, 0xe, 0x35, 0) == 0) {
                    StartExclamationCrateSequence(group, crate);
                }
            }
        } else if (type == 0x11) {
            if (crate->action == -1) {
                sfx = 0xe;
                bounce |= 1;
                if (NewCrateAnimation(crate, 0x11, 0x35, 0) == 0) {
                    DestroyAllNitroCrates(group, crate);
                }
            }
        } else if (type == 0x13) {
            NewCrateAnimation(crate, 0x13, 0x58, 0);
            GameSfx(0x38, &crate->pos);
        } else if (type == 6) {
            sfx = 2;
            if (crate->timer == 0.0f) {
                crate->timer += 0.01666667f;
            }
            if (crate->counter > 1) {
                crate->counter--;
                if ((TimeTrial == 0) && ((player->obj).dead == 0)) {
                    AddScreenWumpa((crate->pos).x, (crate->pos).y + 0.25f, (crate->pos).z, 2);
                }
                NewCrateAnimation(crate, type, (hit == 2) ? 0x58 : 0x16, 0);
            } else {
                float f = crate->timer;
                if (((crate->flags & 0x20) != 0 && (f < 2.5f)) ||
                    ((crate->flags & 0x20) == 0 && (f < 5.0f))
                   ) {
                    if ((TimeTrial == 0) && ((player->obj).dead == 0)) {
                        AddScreenWumpa((crate->pos).x, (crate->pos).y + 0.25f, (crate->pos).z, 2);
                    }
                }
                gone = CrateOff(group, crate, 0, 0);
            }
            bounce |= 1;
        } else {
            if ((type == 4) || (type == 0xd)) {
                NewCrateAnimation(crate, type, 0x58, 0);
                bounce |= 3;
                sfx = 0xe;
                if (type == 4) {
                    sfx = 2;
                }
            } else {
                gone = CrateOff(group, crate, 0, 0);
                sfx = 2;
                bounce |= 1;
            }
        }
    }

LAB_80013a4c:
    if (VEHICLECONTROL != 2) {
        temp_crate_bounce = temp_crate_bounce | bounce;
        if (bounce != 0) {
            NewRumble(&player->rumble, 0x7f);
            NewBuzz(&player->rumble, 0xc);
        }
    }

    if (sfx != -1) {
        GameSfx(sfx, &crate->pos);
    }

    return gone;
}

//NGC MATCH
void CrateSafety(CrateCubeGroup *group,CrateCube *crate,struct obj_s *obj) {
  CrateCube *crate2;
  CrateCube *crate3;
  float size;
  s32 i;
  
  if ((Level == 7) && (Bonus == 2)) {
    obj->pos.y = crate->pos.y - obj->top * obj->SCALE;
  }
  else {
    size = (obj->top - obj->bot) * obj->SCALE;
    if ((obj->bot + obj->top) * obj->SCALE * 0.5f + obj->pos.y < crate->pos.y + 0.25f) {
      crate2 = &Crate[group->iCrate];
      crate3 = crate;
      for(i = 0; i < group->nCrates; i++, crate2++) {
          if (((crate2->on != 0) && (crate2->dx == crate3->dx)) && (crate2->dz == crate3->dz)) {
            if (crate2->pos.y < crate3->pos.y) {
                if (crate3->pos.y - (crate2->pos.y + 0.5f) > size) {
                    break;
                }
                crate3 = crate2;
            }
          }
      }
      obj->pos.y = crate3->pos.y - obj->top * obj->SCALE;
      if ((crate3->shadow != 2000000.0f) && (crate3->pos.y - crate3->shadow > size))
      goto SetBot;
    }
    crate2 = &Crate[group->iCrate];
    crate3 = crate;
    for(i = 0; i < group->nCrates; i++, crate2++) {
        if (((crate2->on != 0) && (crate2->dx == crate3->dx)) && (crate2->dz == crate3->dz)) {
            if (crate2->pos.y > crate3->pos.y) {
                if (crate2->pos.y - (crate3->pos.y + 0.5f) > size) {
                    break;
                }
                crate3 = crate2;
            }
        }
    }
    obj->pos.y = (crate3->pos.y + 0.5f) - obj->bot * obj->SCALE;
  }
SetBot:
  NewTopBot(obj);
  OldTopBot(obj);
  obj->mom.y = 0.0f;
}

//NGC MATCH
float CrateBottomAbove(struct nuvec_s *pos) {
  CrateCubeGroup *group;
  CrateCube *crate;
  s32 i;
  s32 j;
  s32 type;
  float bot;
  float y;
  struct nuvec_s vNew;
  
  y = 2000000.0f;
  group = CrateGroup;
  for (i = 0; i < CRATEGROUPCOUNT; i++, group++) {
      if ((( !(pos->x < group->minclip.x) && !(pos->x > group->maxclip.x)) &&
          !(pos->z < group->minclip.z)) && !(pos->z > group->maxclip.z)) {
        vNew.x = pos->x - group->origin.x;
        vNew.z = pos->z - group->origin.z;
        NuVecRotateY(&vNew,&vNew,-(u32)group->angle);
        crate = &Crate[group->iCrate];
        for (j = 0; j < group->nCrates; j++, crate++) {
            if ((crate->on != 0)) {
                type = GetCrateType(crate,0);
                if ((type == 0 || type == -1)) continue;
                bot = crate->pos.y;
                if (!(pos->y > bot)) {
                    if ( ( (vNew.x < crate->dx  * 0.5f) || (vNew.x > crate->dx  * 0.5f + 0.5f))) continue;
                    if ((vNew.z < crate->dz  * 0.5f) || (vNew.z > crate->dz  * 0.5f + 0.5f)) continue;
                    if (y != 2000000.0f && !(crate->pos.y < y)) continue;
                    y = crate->pos.y;
                }

            }
        }
      }
  }
  return y;
}

//NGC MATCH
void DrawCrates(void) {
  s32 i;
  s32 j;
  s32 k;
  s32 type;
  s32 shadow;
  CrateCubeGroup *group;
  CrateCube *crate;
  struct nuspecial_s *obj;
  struct nuvec_s pos;
  struct numtx_s *m;
  float dx;
  float dz;
  float d;
  float r2;
  float shadow_r2;
  struct numtx_s mGROUP;
  struct numtx_s mCRATE;
  struct CharacterModel *model;
  float **dwa;
  
  shadow = 0;
  if (((((LDATA->flags & 0x200) == 0) && (Level != 0x1d)) || (VEHICLECONTROL != 1)) &&
     ((Level != 0x1c && (Level != 3)))) {
    r2 = 45.0f;
    if ((float)((s32)LDATA->farclip) < 45.0f) {
      r2 = (float)LDATA->farclip;
    }
    r2 *= r2;
  }
  else {
    r2 = (float)(s32)(LDATA->farclip * LDATA->farclip);
  }
  if (((((DRAWCRATESHADOWS != 0) && (VEHICLECONTROL != 2)) &&
       ((VEHICLECONTROL != 1 || ((player->obj).vehicle != 0x20)))) && ((LDATA->flags & 0x1000) == 0)
      ) && (((((LDATA->flags & 0x200) == 0 && (Level != 0x1d)) || (VEHICLECONTROL != 1)) &&
            (Level != 0xb)))) {
    shadow = 1;
    shadow_r2 = r2;
  }
  group = CrateGroup;
  for(i = 0; i < CRATEGROUPCOUNT; i++, group++) {
      NuMtxSetRotationY(&mGROUP,(uint)group->angle);
      crate = &Crate[group->iCrate];
      for(j = 0; j < group->nCrates; j++, crate++) {
          crate->in_range = 0;
          type = GetCrateType(crate,1);
          if (((type != -1) && ((crate->on != 0 || (type == 7)))) &&
             (((((LDATA->flags & 0x200) != 0 || (Level == 0x1d)) && (VEHICLECONTROL == 1)) ||
              (dx = pCam->pos.x - crate->pos.x, dz = pCam->pos.z - crate->pos.z,
              d = (dx * dx + dz * dz), !(d > r2))))) {
                crate->in_range = 1;
                pos.x = crate->pos.x;
                pos.y = crate->pos.y + 0.25f;
                pos.z = crate->pos.z;
                if ((crate->xrot != 0) || (crate->zrot != 0)) {
                  m = &mCRATE;
                  NuMtxSetRotationY(&mCRATE,(uint)group->angle);
                  NuMtxRotateZ(&mCRATE,(uint)crate->zrot);
                  NuMtxRotateX(&mCRATE,(uint)crate->xrot);
                } else {
                   m = &mGROUP;
                }
                m->_30 = pos.x;
                m->_31 = pos.y;
                m->_32 = pos.z;
                if ((crate->flags & 0x400) != 0) {
                  j = (type == 5) ? 0x43 : 0x41;
                  if (ObjTab[j].obj.special != NULL) {
                    obj = ObjTab[j].obj.special;
                    NuRndrGScnObj((ObjTab[j].obj.scene)->gobjs[obj->instance->objid],m);
                  }
                }
                k = crate_list[type].character;
                if (((u32)k < 0xbf) && (CRemap[k] != -1)) {
                  model = &CModel[CRemap[k]];
                  if ((crate->action != -1) && ((s32)crate->character == k)) {
                    if (model->fanmdata[crate->action] != NULL) {
                      dwa = NuHGobjEvalDwa(1,NULL,model->fanmdata[crate->action],crate->anim_time);
                    }
                    else {
                      dwa = NULL;
                    }
                    if (model->anmdata[crate->action] != NULL) {
                      NuHGobjEvalAnim(model->hobj,model->anmdata[crate->action],crate->anim_time,0,NULL,tmtx);
                    }
                    else {
                      NuHGobjEval(model->hobj,0,NULL,tmtx);
                    }
                  }
                  else {
                    NuHGobjEval(model->hobj,0,NULL,tmtx);
                    dwa = NULL;
                  }
                  NuHGobjRndrMtxDwa(model->hobj,m,1,NULL,tmtx,dwa);
                  if ((crate->flags & 0x200) != 0) {
                    mCRATE = *m;
                    mCRATE._01 = -mCRATE._01;
                    mCRATE._11 = -mCRATE._11;
                    mCRATE._21 = -mCRATE._21;
                    mCRATE._31 = crate->shadow - (mCRATE._31 - crate->shadow);
                    NuHGobjRndrMtxDwa(model->hobj,&mCRATE,1,NULL,tmtx,dwa);
                  }
                }
                else {
                  obj = crate_list[type].obj.special;
                  if ((obj != NULL) && (NuRndrGScnObj(crate_scene->gobjs[obj->instance->objid],m),
                     (crate->flags & 0x200) != 0)) {
                    mCRATE = *m;
                    mCRATE._01 = -mCRATE._01;
                    mCRATE._11 = -mCRATE._11;
                    mCRATE._21 = -mCRATE._21;
                    mCRATE._31 = crate->shadow - (mCRATE._31 - crate->shadow);
                    NuRndrGScnObj(crate_scene->gobjs[obj->instance->objid],&mCRATE);
                  }
                }
                if (((((shadow) && (type != 7)) && (type != 0)) &&
                    ((((crate->flags & 0x2000) != 0 && (crate->shadow != 2000000.0f)) &&
                     ((crate->pos.y > crate->shadow &&
                      ((d < shadow_r2 && (ObjTab[21].obj.special != NULL)))))))) &&
                   (LowestActiveCrate(group,crate) != 0)) {
                  NuMtxSetRotationY(&mCRATE,(uint)group->angle);
                  NuMtxRotateZ(&mCRATE,(uint)crate->surface_zrot);
                  NuMtxRotateX(&mCRATE,(uint)crate->surface_xrot);
                  mCRATE._30 = crate->pos.x;
                  mCRATE._31 = crate->shadow + 0.025f;
                  mCRATE._32 = crate->pos.z;
                  NuRndrGScnObj((ObjTab[21].obj.scene)->gobjs[(ObjTab[21].obj.special)->instance->objid],&mCRATE);
                }
          }
      }
  }
}
