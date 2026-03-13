#ifndef MAIN_H
#define MAIN_H

#include "../types.h"
#include "../nu.h"
#include "nu3dx.h"
#include "gamecode/creature.h"
#include "gamecode/game.h"
#include "gamecode/font3d.h"
#include "gamecode/camera.h"
#include "nuxbox/nuxboxtypes.h"

s32 SWIDTH;
s32 SHEIGHT;
struct nugscn_s* world_scene[32];
struct nuscene_s* world_scene2[32];
s32 iss3cmp;
s32 USELIGHTS;
struct numtx_s tmtx[256];
s32 CurrentCausticTexture;
struct MYDRAW;
struct ATLASSTRUCT;
struct plritem_s;
struct enemyjeep_s;
struct nutexanimprog_s;
struct deb3_s;
struct ZOFFASTRUCT;
struct ZOFFASTART;
struct ASTEROIDSTRUCT;
struct MYSPLINE;
struct instNUGCUTSCENE_s;
struct cammtx_s;
struct visidata_s;

void Text3D2 (char * txt, float x, float y, float z, float scalex, float scaley, float scalez, int align, int colour);
void Text3D (char * txt, float x, float y, float z, float scalex, float scaley, float scalez, int align, int colour);
void DrawPanel(void);
void DrawMenu(struct cursor_s *cursor, s32 paused);
void PlayCutMovie (int movie);
void ResetSuperBuffer (void);
void ResetSuperBuffer2 (void);
void GameSfx(s32 sfx, struct nuvec_s *pos);
void GameSfxLoop(s32 sfx, struct nuvec_s *pos);
void MyGameSfx(s32 Id, struct nuvec_s *Pos, s32 Vol);
void ResetGameSfx(void);
void UpdateGameSfx(void);
void GameMusic(s32 sfx, s32 i);
s32 ProcessTimer(float *Timer);
void AddSpacesIntoText(char* txt, u32 bits);
s32 RotDiff(u16 a0,u16 a1);
u16 SeekRot(u16 a0,u16 a1,s32 shift);
void UpdateCharacterIdle(struct creature_s* c, s32 character);
void NewRumble(struct rumble_s *rumble,s32 power);
void NewBuzz(struct rumble_s *rumble,s32 frames);
void ResetAnimPacket(struct anim_s *anim,s32 action);
s32 KillGameObject(struct obj_s *obj, s32 die);
s32 KillPlayer(struct obj_s *player_obj,s32 die);
s32 MyInitModelNew(struct MYDRAW *Draw, s32 Character, s32 Action, s32 NumJoints, struct NUJOINTANIM_s *JointList, struct nuvec_s *Position);
s32 MyDrawModelNew(struct MYDRAW *Draw, struct numtx_s *Mat, struct numtx_s *Locators);
void AddGameDebris(s32 i,struct nuvec_s *pos);
void AddGameDebrisRot(s32 i,struct nuvec_s *pos,s32 n,u16 xrot,u16 yrot);
void AddFiniteShotDebrisEffect(s32 *key,s32 type,struct nuvec_s *pos,s32 repeats);
void AddVariableShotDebrisEffect(s32 type,struct nuvec_s *pos,s32 numdeb,short emitrotz,short emitroty);
void NuDatSet(struct nudathdr_s *hdr);
void NuTexSetTextureStates(struct numtl_s *mtl);
void NuTexSetTexture(u32 stage,s32 tid);
s32 NudxFw_SetRenderState(enum _D3DRENDERSTATETYPE state, u32 data);
s32 NudxFw_SetTextureState(u32 stage, enum _D3DTEXTURESTAGESTATETYPE state, u32 data);
void CloseCutMovie(s32 all);
void NewGameCutAnim(void);
void SetCutSceneLights(void);
void ResetPlayerMoves(struct creature_s *c);
void RemoveCreature(struct creature_s *c);
void PlayerStartPos(struct creature_s *c,struct nuvec_s *pos);
void MovePlayer(struct creature_s *plr,struct nupad_s *pad);
void UpdateChaseRunAnim(struct creature_s *plr);
s32 DrawPanel3DTempCharacter(float x, float y, float z, float scale, u16 xrot, u16 yrot, u16 zrot, s32 rot);
s32 DrawPanel3DObject(s32 object,float x,float y,float z,float scalex,float scaley,float scalez, u16 xrot,u16 yrot,u16 zrot,struct nugscn_s *scn,struct nuspecial_s *obj,s32 rot);
s32 DrawPanel3DCharacter(s32 character,float x,float y,float z,float scalex,float scaley,float scalez, u16 xrot,u16 yrot,u16 zrot,s32 action,float anim_time,s32 rot);
void DrawPanelDebris(void);
void LoadWumpa(void);
void ResetWumpa(void);
void FlyWumpa(struct wumpa_s* wumpa);
s32 Draw3DCharacter(struct nuvec_s *pos,u16 xrot,u16 yrot,u16 zrot,struct CharacterModel *model, s32 action,float scale,float anim_time, s32 rot);
u16 TurnRot(u16 a0, u16 a1, s32 rate);
s32 AheadOfCheckpoint(s32 iRAIL,s32 iALONG,float fALONG);
void NewGame(void);
void ResetItems(void);
void ResetGemPath(void);
void ResetDeath(void);
void ResetBonus(void);
void OpenGame(void);
void ResetLevel(void);
void CleanLetters(char *txt);
void NextMenuEntry(float *y, s32 *i);
void GetMenuInfo(struct cursor_s* cur);
char* MakeEditText(char* txt);
void CalculateGamePercentage(struct game_s *game);
void ResetTimer(struct GTimer *t);
void UpdateTimer(struct GTimer* t);
void ResetTimeTrial(void);
void ClockOff(void);
s32 WumpaRayCast(struct nuvec_s* p0, struct nuvec_s* p1, float ratio0);
float GetZoffaBestTarget(float Best, struct nuvec_s **TargetPos, struct nuvec_s **Vel, s32 *Moving);
void ProcessAsteroids(void);
void DrawAsteroids(void);
s32 GetGliderHealthPercentage(struct creature_s *Cre);
s32 GetRumblePlayerHealthPercentage(struct creature_s *Cre);
float GetGunBoatBestTarget(float Best, struct nuvec_s **TargetPos, struct nuvec_s **Vel, s32 *Moving);
float GetBattleShipBestTarget(float Best, struct nuvec_s **TargetPos, struct nuvec_s **Vel, s32 *Moving);
void DrawGliderTarget(void);
void DrawFireFlyLevelExtra(void);
void ProcessFireFlyIntro(void);
void DrawWeatherResearchLevelExtra(void);
struct nuvec_s *FireWBBolt(struct nuvec_s *Pos, struct nuvec_s *Vel, int Type, float Life, int Owner);
void InitWBIntro(void);
void DrawWeatherBossLevelExtra(void);
void ProcessWBIntro(void);
void MoveAtlas(struct creature_s *Cre, struct nupad_s *Pad);
void DrawEarthBoss(void);
void CheckAtlasVortex(struct ATLASSTRUCT *Atlas);
void InitTrail(void);
void InitJeepRocks(void);
void DrawJeepRocks(void);
void DrawVehMasks(void);
void UpdateSaveSlots(struct cursor_s *cur);
void NewMenu(struct cursor_s *cur, s32 menu, s32 y, s32 level);
void UpdatePanelItem(struct plritem_s* item, int force_update, int use_change);
void MyGameSfxLoop(s32 Id, struct nuvec_s *Pos, s32 Vol);
void InitVehMasks(void);
void InitVehMask(s32 Indx, s32 Id);
struct ZOFFASTRUCT *InitZoffa(struct ZOFFASTRUCT *Zoffa, struct ZOFFASTART *Start);
void MoveZoffaUFO(struct ZOFFASTRUCT *Zoffa);
void ProcessAsteroid(struct ASTEROIDSTRUCT *Asteroid);
float FindSplineClosestPointAndDist(struct MYSPLINE *Spline, s32 Control, struct nuvec_s *Point, struct nuvec_s *TargetPoint, s32 Wrap, s32 BigLook);
void ResetLights(struct Nearest_Light_s *nl);
void GetLights(struct nuvec_s *pos, struct Nearest_Light_s *nearest_lights, s32 SearchMode);
void SetLights(struct nucolour3_s *vCOL0, struct nuvec_s *vDIR0, struct nucolour3_s *vCOL1, struct nuvec_s *vDIR1, struct nucolour3_s *vCOL2, struct nuvec_s *vDIR2, struct nuvec_s *vAMB);
void EvalModelAnim(struct CharacterModel *model, struct anim_s *anim, struct numtx_s *m, struct numtx_s *tmtx, float ***dwa, struct numtx_s *mLOCATOR);
float DotProduct(struct nuvec_s *A, struct nuvec_s *B);
void GetTopBot(struct creature_s *c);
void OldTopBot(struct obj_s *obj);
void NewTopBot(struct obj_s *obj);
void ObjectRotation(struct obj_s *obj, s32 mode, s32 set);
void ResetJeep(struct creature_s *Cre);
void MoveLoopXZ(struct obj_s *obj, u16 *ay);
void FlyGameObject(struct obj_s *obj, u16 yrot);
void RemoveGameObject(struct obj_s *obj);
void AddProjectile(struct nuvec_s *src, struct nuvec_s *dst, struct nuvec_s *mom, s32 type, u16 yrot, struct obj_s *obj);
void PickupBonusGem(unsigned int item);
void PlayerCreatureCollisions(struct obj_s *obj);
s32 WumpaCollisions(struct obj_s *obj);
float CrateTopBelow(struct nuvec_s *pos);
s32 HitCrates(struct obj_s *obj, s32 destroy);
s32 HitItems(struct obj_s *obj);
s32 GetDieAnim(struct obj_s *obj, s32 die);
void FindAnglesZX(struct nuvec_s *src);
s32 NuSpecialFind(struct nugscn_s *scene, struct nuhspecial_s *special, char *name);
struct nuanimdata_s *InstAnimDataLoad(char *name);
s32 LoadCutMovie(s32 movie);
void instNuGCutSceneSetEndCallback(struct instNUGCUTSCENE_s *icutscene, void(*fn)(void*));
void ResetGameCameras(struct cammtx_s *Gamecam, s32 n);
void ComplexRailPosition(struct nuvec_s *pos, s32 iRAIL, s32 iALONG, struct RPos_s *rpos, s32 set);
void JeepCamFollowAng(struct cammtx_s *cam, s32 blend);
float FindNearestCreature(struct nuvec_s *pos, s32 character, struct nuvec_s *dst);
void MakeLevelTimeString(struct time_s *time, char *txt);
void StartTimeTrial(struct nuvec_s *pos, s32 off);
s32 HubFromLevel(s32 level);
void NuGScnRndr3(struct nugscn_s *scn);
void NuBridgeDraw(struct nugscn_s *scn, struct numtl_s *mtl);
void NuWindDraw(struct nugscn_s *scn);
void edobjRenderObjects(struct nugscn_s *scn);
s32 edppLoadEffects(char* file, char list);
s32 edppMergeEffects(char* file, char list);
void DebrisSetRenderGroup(s32 group);
void MoveVehicle(struct creature_s *plr, struct nupad_s *pad);
void MoveCOCO(struct creature_s *plr, struct nupad_s *pad);
void MoveCRASH(struct creature_s *plr, struct nupad_s *pad);
void MoveFIREENGINE(struct creature_s *plr, struct nupad_s *pad);
void MoveGYRO(struct creature_s *plr, struct nupad_s *pad);
void MoveMECH(struct creature_s *plr, struct nupad_s *pad);
void MoveMINECART(struct creature_s *plr, struct nupad_s *pad);
void MoveMINETUB(struct creature_s *plr, struct nupad_s *pad);
void MoveOFFROADER(struct creature_s *plr, struct nupad_s *pad);
void MoveSCOOTER(struct creature_s *plr, struct nupad_s *pad);
void MoveSNOWBOARD(struct creature_s *plr, struct nupad_s *pad);
void MoveSUBMARINE(struct creature_s *plr, struct nupad_s *pad);
void MoveSWIMMING(struct creature_s *plr, struct nupad_s *pad);
void AnimateATLASPHERE(struct creature_s *plr);
void AnimateCOCO(struct creature_s *plr);
void AnimateCRASH(struct creature_s *plr);
void AnimateDROPSHIP(struct creature_s *plr);
void AnimateFIREENGINE(struct creature_s *plr);
void AnimateGLIDER(struct creature_s *plr);
void AnimateGYRO(struct creature_s *plr, struct nupad_s *pad);
void AnimateJEEP(struct creature_s *plr);
void AnimateMECH(struct creature_s *plr);
void AnimateMINECART(struct creature_s *plr);
void AnimateMINETUB(struct creature_s *plr);
void AnimateMOSQUITO(struct creature_s *plr);
void AnimateOFFROADER(struct creature_s *plr);
void AnimateSCOOTER(struct creature_s *plr);
void AnimateSNOWBOARD(struct creature_s *plr);
void AnimateSUBMARINE(struct creature_s *plr);
void AnimateSWIMMING(struct creature_s *plr);
void BonusTiming(struct creature_s *plr);
void BonusTransporter(struct creature_s *plr);
void DeathTransporter(struct creature_s *plr);
void GemPathTransporter(struct creature_s *plr);
void CheckPlayerEvents(struct obj_s *obj);
void CheckFinish(struct obj_s *obj);
void CheckGates(struct obj_s *obj);
void CheckRings(struct obj_s *obj, s32 *ring);
void CrateCollisions(struct obj_s *obj);
void Draw3DCrateCount(struct nuvec_s *pos, u16 yrot);
void DrawAtlas(struct creature_s *c);
void DrawGlider(struct creature_s *c);
struct numtx_s *DrawPlayerJeep(struct creature_s *c);
void DrawProbeFX(struct obj_s *obj);
void HubLevelSelect(struct obj_s *obj, s32 hub);
void HubSelect(struct creature_s *c);
void HubStart(struct obj_s *obj, s32 hub, s32 level, struct nuvec_s *pos);
s32 InLoadSaveZone(struct creature_s *plr);
void ToggleVehicle(struct creature_s *plr);
void ResetAtlas(struct creature_s *c);
void SetWeatherStartPos(struct creature_s *c);
s32 AddAward(s32 hub, s32 level, u16 got);
s32 AddGameObject(struct obj_s *obj, void *data);
void AddMaskFeathers(struct mask_s *mask);
void UpdateMask(struct mask_s *mask, struct obj_s *obj);
struct nuvec_s *NuHGobjGetPOI(struct NUHGOBJ_s *hgobj, u8 poi_id);
void BuildVisiTable(struct nugscn_s *gsc);
struct visidata_s* visiLoadData(char* fname, struct nugscn_s* sc, union variptr_u* buff);
struct numtl_s *CreateAlphaBlendTexture64(char *fname, s32 uvmode, s32 alpha, s32 pri);
void InitClouds(union variptr_u *buffer, union variptr_u *buffend);
void InitFont3D(struct nugscn_s *gscn);
void InitLocalSfx(struct pSFX *sfx, s32 count);
void PauseGameAudio(s32 pause);
void InitXboxEffectSystem(s32 level);
void InitXboxGSceneEffects(struct nugscn_s *gsc, union variptr_u *buffer, union variptr_u *buffend);
void NuBridgeRegisterBaseScene(struct nugscn_s *scn);
void NuBridgeUpdate(struct nuvec_s *playerpos);
void NuLigthSetPolyHazeMat(struct numtl_s *mtl, float *arg1, float *arg2);
void NuRndrRect2di(s32 x, s32 y, s32 w, s32 h, s32 color, struct numtl_s *mtl);
void NuVpSetSize(float w, float h);
struct nuscene_s *NuSceneReader2(union variptr_u *buffer, union variptr_u *buffend, char *file);
void NuWindUpdate(struct nuvec_s *pos);
s32 edanimFileLoad(char *file);
void edanimRegisterBaseScene(struct nugscn_s *scn);
void edbitsRegisterBaseScene(struct nugscn_s *s);
void edbitsRegisterSfx(struct pSFX *sfxGlobalTab, struct pSFX *sfxLevelTab, u32 sfxGlobalCount, u32 sfxAllCount);
s32 edgraFileLoad(char *file);
s32 edobjFileLoadObjects(char *file);
void edobjRegisterBaseScene(struct nugscn_s *scn);
void tbslotBegin(s32 tbh, s32 slotix);
void tbslotEnd(s32 tbh, s32 slotix);
void SetCreatureLights(struct creature_s *c);
void MyChangeAnim(struct MYDRAW *Draw, s32 Action);
void NuPs2PadSetMotors(struct nupad_s *pad, s32 motor1, s32 motor2);
void Reseter(s32 mode);
struct nutexanimprog_s *NuTexAnimProgReadScript(union variptr_u *buff, char *fname);
void InitEnemyJeep(struct enemyjeep_s *Jeep, s32 id);
s32 NuTexCreate(struct nutex_s *nutex);
void NuTexDestroy(s32 id);
s32 NuSpecialDrawAt(struct nuhspecial_s *sph, struct numtx_s *mtx);
s32 JStrLen(char *txt);
s32 CombinationCharacterBD(char c0, char c1);
s32 CombinationCharacterBC(char c0, char c1);
void DefaultTimeTrialNames(s32 all);
void NewLanguage(s32 l);
void ChangeCharacter(struct creature_s *c, s32 character);
float RatioAlongLine(struct nuvec_s* pos, struct nuvec_s* p0, struct nuvec_s* p1);
struct deb3_s *AddDeb3(struct nuvec_s *pos, s32 db, s32 emit, struct nuangvec_s *angle);
void ProcessAtlasAtlasCollisions_a(struct ATLASSTRUCT *a, struct ATLASSTRUCT *b);
struct nulsthdr_s *NuLstCreate(s32 elcnt, s32 elsize);
void NuLstDestroy(struct nulsthdr_s *hdr);
struct nulnkhdr_s *NuLstAlloc(struct nulsthdr_s *hdr);
void NuLstFree(struct nulnkhdr_s *lnk);
struct nulnkhdr_s *NuLstGetNext(struct nulsthdr_s *hdr, struct nulnkhdr_s *lnk);

// Size: 0x10
struct txanmlist
{
    char* path; // Offset: 0x0, DWARF: 0x40B859
    int pad1; // Offset: 0x4, DWARF: 0x40B883
    unsigned long long levbits; // Offset: 0x8, DWARF: 0x40B8AA
};

typedef struct pCHASE pCHASE, *PpCHASE;

typedef struct pSFX pSFX, *PpSFX;

struct pSFX {
    char name[16];
    u16 pitch;
    u16 volume;
    u8 buzz;
    u8 rumble;
    u8 delay;
    u8 wait;
    char * path;
    u16 frequency;
    u16 stream;
    char type;
    char pad1;
    u16 id;
    struct nuvec_s Pos;
};

struct pCHASE {
    char i;
    char i_last;
    char i_next;
    u8 cuboid;
    short character[6];
    short action[6];
    float scale[6];
    float duration;
};



typedef struct ghg_inst_s ghg_inst_s, *Pghg_inst_s;

struct ghg_inst_s {
    char * name;
    struct NUHGOBJ_s * scene;
};

typedef enum Actions_ {
    ACT_ATTACK=0,
    ACT_ATTACK2=1,
    ACT_CRAWL=2,
    ACT_CROUCHDOWN=3,
    ACT_CROUCHIDLE=4,
    ACT_CROUCHUP=5,
    ACT_DIE=6,
    ACT_DIE2=7,
    ACT_DOWN=8,
    ACT_DROWN=9,
    ACT_EXPLODE=10,
    ACT_FALL=11,
    ACT_FILL=12,
    ACT_FLY=13,
    ACT_GETUP=14,
    ACT_HANG=15,
    ACT_HOP=16,
    ACT_IDLE=17,
    ACT_IDLE00=18,
    ACT_IDLE01=19,
    ACT_IDLE02=20,
    ACT_IDLE03=21,
    ACT_IDLE04=22,
    ACT_IDLE05=23,
    ACT_IDLE06=24,
    ACT_IDLE07=25,
    ACT_IN=26,
    ACT_JUMP=27,
    ACT_JUMP2=28,
    ACT_LAND=29,
    ACT_LAND2=30,
    ACT_LEFT=31,
    ACT_LOSE=32,
    ACT_OPEN=33,
    ACT_OUT=34,
    ACT_PULL=35,
    ACT_PUSH=36,
    ACT_RIGHT=37,
    ACT_RUN=38,
    ACT_RUN2=39,
    ACT_RUNNINGJUMP=40,
    ACT_SHAKE=41,
    ACT_SIT=42,
    ACT_SKATE=43,
    ACT_SKID=44,
    ACT_SLAM=45,
    ACT_SLEEP=46,
    ACT_SLIDE=47,
    ACT_SOMERSAULT=48,
    ACT_SPENT=49,
    ACT_SPIN=50,
    ACT_SPIN2=51,
    ACT_SPRINT=52,
    ACT_STARJUMP=53,
    ACT_SUPERSLAM=54,
    ACT_SWIM=55,
    ACT_SWING=56,
    ACT_TIPTOE=57,
    ACT_TRAPPED=58,
    ACT_TRAPPEDSHAKE=59,
    ACT_TURN=60,
    ACT_UP=61,
    ACT_WADE=62,
    ACT_WAIT=63,
    ACT_WALK=64,
    ACT_WIN=65,
    ACT_FLYZOFFA=66,
    MAXACTIONS=67
} Actions_;

typedef struct cdata_s cdata_s, *Pcdata_s;

struct cdata_s { /* CharacterData */
    char * path;
    char * file;
    char * name;
    struct animlist * anim;
    float radius;
    struct nuvec_s min;
    struct nuvec_s max;
    float scale;
    float shadow_scale;
};


struct mask {
    struct numtx_s mM;
    struct numtx_s mS;
    struct nuvec_s pos;
    struct nuvec_s newpos;
    struct Nearest_Light_s lights;
    struct anim_s anim;
    float scale;
    float shadow;
    short character;
    short active;
    short sfx;
    char pad1;
    char pad2;
    u16 xrot;
    u16 yrot;
    u16 angle;
    u16 surface_xrot;
    u16 surface_zrot;
    u16 wibble_ang[3];
    float idle_time;
    float idle_duration;
    char idle_mode;
    char reflect;
    u8 offset_frame;
    char hold;
};



typedef struct csc_s csc_s, *Pcsc_s;

struct csc_s {
    struct NUHGOBJ_s * obj;
    char * path;
    char * name;
};

typedef struct PADStatus PADStatus, *PPADStatus;

struct PADStatus {
    u32 button;
    short stickX;
    short stickY;
    short substickX;
    short substickY;
    u32 triggerLeft;
    u32 triggerRight;
    u32 analogA;
    u32 analogB;
    short err;
};

typedef struct PData *PPData;

struct PData {
    u8 character;
    u8 font3d_letter;
    char pad1;
    char pad2;
    char * name;
    int * description;
};

typedef struct PlrEvent PlrEvent, *PPlrEvent;

struct PlrEvent {
    struct nugspline_s * spl;
    struct nuhspecial_s obj[16];
    struct nuvec_s pos;
    char played;
    char iRAIL;
    short iALONG;
    float fALONG;
};

typedef struct pVTog VTog_s, *PVTog_s;

struct pVTog {
    struct nugspline_s * pTRIGGER;
    struct nugspline_s * pCAM;
    struct nugspline_s * pLOOK;
    struct numtx_s m;
    struct nuvec_s pos;
    struct anim_s anim;
    char type;
    char iRAIL;
    short iALONG;
    float fALONG;
    u16 xrot;
    u16 yrot;
    u16 zrot;
    char pad1;
    char pad2;
};
// Size: 0x10, DWARF: 0x1464F0
struct nulnkhdr_s
{
    struct nulsthdr_s* owner; // Offset: 0x0, DWARF: 0x14650F
    struct nulnkhdr_s* succ; // Offset: 0x4, DWARF: 0x14653C
    struct nulnkhdr_s* prev; // Offset: 0x8, DWARF: 0x146568
    unsigned short id; // Offset: 0xC, DWARF: 0x146594
    unsigned short used : 1; // Offset: 0xE, DWARF: 0x1465B9, Bit Offset: 0, Bit Size: 1
};

// Size: 0x10, DWARF: 0x1463F8
struct nulsthdr_s
{
    struct nulnkhdr_s* free; // Offset: 0x0, DWARF: 0x146417
    struct nulnkhdr_s* head; // Offset: 0x4, DWARF: 0x146443
    struct nulnkhdr_s* tail; // Offset: 0x8, DWARF: 0x14646F
    short elcnt; // Offset: 0xC, DWARF: 0x14649B
    short elsize; // Offset: 0xE, DWARF: 0x1464C3
};



#endif // !MAIN_H
