#include "main.h"
#include <stddef.h>
#include <string.h>

struct nuquat_s { float x, y, z, w; };

/* ---- function declarations ---- */
struct nuvec_s SetNuVec(f32 x, f32 y, f32 z);
struct nuvec_s *SetNuVecPntr(f32 x, f32 y, f32 z);
struct Quat SetNuQuat(f32 x, f32 y, f32 z, f32 w);
s32 AddGliderBullet(struct numtx_s *Mat, struct nuvec_s *Pos,
                    struct nuvec_s *Vel, s32 Enemy);
static s32 AddGliderBomb(struct nuvec_s *Pos, struct nuvec_s *Vel, float AngY,
                         s32 Enemy, struct nuvec_s *TargetPoint,
                         struct nuvec_s *TargetVel, s32 Moving);
float Rationalise360f(float a);
float frand(void);
float fsign(float x);
void SeekHalfLife(float *dest, float target, float halflife, float dt);
void SeekAngHalfLife360f(float *dest, float target, float halflife, float dt);
float GetBigGunBestTarget(float Best, struct nuvec_s **TargetPos,
                          struct nuvec_s **TargetVel, s32 *Moving);
float GetBattleShipBestTarget(float Best, struct nuvec_s **TargetPos,
                              struct nuvec_s **TargetVel, s32 *Moving);
float GetGunBoatBestTarget(float Best, struct nuvec_s **TargetPos,
                           struct nuvec_s **TargetVel, s32 *Moving);
float GetZoffaBestTarget(float Best, struct nuvec_s **TargetPos,
                         struct nuvec_s **TargetVel, s32 *Moving);
static s32 CollideGliderBombs(struct nuvec_s *, struct nuvec_s *, s32, float);
void DebFree(s32 *key);
void ProcessWesternArenaLevel(struct nupad_s *Pad);
void ProcessEarthBossLevel(struct nupad_s *Pad);
void ProcessFireBossLevel(struct nupad_s *Pad);
void WesternArenaReset(s32 PlayerDead);
void FireBossReset(s32 PlayerDead);
void MyResetAnimPacket(struct MYDRAW *Draw, s32 Action);
void UnleashLighteningHail(struct nuvec_s *Pos, s32 AttNum);
s32 NuHGobjRndr(void *hgobj, struct numtx_s *wm, s32 nlayers, short *layers);
void edppRegisterPointerToGameCharLocation(struct nuvec_s *charloc);
s32 NewRayCastMask(struct nuvec_s *Pos, struct nuvec_s *Dir, s32 mask);
void AddExtraLife(struct nuvec_s *Pos, s32 type);
s32 TerrainPlatId(void);
void TerrainPlatGetMtx(s32 platid, struct numtx_s **old, struct numtx_s **cur);
s32 NewRayCastPlatForm(struct nuvec_s *vpos, struct nuvec_s *vvel, float size, float timeadj, s32 platformid);
s32 NewRayCastSetHandel(struct nuvec_s *Pos, struct nuvec_s *Dir, float size, float a, float b, short *handle);
void ProcessVehMasks(void);
static const struct nuvec_s kHalfScale = {0.5f, 0.5f, 0.5f};

/* ---- struct definitions ---- */

struct NEWBUGGY {
    void *a;
};

typedef struct CrateCube_s {
    void *model;
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

struct MYDRAW {
    struct anim_s Anim;
    struct CharacterModel *Model;
    char _pad1[0xc0];
};

struct MYSPLINE {
    struct nugspline_s *Spline;
    float Cur;
    float Nex;
    float Act;
    float Inc;
    struct nuvec_s CurPos;
    struct nuvec_s NexPos;
    float LookaheadDist;
};

struct objtab_s {
    struct nuhspecial_s obj;
    struct nugscn_s **scene;
    char visible;
    char font3d_letter;
    char pad1;
    char pad2;
    char *name;
    char unk[4];
    u64 levbits;
};

struct gdeb_s {
    s32 i;
};

struct plritem_s {
    s32 draw;
    s32 count;
    s32 frame;
};

typedef struct GLIDERSTRUCT GLIDERSTRUCT;
struct GLIDERSTRUCT {
    struct creature_s *Cre;
    struct nuvec_s vel;
    int Dead;
    int CocoDead;
    float CocoDeadTimer;
    float CocoDeathSpinX;
    float CocoDeathSpinZ;
    float NextEngRum;
    float FixVelTimer;
    float ImmuneAsteroidsTimer;
    struct nuvec_s Position;
    struct nuvec_s OldPosition;
    struct nuvec_s Velocity;
    struct nuvec_s Resolved;
    struct nuvec_s RailPoint;
    float RailAngle;
    float TiltX;
    float TiltZ;
    float DestTiltX;
    float DestTiltZ;
    float AngleY;
    float CamAngleY;
    float CamTiltX;
    float CamTornRecoverTimer;
    float InputX;
    float InputZ;
    int BarrelRoll;
    float BarrelDelta;
    float BarrelSpeedX;
    int TerminalDive;
    int TerminalDir;
    float FireTimer;
    float HitTimer;
    int AutoPilot;
    int ForceTurn;
    int HitPoints;
    char LocatorList[16];
    float LocatorTime[16];
    int InTornado;
    int LastInTornado;
    float InTornadoTime;
    float TornadoSpin;
    float InTornadoScale;
    struct nuvec_s ApparentPosition;
    struct nuvec_s ApparentVelocity;
    struct nuvec_s PositionStack[30];
    struct nuvec_s VelocityStack[30];
    int StackIndx;
    float Speed;
    float TargetSpeed;
    float NotInFrontTimer;
    float PullUpTimer;
    float OverideTiltZ;
    float TargetTimer;
    struct nuvec_s *MovingTargetPoint;
    struct nuvec_s *MovingTargetVel;
    int TargetOn;
    float TargetedTime;
    int TargetMoving;
    float NextHitSoundTimer;
};

struct ATLASSTRUCT {
    struct creature_s *Cre;
    int Whacko;
    int Type;
    int Dead;
    int HitPoints;
    int DestHitPoints;
    int HitPointCounter;
    int DrawCrunch;
    int DrawShell;
    float InhibitControlTimer;
    int NumAttacks;
    int Action;
    int LastAction;
    float ActionTimer;
    float ActionTimer2;
    int BeenHit;
    int CantBeHit;
    float RollSpeed;
    float RollAccTimer;
    struct MYDRAW Shell;
    struct MYDRAW Crunch;
    struct nuvec_s Position;
    struct nuvec_s OldPosition;
    struct nuvec_s LastPosition;
    struct nuvec_s TargetPosition;
    struct nuvec_s Velocity;
    struct nuvec_s OldVelocity;
    struct nuvec_s Resolved;
    struct nuvec_s Force;
    float Radius;
    float AngleX;
    float AngleY;
    float AngleZ;
    float InputAng;
    float InputMag;
    struct Quat Quat;
    struct Quat ThisQuat;
    struct Quat FrameQuat[4];
    struct Quat LastQuat;
    int LastHit;
    int Axis;
    struct nuvec_s LastNormal;
    struct nuvec_s StoreLastNormal;
    float DebugAxisTurn;
    float DebugAngY;
    float D[7];
    int OnGround;
    int PlatformId;
    struct nuvec_s PlatformNormal;
    float ShadowY;
    int SurfaceType;
    int TrailSurfaceType;
    int BigDrop;
    int Embedded;
    int Rock;
    int RockNum;
    float CrunchY;
    float DestCrunchY;
    short *TerrHandle;
    float BoostTimer;
    struct nuvec_s InterestPoint;
};

typedef struct ZOFFASTRUCT ZOFFASTRUCT;
struct ZOFFASTRUCT {
    struct MYDRAW MainDraw;
    int ActiveMode;
    float RespawnTimer;
    struct nuvec_s Position;
    struct nuvec_s Velocity;
    struct nuvec_s Resolved;
    float TiltX;
    float TiltZ;
    float DestTiltX;
    float DestTiltZ;
    float DestAngleY;
    float AngleY;
    int NoFireSound;
    float Temp[6];
    float VisibleTimer;
    float NewDirectionTimer;
    float NewAltitudeTimer;
    float NewAltitudeTarget;
    int TerminalDive;
    int Explode;
    int HitPoints;
    int FireFrame;
    int FireNow;
    float FireTimer;
    int Seen;
    int SmkTimer;
    struct numtx_s Locators[16];
    int SmokeCounter;
    float AggressionTimer;
    int PlayerCloseAndVisable;
    float NotSeenTimer;
    float AggressionPoints;
    int Teleport;
    float NoTeleportTimer;
    float Speed;
    int InFront;
    int FacingTarget;
    float Dist;
    int FireSide;
    float NotInFrontTimer;
    float LockedOnTimer;
    float KeepSameVelocityTimer;
    float FireBurstTimer;
};

typedef struct HOVASTRUCT HOVASTRUCT;
struct HOVASTRUCT {
    int ActiveMode;
    struct MYDRAW MainDraw;
    struct nuvec_s Position;
    struct nuvec_s StartPos;
    struct nuvec_s Velocity;
    float AngleY;
    int Thrust;
    float ThrustOff;
    int TerminalDive;
    int Explode;
    int HitPoints;
    float StayStillTimer;
    float MoveTimer;
    struct nuvec_s TargetVelocity;
};

typedef struct TORNADOSTRUCT TORNADOSTRUCT;
struct TORNADOSTRUCT {
    int Active;
    struct nuvec_s Position;
    struct nuvec_s StartPosition;
    struct MYDRAW MainDraw;
    float Scale;
    float YAng;
    float YAngInc;
    float Rad;
    float RadInc;
};

typedef struct SATELLITESTRUCT SATELLITESTRUCT;
typedef struct SPACESTATIONSTRUCT SPACESTATIONSTRUCT;

struct SATELLITESTRUCT {
    int Active;
    struct MYDRAW MainDraw;
    struct nuvec_s Position;
    struct nuvec_s Velocity;
    float AngleY;
    float TiltX;
    float TiltZ;
    float DestTiltX;
    float DestTiltZ;
    int Seen;
    int HitPoints;
    int Explode;
    SPACESTATIONSTRUCT *SpaceStation;
    float SatelliteAngleY;
};

struct SPACESTATIONSTRUCT {
    struct numtx_s Matrix;
    int Active;
    struct nuvec_s Position;
    struct nuvec_s Velocity;
    float AngleY;
    int Seen;
    int HitPoints;
    int Explode;
    float SatelliteAngleY;
    SATELLITESTRUCT *Satellite[3];
    struct numtx_s ShieldMtx;
    int ShieldDraw;
    int ShieldGreen;
    struct nuvec_s AppCentre;
};

typedef struct GLIDERBULLET GLIDERBULLET;
struct GLIDERBULLET {
    struct numtx_s Mat;
    struct nuvec_s Vel;
    short Life;
    short FirstLife;
};

typedef struct PLANESTRUCT PLANESTRUCT;
struct PLANESTRUCT {
    struct nuvec_s Pos;
    struct nuvec_s Vel;
    float ActionTimer;
    int ActionStatus;
    int Active;
    struct MYDRAW MainDraw;
};

typedef struct BATTLESHIPSTRUCT BATTLESHIPSTRUCT;
struct BATTLESHIPSTRUCT {
    int Active;
    struct MYDRAW MainDraw;
    struct numtx_s Locator[16];
    struct nuvec_s Position;
    float AngleY;
    float TiltX;
    float TiltZ;
    float DestTiltX;
    float DestTiltZ;
    float DestY;
    float Seek;
    int Seen;
    int HitPoints;
    float FireTimer[2];
    struct nuvec_s GooScale;
    struct nuvec_s DestGooScale;
    struct nuvec_s BaseGooScale;
    float GooSpeed;
    struct nuvec_s GooTimer;
    int KillMeNow;
};

typedef struct BIGGUNSTRUCT BIGGUNSTRUCT;
struct BIGGUNSTRUCT {
    int Active;
    struct MYDRAW MainDraw;
    struct nuvec_s Position;
    int Type;
    int HitPoints;
    float AngleY;
    float FireAngleX;
    float FireAngleY;
    float TiltX;
    float TiltZ;
    float DestAngleY;
    float DestTiltX;
    float DestTiltZ;
    int FireFrame;
    int Seen;
    int Action;
    int LastAction;
    float FireAngMomX;
    float FireAngMomY;
    float BurstTimer;
    int ExplosionEffect;
    float FireAngMinX;
    float FireAngMaxX;
    float FireAngMainY;
    float FireAngDeviationY;
    struct nuvec_s TerrPos;
};

typedef struct GUNBOATSTRUCT GUNBOATSTRUCT;
struct GUNBOATSTRUCT {
    int Active;
    int Character;
    struct MYDRAW MainDraw;
    struct nuvec_s Position;
    float AngleY;
    float FireAngleX;
    float FireAngleY;
    float TiltX;
    float TiltZ;
    float DestAngleY;
    float DestTiltX;
    float DestTiltZ;
    int FireFrame;
    int Seen;
    int Action;
    int LastAction;
    float FireAngMomX;
    float FireAngMomY;
    float BurstTimer;
    float SunkTimer;
};

typedef struct BOSSSTRUCT BOSSSTRUCT;
struct BOSSSTRUCT {
    int Active;
    int Unleashed;
    float FireTimer;
    int FireSide;
    struct MYDRAW MainDraw;
    struct MYDRAW BonesDraw;
    struct numtx_s Locator[16];
    struct nuvec_s Position;
    float PossYDest;
    float AngleY;
    float BaseAngleY;
    float DestAngleY;
    int HitPoints[4];
    int Seen;
    float Distance;
    float DistanceDest;
    int Action;
    int OldAction;
    int NextAction;
    float ActionTimer;
    int LastNonSeekAction;
    int Dead;
    struct MYSPLINE MainSpline;
    float HoldFrameTimer;
    float ChestSoundBTimer;
    int HitSoundFrame;
};

typedef struct ASTEROIDSTRUCT ASTEROIDSTRUCT;
struct ASTEROIDSTRUCT {
    int Active;
    struct nuvec_s Position;
    struct nuvec_s Velocity;
    float AngleX;
    float AngleY;
    float AngVelX;
    float AngVelY;
    struct nuvec_s Scale;
    int Seen;
    int HitPoints;
    float CentreDist2;
    int Stealth;
    int Explode;
};

typedef struct ZOFFASTART ZOFFASTART;
struct ZOFFASTART {
    float x;
    float y;
    float z;
    float Angle;
};

typedef struct LIGHTENINGHAIL LIGHTENINGHAIL;
struct LIGHTENINGHAIL {
    int Mode;
    float Timer;
    float FallX;
    struct nuvec_s Position;
    struct nuvec_s Velocity;
};

typedef struct WBBOLT WBBOLT;
struct WBBOLT {
    int Mode;
    int Owner;
    struct nuvec_s Position;
    struct nuvec_s Velocity;
    float Life;
    int Type;
    float SeekSpeed;
    float Scale;
};

typedef struct GLIDERBOMBSTRUCT GLIDERBOMBSTRUCT;
struct GLIDERBOMBSTRUCT {
    struct nuvec_s Vel;
    struct nuvec_s Pos;
    float AngY;
    int Life;
    float Gravity;
    struct nuvec_s Target;
    struct nuvec_s *TargetPntr;
    struct nuvec_s *TargetVelPntr;
    int TargetMoving;
    int UnderWater;
    float DropTimer;
};

struct JEEPROCK {
    struct nuvec_s Pos;
    struct nuvec_s Vel;
    int Active;
    int Seen;
    int Stuck;
    int Explode;
    int SmallDamage;
    float Life;
    struct ATLASSTRUCT Atlas;
    int Grabbed;
    int Mode;
    float FlameTimer;
    struct nuvec_s Scale;
    int FireBlob;
    int SmashMe;
};

struct VEHMASK {
    int Active;
    int Id;
    int KillAtEnd;
    struct nuvec_s Position;
    struct MYDRAW MainDraw;
    float DrawAngY;
    int Action;
    int LastAction;
    int EffectType;
    float Tween;
    float TweenInc;
    float Ang;
    struct nuvec_s Store[2];
    struct nuvec_s *Point[2];
    struct nuvec_s Offset[2];
    float AngInc[2];
    float Rad[2];
    float Scale[2];
    float TiltX[2];
    float DrawScale;
    int Seen;
};

void ProcessJeepRock(struct JEEPROCK *Rock);
void ProcessBattleShip(struct BATTLESHIPSTRUCT *BattleShip);
void ProcessBigGun(struct BIGGUNSTRUCT *BigGun);
void ProcessGunBoat(struct GUNBOATSTRUCT *GunBoat);
void ProcessSpaceStation(struct SPACESTATIONSTRUCT *SpaceStation);
void ProcessAtlas(struct ATLASSTRUCT *Atlas);
void ProcessAtlasTrail(struct ATLASSTRUCT *Atlas);
void ProcessHovaBlimp(struct HOVASTRUCT *Hova);
void ProcessEarthBossActions(struct ATLASSTRUCT *EarthBoss);

/* ---- vehicle.c globals ---- */

GLIDERBOMBSTRUCT GliderBombs[10];
GLIDERBOMBSTRUCT EnemyGliderBombs[0x1c];
struct ATLASSTRUCT EarthBoss;
struct ATLASSTRUCT PlayerAtlas;
SATELLITESTRUCT SatelliteList[9];
SPACESTATIONSTRUCT SpaceStationList[3];
s32 SatelliteCharacterId;
BATTLESHIPSTRUCT BattleShipList[6];
BIGGUNSTRUCT BigGunList[12];
s32 BigGunIndx;
GUNBOATSTRUCT GunBoatList[4];
BOSSSTRUCT WeatherBoss;
ASTEROIDSTRUCT AsteroidList[100];
s32 AsteroidDebug;
WBBOLT BoltList[0x78];
LIGHTENINGHAIL HailList[0x1e];
struct JEEPROCK JeepRock[6];
PLANESTRUCT PlayerPlane;
TORNADOSTRUCT TornadoList[6];
struct numtx_s WeatherBossCamMtx;
struct MYSPLINE WeatherBossCamSpline;
struct MYSPLINE WeatherBossIntroSpline;
struct numtx_s BoltMtxA;
struct numtx_s BoltMtxB;
struct numtx_s BoltMtxC;
struct nuvec_s WeatherBossTargetVel;
s32 JeepInControl;
struct nuvec_s GliderTargetPos;
struct nuvec_s GliderCollPoints[6];
struct nuvec_s TeleportPos[4];
struct nuvec_s TeleportVel[4];
ZOFFASTART ZoffaStartPoints[16];
s32 ZoffaTeleportIndx;
s32 ZoffaCollisionCounter;
s32 CurrentAggressor;
s32 JonnyOn;
s32 ebgo;
struct nuvec_s ebpos[3];
float ebtime[3];
s32 jontorn[20];
s32 NumRockPanel;
s32 RockPanelObj[6];
struct nuvec_s RockPanelData[6];
float RockPanelScale;
s32 RockPanelRotX[6];
s32 RockPanelRotY[6];
s32 RockPanelRotZ[6];
float RumbleDisplayY;
s32 RumbleDisplayMode;
float RumbleCamTween;
float RumbleCamTweenDest;
float RumbleCamTweenInterest;
float RumbleCamVal;
struct nuvec_s RumbleCamPos;
s32 RumbleCamY;
s32 ResetAtlasCamera;
float AngleY_836;
float AtlasWhackValue;
float AtlasWhackTimer;
s32 AtlasFrame;
s32 AtlasEmbeddedFrame;
s32 kj;
s32 FarmFrame;
s32 FarmResetTimer;
float GliderCamHighTimer;
s32 LastNumZoffasFiring;
s32 NumZoffasFiring;
s32 ActiveAsteroidListNum;
s32 ACTIVEBLIMPCOUNT;
float FlyingLevelCompleteTimer;
s32 FlyingLevelExtro;
s32 FlyingLevelVictoryDance;
float FlyingLevelVictoryDanceTimer;
s32 FireFlyIntroAction;
s32 FireFlyIntroOldAction;
s32 FireFlyIntroOn;
float FireFlyStartAngle;
struct nuvec_s GliderIntroCamPos;
struct nuvec_s GliderIntroInterest;
float Timer_549;
float CamAngY_550;
float MatchTimer;
float MatchMaxDist;
float MatchMinDist;
float PrePanTime;
float PanSeekSpeed;
float MinPanSeekSpeed;
s32 VehicleLevelImmune;
s32 ChrisBigBossDead;
s32 WeatherBossDead;
s32 EarthBossDeathTimer;
s32 EarthBossJustEntered;
s32 EarthBossDeathEffect;
s32 EarthBossVortexOpen;
s32 ShootRockSound;
float WeatherBossSkeletonTimer;
float WeatherBossSkeletonGlitchTimer;
float AtmosphericPressureHackedZ;
struct nuvec_s WBIntroGliderPos;
float WBIntroDestTiltX;
float WBIntroDestTiltZ;
float WBIntroTweenTimer;
struct nuvec_s WeatherStartPos;
s32 LevelResetTimer;
s32 GliderFrameCounter;
s32 BazookaIconOn;
struct nuvec_s BazookaTokenPos;
struct nuvec_s BazookaTokenCurrentPos;
struct MYDRAW IconMainDraw;
struct nuvec_s ElectricalPosition;
s32 RumbleStoreTotalRoks;
s32 RumbleStoreCrunchRoks;
s32 RumbleStoreCrashRoks;
s32 NumRockPanel;

/* ---- extern declarations ---- */

extern struct nuvec_s v000;
extern struct numtx_s mTEMP;
extern struct objtab_s ObjTab[201];
extern struct gdeb_s GDeb[170];
extern struct nuvec_s *vec;
extern s32 Paused;
extern s32 new_mode;
extern s32 new_level;
extern s32 GameMode;
extern s32 Adventure;
extern s32 TimeTrial;
extern s32 Hub;
extern s32 LivesLost;
extern s32 InvincibilityCHEAT;
extern struct game_s Game;
extern struct plritem_s plr_lives;
extern struct nuvec_s ShadNorm;
extern s32 temp_crate_type;
extern CrateCube *temp_pCrate;
extern s32 temp_xzmomset;
extern struct nuvec_s *CamOveride;
extern struct nuvec_s BattleShipCollScale;
extern struct nuvec_s SPACETRAN;
extern s32 PLAYERCOUNT;
extern s32 VEHICLECONTROL;
extern struct nuvec_s FarmCameraStart;
extern float FarmZepplinSpeed;
extern struct nuvec_s FarmZepplinStart;
extern float FarmZepplinTimer;
extern struct nuvec_s ZepplinGliderOffset;
extern struct nuvec_s WRStartPos;
extern float WRStartAng;
extern struct trail_s trail[128];
extern s32 trailpt;
extern s32 trailair;
extern float CRATEBALLOONOFFSET;
extern float CRATEBALLOONRADIUS;

/* vehicle tuning externs */
extern float Level_GliderSpeed;
extern float Level_GliderFloor;
extern float Level_GliderCeiling;
extern float Level_GliderCurrentCeiling;
extern float Level_GliderRadius;
extern float Level_GliderCentreX;
extern float Level_GliderCentreZ;
extern float Level_DeadTime;
extern float Level_TargetTime;
extern float WBLIMX;
extern float WBTILTZSCALE;
extern float WBTILTXSCALE;
extern float WBBARRELSPEED;
extern float WBBARRELSEEKSPEEDZERO;
extern float WBBARRELSEEKSPEEDNONZERO;
extern float WBFRICX;
extern float WBFRICY;
extern float FIXVELFADETIME;
extern s32 BEENHITTORNBUZZTIME;
extern s32 BEENHITTORNRUMTIME;
extern float WBTARGLIMX;
extern float WBTARGLIMY;
extern float WBTARGSCALE;
extern float WeatherBossTargetAppearTime;
extern s32 BEENHITBIGBUZZTIME;
extern s32 BEENHITBIGRUMTIME;
extern float GLIDERHITSOUNDFREQUENCY;
extern s32 GLIDERHITSOUNDOTHERID;
extern int CRASHTEROIDSSATELLITEVOL;
extern int CRASHTEROIDSAMBIENTVOL;
extern float HAILCOLLRAD;
extern float HAILHIDDENTIME;
extern float HailFallSpeed;
extern float BOLTHOMESEEKSPEED;
extern float BOLTHOMESEEKTIME;
extern float WBTYPE2SCALETIME;
extern float WBBOSSINTRODIST;
extern float WBIntroTweenTime;
extern float WBINTROLOOKAHEAD;
extern float WBITILTZSCALE;
extern float WBITILTXSCALE;
extern float WBDISTANCESPEED;
extern float WBDISTANCETIME;
extern float WBDISTANCEYSPEED;
extern float WBDISTANCEYTIME;
extern float WBSCALE;
extern float WBSKELTIME;
extern float WBSKELTIMERAND;
extern float WBSNOWBALLSPEED;
extern float WBSNOWCONESPEED;
extern float WBANGSCALE;
extern float WBANGSCALE2;
extern float WBANGSCALE3;
extern float WBANGOFF;
extern float WBANGOFF2;
extern float WBLOLOSCALE;
extern float WBLOLOTILTX;
extern struct nuvec_s WBMASKONHIGH;
extern struct nuvec_s WBMASKLEFT;
extern float WBMASKLEFTRAD;
extern struct nuvec_s WBMASKRIGHT;
extern float WBMASKRIGHTRAD;
extern struct nuvec_s WBMASKEYES;
extern float WBMASKEYESRAD;
extern struct nuvec_s WBMASKCHEST;
extern float WBMASKCHESTRAD;
extern struct nuvec_s WBAKUMASKONHIGH;
extern float WBAKUAKUSCALE;
extern float WBLeftStartFrame;
extern float WBLeftStopFrame;
extern float WBLeftStopTurnFrame;
extern struct nuvec_s WBLeftFirePos;
extern int BALLATTACKVOL;
extern int CHESTATTACKAVOL;
extern int CHESTATTACKBVOL;
extern float CHESTATTACKSOUNDTIME;
extern int BEAMVOL;
extern int EYEATTACKVOL;
extern float EYESTOPTIME;
extern float EYEBOLTFIRESCALEX;
extern float EYEBOLTFIRESCALEY;
extern float EYEBOLTFIRESPEED;
extern float EYEBOLTFIREY;
extern float RUMZOOM;
extern float RADARBASESCALE;
extern float RADARSCALE;
extern float RADARSCALEX;
extern float DOTSCALE;
extern float RadarX;
extern float RadarY;
extern float RadarZ;
extern float ATLASBOOSTSPEED;
extern int ATLASBOOSTVOL;
extern float ATLASLOOPVOL;
extern float BALLRUMBLESLOPE;
extern s32 LIFTPLAYER;
extern float FFINTROHEIGHT;
extern float FFINTROCAMYOFF;
extern struct nuvec_s SpaceFirePosA;
extern struct nuvec_s SpaceFirePosB;
extern struct nuvec_s WeatherBossFirePoint;
extern float WBBULLETSPEED;
extern float WBBULLETFIRERATE;
extern float TornFireY;
extern s32 TORPTARGBLEEPTIME;
extern s32 TORPTARGFAILBLEEPTIME;
extern s32 TORPTARGAQUIREBLEEPTIME;
extern float TorpedoTime;
extern float TARGETBLEEPTIME;
extern float SeekTiltZOveride;
extern float SeekTiltZZero;
extern float SeekTiltZNonZero;
extern float CRASHTEROIDSSeekTiltXTimeZero;
extern float CRASHTEROIDSSeekTiltXTimeNonZero;
extern float TORNSeekTiltXTimeZero;
extern float TORNSeekTiltXTimeNonZero;
extern GLIDERBULLET *FreeGliderBulletList[64];
extern GLIDERBULLET GliderBullet[64];
extern GLIDERBULLET *FreeEnemyGliderBulletList[100];
extern GLIDERBULLET EnemyGliderBullet[100];
extern GLIDERBULLET *UsedEnemyGliderBulletList[100];
extern GLIDERBULLET *UsedGliderBulletList[64];
extern s32 FreeEnemyGliderBulletNum;
extern s32 FreeGliderBulletNum;
extern s32 UsedEnemyGliderBulletNum;
extern s32 UsedGliderBulletNum;
extern s32 BulletEnemyNumFrees;
extern s32 BulletNumFrees;
extern struct creature_s *player;
extern s32 BEENHITBUZZTIME;
extern float BIGSCALE;
extern s32 CRASHTEROIDSENGINEVOL;
extern s32 CRASHTEROIDSZOFFAVOL;
extern float CrashteroidsStartAngle;
extern float FIREFLYVICDANCETIME;
extern float FIXVELBUMPAMOUNT;
extern float FIXVELTIME;
extern s32 GLIDENGINEVOL;
extern float GLIDENGPITCHSLOPE;
extern float GLIDENGRUMBLESLOPE;
extern float GLIDRUMTIM;
extern s32 GLIDRUMVOL;
extern s32 GliderRumble;
extern s32 HITSHIELDVOL;
extern s32 HITSHIELDVOLA;
extern float HovaRad;
extern float HovaY;
extern s32 LastNumZoffasFiring;
extern float MOSENGPITCHSLOPE;
extern float MOSENGRUMBLESLOPE;
extern float OIDENGPITCHSLOPE;
extern float OIDENGRUMBLESLOPE;
extern float OIDZOFPITCHSLOPE;
extern float SHIELDSCALEY;
extern s32 SINKENGINEVOL;
extern float SMALLSCALE;
extern float SPACEMINSPEEDIN;
extern s32 SPACESTATIONEXPLODEVOL;
extern float SPACETORNADOSCALE;
extern s32 UFOEXPLODEID;
extern float ZoffaAlter;
extern s32 jonemit;
extern s32 jonframe1;
extern float jtemp1;
extern float torndist;
extern float ROCKSMASHVOL;
extern float HAILFLYUPSPEED;
extern float HAILTIMERANGE;
extern s32 SkipNum;
extern float SkipLeftA;
extern float SkipLeftB;
extern float SkipLeftC;
extern float LighteningFallXMin;
extern float LighteningFallXMax;
extern struct nuvec_s ExtroStillCamPos;
extern struct nuvec_s ExtroStillInterest;
extern struct nuvec_s ExtroStillVelocity;
extern struct nuvec_s GliderBulletReflectPos;
extern struct numtx_s GliderInvMtx;
extern struct nuvec_s GliderBombCollisionPos;
extern float FireFlyStartAngle;
extern float CrashteroidsStartAngle;
extern float TimeTrialWait;
extern struct nuvec_s ExhaustPosA;
extern struct nuvec_s ExhaustPosB;
float WBCamDist;
float SeekGliderCamTime;
struct nuvec_s TORNADOCAMIDEAL;
float TORNADOINTERESTY;
float ExtroAngOff;
float ExtroChangeVal;
struct nuvec_s ExtroIdealPos;
float ExtroSeekZeroTime;
float ExtroStillToEndTime;
float ExtroTargAng;
float FFTWEENTIME;
float CameraSeekSpeedAutoPilot;
s32 *GliderBulletReflectMtx;
float MOSENGPITCH;
float OIDENGPITCH;
float GLIDENGPITCH;
s32 GUNBOATVOLUME;
struct nuvec_s FireFlyStart;
struct nuvec_s CrashteroidsStart;
struct nuvec_s LighteningHailVel;
s32 jonfirst = 0;
struct VEHMASK VehicleMask[2];
ZOFFASTRUCT EnemyZoffa[4];
HOVASTRUCT HovaBlimp[6];
static struct nuvec_s GliderAutoPosition;
s32 WBIntroOn;
float FireFlyIntroTween;
s32 FlyingLevelExtro;
GLIDERSTRUCT PlayerGlider;
float WBLIMX;
float WBMAXY;
float WBMINY;
float WBTARGLIMX;
float WBTARGLIMY;
float WeatherBossTargetAppearTimer;
struct nuvec_s WeatherBossTargetPos;

// NGC MATCH
void VehicleSetup(void) { JeepInControl = 0; }

// NGC MATCH
void MoveVehicle(struct creature_s *Cre, struct nupad_s *Pad) {
  switch (Cre->obj.vehicle) {
  case 0x81:
  case 0x8B:
  case 0x36:
    MoveGlider(Cre, Pad);
    return;
  case 0x53:
    MoveAtlas(Cre, Pad);
    return;
  case 0x63:
    MovePlayerJeep();
    return;
  }
}

// NGC MATCH
s32 PickGliderTarget(struct nuvec_s **Target, struct nuvec_s **Vel,
                     s32 *Moving) {
  float best;

  if ((PlayerGlider.AutoPilot != 0) || (FlyingLevelExtro != 0)) {
    return 0;
  } else {
    if (Level == 0x24) {
      best = GetBigGunBestTarget(0.0f, Target, Vel, Moving);
    } else {
      best = GetZoffaBestTarget(
          GetGunBoatBestTarget(
              GetBattleShipBestTarget(0.0f, Target, Vel, Moving), Target, Vel,
              Moving),
          Target, Vel, Moving);
    }
    return (best > 0.96f);
  }
}

// NGC MATCH
s32 StillLockedOnTarget(struct nuvec_s *Target) {
  struct nuvec_s *v0;
  struct nuvec_s nStack_40;
  struct nuvec_s nStack_30;
  struct nuvec_s nStack_20;
  float Dot;
  float Mag;

  if (FlyingLevelExtro == 0) {
    v0 = SetNuVecPntr(0.0f, 0.0f, 1.0f);
    NuVecMtxRotate(&nStack_40, v0, &GameCam[0].m);
    NuVecSub(&nStack_30, Target, &GameCam[0].pos);
    Mag = NuVecMag(&nStack_30);
    if (10.0f < Mag) {
      NuVecScale(1.0f / Mag, &nStack_30, &nStack_30);
      Dot = DotProduct(&nStack_40, &nStack_30);
      if (0.9f < Dot) {
        NuVecSub(&nStack_20, &PlayerGlider.Position, Target);
        NuVecNorm(&nStack_20, &nStack_20);
        NuVecScale(3.0f, &nStack_20, &nStack_20);
        if (NewRayCast(Target, &nStack_20, 0.0f) == 0) {
          return 1;
        }
        return 0;
      }
    }
  }
  return 0;
}

// NGC MATCH
s32 GetGliderHealthPercentage(struct creature_s *Cre) {
  if (Cre->Buggy == NULL) {
    return 100;
  }
  return Cre->Buggy[0x2f].a;
}

// NGC MATCH
void InitWeatherBossTarget(void) {
  WeatherBossTargetPos = SetNuVec(0.0f, 0.0f, -15.0f);
  WeatherBossTargetVel = v000;
  WeatherBossTargetAppearTimer = 0;
  return;
}

// NGC MATCH
void JonnyParticles(struct nuvec_s *Pos) {
  struct nuvec_s vec;
  s32 i;

  if (ebgo != 0) {
    if (ebgo == 1) {
      ebgo = 2;
      ebpos[0].x = 99999.0f;
      ebtime[0] = 0.0f;
      ebtime[1] = 10.0f;
      ebtime[2] = 20.0f;
      ebpos[1].x = 99999.0f;
      ebpos[2].x = 99999.0f;
    }
    for (i = 0; i < 3; i++) {
      ebtime[i] = (ebtime[i] - 1.0f);
      if (ebtime[i] < 0.0f) {
        if (ebpos[i].x != 99999.0f) {
          ebpos[i].y = ebpos[i].y + 1.0f;
          AddGameDebris(0x17, &ebpos[i]);
        }
        ebtime[i] = 30.0f;
        vec = *Pos;
        vec.x = ((s32)(qrand() - 0x8000U) * 0.000061035156f + vec.x);
        vec.z = ((s32)(qrand() - 0x8000U) * 0.000061035156f + vec.z);
        vec.y = (vec.y + 12.0f);
        vec.y = (NewShadow(&vec, 0.0f) + 12.0f);
        ebpos[i] = vec;
      }
      if (ebpos[i].x != 99999.0f) {
        ebpos[i].y -= 0.4f;
        AddVariableShotDebrisEffect(GDeb[64].i, &ebpos[i], 1, 0, 0);
      }
    }
  }
  return;
}

// NGC MATCH
void DrawJonny(void) {
  s32 loop;
  struct numtx_s m;

  if ((JonnyOn == 1) && (ebgo != 0)) {
    for (loop = 0; loop < 3; loop++) {
      if (ebpos[loop].x != 99999.0f) {
        NuMtxSetRotationX(&m, (ebtime[loop] * 128.0f));
        NuMtxRotateZ(&m, (ebtime[loop] * 200.0f));
        NuMtxTranslate(&m, &ebpos[loop]);
        NuSpecialDrawAt(&ObjTab[0x58].obj, &m);
      }
    }
  }
  return;
}

// NGC MATCH
void BossBar(float x, float y, float z, float xs, float ys, s32 i, s32 j) {
  float HealthScale;

  HealthScale = ((float)i / (float)j);
  if (1.0f < HealthScale) {
    HealthScale = 1.0f;
  }
  if (HealthScale < 0.0f) {
    HealthScale = 0.0f;
  }
  DrawPanel3DObject(0xb9, x, y, z, xs, ys, 0.1f, 0, 0, 0, ObjTab[185].obj.scene,
                    ObjTab[185].obj.special, 1);
  DrawPanel3DObject(0xbb, x, y, z, (xs * HealthScale), ys, 0.1f, 0, 0, 0,
                    ObjTab[187].obj.scene, ObjTab[187].obj.special, 1);
  return;
}

s32 GetCurrentRumbleObjectives(void) { return EarthBoss.HitPoints; }

s32 GetRumblePlayerHealthPercentage(struct creature_s *Cre) {
  return PlayerAtlas.HitPoints;
}

// NGC MATCH
void DrawAtlas(struct creature_s *Cre) {
  struct ATLASSTRUCT *Atlas;
  struct numtx_s Matrix;
  s32 i;

  Atlas = (struct ATLASSTRUCT *)Cre->Buggy;
  NuQuatToMtx(&Atlas->Quat, &Matrix);
  NuMtxTranslate(&Matrix, &Atlas->Position);
  i = CRemap[83];
  if (i != -1) {
    NuHGobjRndr(CModel[i].hobj, &Matrix, 1, NULL);
  }
  return;
}

// NGC MATCH
void ControlAtlas(struct ATLASSTRUCT *Atlas, struct nupad_s *Pad,
                  float DeltaTime) {
  float AnX;
  float AnZ;
  struct nuvec_s Ctrl;
  float Scale;

  AnX = (float)(Pad->l_alg_x - 0x7f);
  AnZ = (float)(Pad->l_alg_y - 0x7f);
  if ((Pad->paddata & 0x4000) != 0) {
    AnZ = 127.0f;
  }
  if ((Pad->paddata & 0x1000) != 0) {
    AnZ = -127.0f;
  }
  if ((Pad->paddata & 0x2000) != 0) {
    AnX = 127.0f;
  }
  if ((Pad->paddata & 0x8000) != 0) {
    AnX = -127.0f;
  }
  if (ProcessTimer(&Atlas->InhibitControlTimer) == 0) {
    if (Atlas->InhibitControlTimer < 1.0f) {
      Scale = 1.0f - Atlas->InhibitControlTimer;
    } else {
      Scale = 0.0f;
    }
    AnX *= Scale;
    AnZ *= Scale;
  }
  if (((Atlas->OnGround & 3U) == 0) && (Level == 0x15)) {
    AnX = 0.0f;
    AnZ = 0.0f;
  }
  Ctrl.x = AnX;
  Ctrl.z = -AnZ;
  Ctrl.y = 0.0f;
  if (GameTimer.frame < 0x3c) {
    Ctrl.z = 0.0f;
    Ctrl.x = 0.0f;
  }
  Atlas->InputMag = NuVecMag(&Ctrl);
  if (Atlas->InputMag >= 40.0f) {
    Atlas->InputAng =
        ((GameCam[0].yrot + NuAtan2D(Ctrl.x, Ctrl.z)) & 0xffff) / 182.0444f;
    Atlas->InputMag = ((Atlas->InputMag - 40.0f) * 1.02f) / 88.0f;
    if (Atlas->InputMag > 1.0f) {
      Atlas->InputMag = 1.0f;
    } else if (Atlas->InputMag < -1.0f) {
      Atlas->InputMag = -1.0f;
    }
  } else {
    Atlas->InputMag = 0.0f;
  }
}

// NGC MATCH
void ProcessMovementAtlas(struct ATLASSTRUCT *Atlas, float DeltaTime) {
  float Time2;
  float Fric;
  float Scale;
  float NewMag;
  float Mag;
  struct nuvec_s Resolved;
  float Tilt;

  Time2 = (DeltaTime * DeltaTime);
  Mag = NuVecMag(&Atlas->Velocity);
  if (Mag > 0.0f) {
    NewMag = Mag;
    if (Mag < 4.0f) {
      Scale = 0.05f;
    } else {
      Scale = (Mag - 4.0f) * 0.1f;
    }
    ApplyFriction(&NewMag, Scale, DeltaTime);
    Atlas->Velocity.x *= (NewMag / Mag);
    Atlas->Velocity.y *= (NewMag / Mag);
    Atlas->Velocity.z *= (NewMag / Mag);
  }
  Atlas->Force = SetNuVec(0.0f, -10.0f, 0.0f);
  NuVecRotateY(&Resolved, &Atlas->Velocity,
               (int)(-Atlas->InputAng * 182.04445f));
  if (Resolved.z < 0.0f) {
    Resolved.z = 0.0f;
  }
  if (Level == 0x15) {
    Tilt = -Atlas->InputMag * 89.0f;
  } else {
    Tilt = -Atlas->InputMag * 53.0f;
  }
  SeekHalfLife(&Tilt, 0.0f, 3.0f, Resolved.z);
  NuVecRotateX(&Atlas->Force, &Atlas->Force, (int)(Tilt * 182.04445f));
  NuVecRotateY(&Atlas->Force, &Atlas->Force,
               (int)(Atlas->InputAng * 182.04445f));
  Atlas->TargetPosition.x = (Time2 * Atlas->Force.x) * 0.5f +
                            (Atlas->Velocity.x * DeltaTime + Atlas->Position.x);
  Atlas->TargetPosition.y = (Time2 * Atlas->Force.y) * 0.5f +
                            (Atlas->Velocity.y * DeltaTime + Atlas->Position.y);
  Atlas->TargetPosition.z = (Time2 * Atlas->Force.z) * 0.5f +
                            (Atlas->Velocity.z * DeltaTime + Atlas->Position.z);
  Atlas->Velocity.x = (Atlas->Force.x * DeltaTime + Atlas->Velocity.x);
  Atlas->Velocity.y = (Atlas->Force.y * DeltaTime + Atlas->Velocity.y);
  Atlas->Velocity.z = (Atlas->Force.z * DeltaTime + Atlas->Velocity.z);
}

// NGC MATCH
void InitAtlas(struct ATLASSTRUCT *Atlas, struct nuvec_s *Pos, float Radius,
               s32 Type) {
  struct creature_s *Cre;
  struct creature_s *Cre2;
  struct cdata_s *cdata;

  Cre = Atlas->Cre;
  memset(Atlas, 0, 900);
  Atlas->Type = Type;
  Atlas->Cre = Cre;
  Atlas->Position = *Pos;
  Atlas->OldPosition = Atlas->Position;
  Atlas->Radius = Radius;
  Atlas->AngleY = 90.0f;
  Atlas->Quat = SetNuQuat(0.0f, 0.0f, 0.0f, 1.0f);
  Atlas->FrameQuat[0] = Atlas->Quat;
  Atlas->FrameQuat[1] = Atlas->Quat;
  Atlas->FrameQuat[2] = Atlas->Quat;
  Atlas->FrameQuat[3] = Atlas->Quat;
  Atlas->LastQuat = Atlas->Quat;
  Atlas->LastNormal = SetNuVec(0.0f, 1.0f, 0.0f);
  Atlas->PlatformId = -1;
  if (Cre != NULL) {
    InitTrail();
    Cre2 = Atlas->Cre;
    (Cre2->obj).SCALE = 1.0f;
    (Cre2->obj).scale = 1.0f;
    cdata = &CData[83];
    (Atlas->Cre->obj).bot = cdata->min.y;
    (Atlas->Cre->obj).top = cdata->max.y;
    (Atlas->Cre->obj).vehicle = 0x53;
  }
  return;
}

// NGC MATCH
void ResetAtlas(struct creature_s *Cre) {
  struct ATLASSTRUCT *Atlas;
  struct nuvec_s Pos;

  Atlas = (struct ATLASSTRUCT *)Cre->Buggy;
  if (Atlas == NULL) {
    Cre->Buggy = (struct NEWBUGGY *)&PlayerAtlas;
    Atlas = &PlayerAtlas;
    memset(&PlayerAtlas, 0, 900);
    PlayerAtlas.Cre = Cre;
  }
  AtlasFrame = -1;
  AtlasEmbeddedFrame = 0;
  Pos.x = (Cre->obj).pos.x;
  Pos.y = (Cre->obj).pos.y + 1.0f;
  Pos.z = (Cre->obj).pos.z;
  AtlasWhackValue = 0.0f;
  ResetAtlasCamera = 1;
  AtlasWhackTimer = 0.0f;
  RumbleCamTween = 1.0f;
  RumbleCamTweenDest = 1.0f;
  RumbleCamVal = 0.0f;
  InitAtlas(Atlas, &Pos, 0.65f, 1);
  Atlas->HitPoints = 100;
  Atlas->DestHitPoints = 100;
  return;
}

// NGC MATCH
void KillAtlasphere(struct ATLASSTRUCT *Atlas) {
  if (Atlas->Cre != NULL) {
    Atlas->Dead = 1;
    Atlas->Cre->obj.invincible = 0;
    Atlas->Velocity = v000;
    if ((Atlas->Cre->obj).dead == 0) {
      KillGameObject(&Atlas->Cre->obj, 0xb);
    }
  }
  return;
}

struct nuvec_s lbl_80118DA8 = {0.0f, -2.35f, 7.98f};

// NGC MATCH
void InitEarthBoss(void) {
  struct nuvec_s Start;

  Start = lbl_80118DA8;
  InitAtlas(&EarthBoss, &Start, 1.0f, 2);
  EarthBoss.HitPoints = 3;
  MyInitModelNew(&EarthBoss.Shell, 0xad, 0x1f, 0, NULL, &EarthBoss.Position);
  MyInitModelNew(&EarthBoss.Crunch, 0x7f, 0x22, 0, NULL, &EarthBoss.Position);
  ShootRockSound = 0;
  EarthBoss.LastAction = -1;
  EarthBoss.Action = 0;
  return;
}

void InitRumblePanel(void) { NumRockPanel = 0; }

void DrawEarthBossLevelExtra(void) {
  DrawEarthBoss();
  DrawJeepRocks();
  DrawVehMasks();
  return;
}

// NGC MATCH
void ProcessEarthBossVortex(void) {
  if (EarthBossVortexOpen != 0) {
    CheckAtlasVortex(&EarthBoss);
    CheckAtlasVortex(&PlayerAtlas);
  }
}

// NGC MATCH
void LoadVehicleStuff(void) {
  switch (Level) {
  case 3:
    LoadWesternArenaData();
    break;
  case 0xd:
    jonfirst = 0;
    break;
  }
  return;
}

// NGC MATCH
void InitVehMasks(void) { memset(VehicleMask, 0, 0x2e8); }

// NGC MATCH
void InitVehMask(s32 Indx, s32 Id) {
  struct VEHMASK *Mask;

  memset(&VehicleMask[Indx], 0, 0x174);
  Mask = &VehicleMask[Indx];
  if (MyInitModelNew(&Mask->MainDraw, Id, 0x22, 0, NULL, &Mask->Position) !=
      0) {
    Mask->Id = Id;
  }
  return;
}

// NGC MATCH
void SetNewMaskStuff(s32 Indx, struct nuvec_s *Centre, struct nuvec_s *Off,
                     float Rad, float AngInc, float TweenInc, s32 FixedNUVEC,
                     s32 KillAtEnd, float Scale, float TiltX) {
  struct VEHMASK *Mask;

  Mask = &VehicleMask[Indx];
  Mask->KillAtEnd = KillAtEnd;
  if (Mask->Active != 0) {
    Mask->Tween = 1.0f;
    Mask->TweenInc = TweenInc * 0.01666667f;
    Mask->Offset[1] = Mask->Offset[0];
    Mask->AngInc[1] = Mask->AngInc[0];
    Mask->Rad[1] = Mask->Rad[0];
    Mask->Scale[1] = Mask->Scale[0];
    Mask->TiltX[1] = Mask->TiltX[0];
    if (Mask->Point[0] == &Mask->Store[0]) {
      Mask->Point[1] = &Mask->Store[1];
      Mask->Store[1] = *Mask->Point[0];
    } else {
      Mask->Point[1] = Mask->Point[0];
    }
  } else {
    Mask->Tween = 0.0f;
    Mask->Active = 1;
    Mask->Point[1] = &v000;
  }
  if (FixedNUVEC != 0) {
    Mask->Store[0] = *Centre;
    Mask->Point[0] = &Mask->Store[0];
  } else {
    Mask->Point[0] = Centre;
  }
  Mask->Offset[0] = *Off;
  Mask->AngInc[0] = AngInc * 0.01666667f;
  Mask->Rad[0] = Rad;
  Mask->Scale[0] = Scale;
  Mask->TiltX[0] = TiltX;
  return;
}

// NGC MATCH
void ProcessGliderMovementWB(GLIDERSTRUCT *Glider) {
  float Dest;
  struct nuvec_s Temp;

  if (WBIntroOn != 0) {
    Glider->Position = WBIntroGliderPos;
    Glider->Position.z = 0.0f;
  } else {
    if (Glider->BarrelRoll == 0) {
      Glider->Velocity.z = -Level_GliderSpeed;
      Glider->Velocity.x = Glider->TiltZ * WBTILTZSCALE;
      Glider->Velocity.y = Glider->TiltX * WBTILTXSCALE;
    }
    NuVecScaleAccum(0.01666667f, &Glider->Position, &Glider->Velocity);
    if (Glider->Position.x > WBLIMX) {
      Glider->Position.x = WBLIMX;
    }
    if (Glider->Position.x < -WBLIMX) {
      Glider->Position.x = -WBLIMX;
    }
    if (Glider->Position.y > WBMAXY) {
      Glider->Position.y = WBMAXY;
    }
    if (Glider->Position.y < WBMINY) {
      Glider->Position.y = WBMINY;
    }
    if (Glider->BarrelRoll > 1) {
      Dest = -WBBARRELSPEED;
    } else if (Glider->BarrelRoll <= -2) {
      Dest = WBBARRELSPEED;
    } else {
      Dest = 0.0f;
    }
    if (Dest == 0.0f) {
      SeekHalfLife(&Glider->BarrelSpeedX, Dest, WBBARRELSEEKSPEEDZERO,
                   0.01666667f);
    } else {
      SeekHalfLife(&Glider->BarrelSpeedX, Dest, WBBARRELSEEKSPEEDNONZERO,
                   0.01666667f);
    }
    Temp = SetNuVec(Glider->BarrelSpeedX, 0.0f, 0.0f);
    NuVecRotateY(&Temp, &Temp, (s32)(Glider->AngleY * 182.0444f));
    Glider->Position.x = Temp.x * 0.01666667f + Glider->Position.x;
    Glider->Position.y = Temp.y * 0.01666667f + Glider->Position.y;
    Glider->Position.z = Temp.z * 0.01666667f + Glider->Position.z;
  }
  return;
}

// NGC MATCH
void MoveGlider(struct creature_s *Cre, struct nupad_s *Pad) {
  GLIDERSTRUCT *Glider;
  struct nuvec_s *PosPtr;
  struct nuvec_s Temp;
  struct nuvec_s Temp2;
  struct nuvec_s Temp3;
  struct nuvec_s ClockPos;
  void *Clock;
  s32 Collided;
  s32 debIndex;
  s32 GliderRumbleVal;
  float RumbleSlope;
  float f31;

  Glider = (GLIDERSTRUCT *)Cre->Buggy;
  if (Glider == NULL) {
    Glider = &PlayerGlider;
    Cre->Buggy = (struct NEWBUGGY *)Glider;
    PlayerGlider.Cre = Cre;
    return;
  }

  ProcessTimer(&Glider->NextHitSoundTimer);
  ProcessTimer(&Glider->ImmuneAsteroidsTimer);
  Glider->OldPosition = Glider->Position;

  if (Glider->TerminalDive != 0) {
    Glider->HitPoints = 0;
  }

  if (Glider->HitPoints <= 0) {
    if (Level == 0x1a) {
      DeadGliderCoco(Glider);
      return;
    }
    if (Level == 0x18) {
      Glider->HitTimer = Level_DeadTime;
      GliderSmoke(Glider);
      DeadGliderWB(Glider);
      return;
    }
  }

  if (Glider->Dead != 0) {
    Glider->HitTimer = Level_DeadTime;
    GliderSmoke(Glider);
    Glider->Dead--;
    if (Glider->Dead > 0) return;
    Glider->Dead = 60;
    AddGameDebris(0x18, &Glider->Position);
    return;
  }

  ProcessTimer(&Glider->HitTimer);
  ControlGlider(Glider, Pad);
  if (Level == 0x18) {
    ProcessGliderMovementWB(Glider);
  } else {
    ProcessGliderMovement(Glider, 0.016666668f);
  }
  if (Level == 0x18) {
    ControlGliderWeatherBoss(Glider, Pad);
  }

  if (CollideGliderBullets(&Glider->Position, 0.8f, 1, 1.0f, 0, 0) != 0) {
    Glider->HitTimer += 0.5f;
    if (InvincibilityCHEAT == 0 && VehicleLevelImmune == 0) {
      Glider->HitPoints--;
    }
    if (Glider->HitPoints < 0) {
      Glider->HitPoints = 0;
    }
    NewBuzz(&player->rumble, BEENHITBUZZTIME);
    if (Glider->NextHitSoundTimer == 0.0f) {
      if (Level == 0x1a) {
        MyGameSfx(0xc0, &Glider->Position, 0x7fff);
      } else {
        MyGameSfx(0x5a, &Glider->Position, 0x7fff);
      }
      Glider->NextHitSoundTimer = GLIDERHITSOUNDFREQUENCY;
    } else {
      MyGameSfx(GLIDERHITSOUNDOTHERID, &Glider->Position, 0x4fff);
    }
  }

  GliderBulletsHitThings(Glider);
  if (Level == 0x12 || Level == 0x24) {
    GliderBombsHitThings(Glider);
  }

  if (Glider->HitPoints <= 0) {
    Glider->HitPoints = 0;
    Glider->TerminalDive = 1;
    Glider->HitTimer = Level_DeadTime;
  }

  if (Level != 0x18 && Glider->Dead == 0 && new_mode == -1 && new_level == -1 &&
      GameTimer.frame > 0x3c) {
    Collided = 0;
    if (Glider->Position.y < 6.0f) {
      Collided = (Level == 0xd);
    }
    if (Glider->Position.y < 0.0f) {
      Collided |= (Level == 0x12);
    }
    if (Collided == 0) {
      if (Level == 0x12 && Glider->TerminalDive == 0) {
        Temp = *(struct nuvec_s *)&v000;
        Temp.y = -11.0f;
        Temp2 = Glider->Position;
        Temp2.y += 10.0f;
        if (NewRayCastMask(&Temp2, &Temp, 0xff) != 0) {
          NuVecAdd(&Temp3, &Temp2, &Temp);
          if (Temp3.y > Glider->Position.y) {
            Glider->Position = Temp3;
          }
          Glider->PullUpTimer = 0.5f;
        }
      } else {
        NuVecSub(&Temp, &Glider->Position, &Glider->OldPosition);
        if (NewRayCast(&Glider->OldPosition, &Temp, 0.0f) != 0) {
          Collided = 1;
        }
      }
    }
    if (Collided == 0 && Level == 0x12) {
      if (CollideWithBattleShips(&PlayerGlider.Position, 1.0f) != 0) {
        Collided = 1;
      }
    }
    if (Collided != 0) {
      ExplodeGlider(Glider);
    }
  }

  GliderSmoke(Glider);
  Cre->obj.pos = Glider->Position;

  if (HitCrates(&Cre->obj, 1) != 0) {
    if (temp_crate_type == 5) {
      if (Level == 0x12) {
        if (qrand() <= 0xbfff) {
          Glider->HitPoints += 50;
          if (Glider->HitPoints > 100) {
            Glider->HitPoints = 100;
          }
        }
      } else {
        AddExtraLife(&temp_pCrate->pos, 2);
      }
    }
  } else {
    f31 = Cre->obj.SCALE;
    f31 += CRATEBALLOONRADIUS;
    if (HitCrateBalloons(&Cre->obj.pos, f31) != 0) {
      AddGameDebris(0x1a, &Cre->obj.pos);
      AddScreenWumpa(1, temp_pCrate->pos.x,
                     temp_pCrate->pos.y + CRATEBALLOONOFFSET,
                     temp_pCrate->pos.z);
      temp_pCrate->flags = (temp_pCrate->flags | 0x400) ^ 0x400;
    } else {
      if (HitItems(&Cre->obj) != 0) {
        AddGameDebris(0x1a, &Cre->obj.pos);
      } else {
        Clock = FindClock();
        if (Clock != NULL) {
          PosPtr = (struct nuvec_s *)((char *)Clock + 0x6c);
          ClockPos.x = PosPtr->x;
          ClockPos.y = PosPtr->y + CRATEBALLOONOFFSET;
          ClockPos.z = PosPtr->z;
          f31 = Cre->obj.SCALE;
          f31 += CRATEBALLOONRADIUS;
          if (NuVecDistSqr(&Cre->obj.pos, &ClockPos, &Temp) < f31 * f31) {
            AddGameDebris(0x1a, &Cre->obj.pos);
            StartTimeTrial(PosPtr, 1);
          }
        }
      }
    }
  }

  Glider->LastInTornado = Glider->InTornado;
  Glider->ApparentPosition = Glider->PositionStack[Glider->StackIndx];
  Glider->ApparentVelocity = Glider->VelocityStack[Glider->StackIndx];
  Glider->PositionStack[Glider->StackIndx] = Glider->Position;
  Glider->VelocityStack[Glider->StackIndx] = Glider->Velocity;
  NuVecScaleAccum(0.5f, &Glider->PositionStack[Glider->StackIndx],
                  &Glider->Velocity);
  Glider->StackIndx++;
  if (Glider->StackIndx > 29) {
    Glider->StackIndx = 0;
  }

  ProcessTimer(&Glider->OverideTiltZ);
  GliderFire(Glider, Pad);

  if (Level == 0x12) {
    debIndex = 0x86;
    NuMtxSetRotationZ(&mTEMP, (int)(Glider->TiltZ * 182.04445f));
    NuMtxRotateX(&mTEMP, (int)((Glider->TiltX + 15.0f) * 182.04445f));
    NuMtxRotateY(&mTEMP, (int)(Glider->AngleY * 182.04445f));
    NuMtxTranslate(&mTEMP, &Glider->Position);
    if (Glider->HitPoints == 0) {
      debIndex = 0x84;
    }
    NuVecMtxTransform(&Temp, &ExhaustPosA, &mTEMP);
    AddVariableShotDebrisEffect(GDeb[debIndex].i, &Temp, 1, 0, 0);
    NuVecMtxTransform(&Temp, &ExhaustPosB, &mTEMP);
    AddVariableShotDebrisEffect(GDeb[debIndex].i, &Temp, 1, 0, 0);
  }

  if (Glider->InTornadoTime == 0.0) {
    if (ProcessTimer(&Glider->CamTornRecoverTimer) != 0) {
      Glider->CamAngleY = Glider->AngleY;
      Glider->CamTiltX = Glider->TiltX;
    } else {
      float rangle;
      float t;
      rangle = Glider->AngleY - Glider->CamAngleY;
      rangle = Rationalise360f(rangle);
      t = 1.0f - Glider->CamTornRecoverTimer;
      Glider->CamAngleY = t * rangle + Glider->CamAngleY;
      rangle = Glider->TiltX - Glider->CamTiltX;
      rangle = Rationalise360f(rangle);
      t = 1.0f - Glider->CamTornRecoverTimer;
      Glider->CamTiltX = t * rangle + Glider->CamTiltX;
    }
  }

  if (Level == 0x12) ProcessFireFlyIntro();
  if (Level == 0x1a) ProcessCrashteroidsIntro();

  f31 = NuFabs(Glider->TiltZ) * 0.5f;
  if (Glider->DestTiltX > 0.0f) {
    f31 = Glider->DestTiltX * 0.5f + f31;
  } else {
    f31 -= Glider->DestTiltX;
  }

  GliderRumble = 0;
  if (Level == 0x1a) {
    MyGameSfxLoopVolPitch(0xb5, &Glider->Position, (short)CRASHTEROIDSENGINEVOL,
                          (short)(OIDENGPITCH + (int)(OIDENGPITCHSLOPE * Glider->TiltX)));
    RumbleSlope = OIDENGRUMBLESLOPE;
  } else if (Level == 0x12) {
    MyGameSfxLoopVolPitch(0xb6, &Glider->Position, (short)SINKENGINEVOL,
                          (short)(MOSENGPITCH + (int)(MOSENGPITCHSLOPE * Glider->TiltX)));
    RumbleSlope = MOSENGRUMBLESLOPE * 1.25f;
  } else if (Level == 0x0d || Level == 0x18 || Level == 0x24) {
    MyGameSfxLoopVolPitch(0x5b, &Glider->Position, (short)GLIDENGINEVOL,
                          (short)(GLIDENGPITCH + (int)(GLIDENGPITCHSLOPE * Glider->TiltX)));
    RumbleSlope = GLIDENGRUMBLESLOPE;
  } else {
    goto skip_rumble;
  }
  GliderRumbleVal = (short)(int)(RumbleSlope * f31);
  if (GliderRumbleVal > 255) GliderRumbleVal = 255;
  if (GliderRumbleVal < 0) GliderRumbleVal = 0;
  GliderRumble = GliderRumbleVal;
skip_rumble:

  NewRumble(&player->rumble, GliderRumble);

  if (ProcessTimer(&Glider->NextEngRum) != 0) {
    frand();
    GliderRumble = GLIDRUMVOL;
    NewRumble(&player->rumble, GliderRumble);
    Glider->NextEngRum = GLIDRUMTIM;
  }

  if (Level == 0x1a) {
    if (PlayerGlider.InTornadoTime == 0.0f) {
      AddVariableShotDebrisEffect(GDeb[0x9f].i,
          (struct nuvec_s *)&Glider->Cre->mtxLOCATOR[8][0]._30, 1, 0, 0);
      AddVariableShotDebrisEffect(GDeb[0x9f].i,
          (struct nuvec_s *)&Glider->Cre->mtxLOCATOR[8][1]._30, 1, 0, 0);
      AddVariableShotDebrisEffect(GDeb[0x9f].i,
          (struct nuvec_s *)&Glider->Cre->mtxLOCATOR[9][0]._30, 1, 0, 0);
    }
  }
}


// NGC MATCH
void DrawGlider(struct creature_s *Cre) {
  GLIDERSTRUCT *Glider;
  s32 i;
  float ExtraX;

  Glider = (GLIDERSTRUCT *)Cre->Buggy;
  if ((Level == 0xd) || (Level == 0x1a)) {
    ExtraX = 0.0f;
  } else {
    ExtraX = 15.0f;
  }
  if (Glider != NULL) {
    if ((Glider->AutoPilot != 0) && (Level == 0xd)) {
      NuMtxSetRotationX(&mTEMP, (int)(ExtraX * 182.04445f));
      NuMtxRotateY(&mTEMP, 0);
      NuMtxTranslate(&mTEMP, &GliderAutoPosition);
    } else {
      if (Level == 0x1a) {
        NuMtxSetRotationZ(&mTEMP, (int)(Glider->TiltZ * 182.04445f));
        NuMtxRotateX(&mTEMP,
                     (int)((Glider->TiltX + Glider->TiltX) * 182.04445f));
        NuMtxRotateY(&mTEMP, (int)(Glider->AngleY * 182.04445f));
        NuMtxTranslate(&mTEMP, &Glider->Position);
      } else {
        NuMtxSetRotationZ(&mTEMP, (int)(Glider->TiltZ * 182.04445f));
        NuMtxRotateX(&mTEMP,
                     (int)((float)(Glider->TiltX + ExtraX) * 182.04445f));
        NuMtxRotateY(&mTEMP, (int)(Glider->AngleY * 182.04445f));
        NuMtxTranslate(&mTEMP, &Glider->Position);
      }
    }
    switch (Level) {
    case 0x1a:
      i = CRemap[0x81];
      break;
    case 0x12:
      i = CRemap[0x36];
      break;
    default:
      i = CRemap[0x8b];
      break;
    }
    if (i != -1) {
      DrawCharacterModel(CModel + i, &(Cre->obj).anim, &mTEMP, NULL, 1, NULL,
                         Cre->mtxLOCATOR[8], Cre->momLOCATOR[8], &Cre->obj);
    }
  }
  return;
}

// NGC MATCH
void InitGlider(GLIDERSTRUCT *Glider, struct nuvec_s *StartPos,
                float StartAng) {
  struct cdata_s *cdata;
  struct creature_s *temp;
  s32 i;

  WBIntroOn = 0;
  memset(Glider, 0, 0x440);
  Glider->Cre = player;
  if (StartPos != NULL) {
    Glider->Position = *StartPos;
  }
  Glider->HitPoints = 100;
  Glider->AngleY = StartAng;
  Glider->BarrelRoll = 0;
  Glider->TiltX = 0.0f;
  Glider->DestTiltX = 0.0f;
  Glider->TiltZ = 0.0f;
  Glider->DestTiltZ = 0.0f;
  Glider->FireTimer = 0.0f;
  Glider->RailAngle = 0.0f;
  for (i = 0; i < 0x10; i++) {
    Glider->LocatorList[i] = 0x7f;
    Glider->LocatorTime[i] = 0.0f;
  }
  edppRegisterPointerToGameCharLocation(&Glider->Position);
  if (Level == 0x1a) {
    cdata = CData + 0x81;
  } else {
    cdata = CData + 0x36;
  }
  temp = Glider->Cre;
  temp->obj.SCALE = 1.0f;
  temp->obj.scale = 1.0f;
  Glider->Cre->obj.bot = cdata->min.y;
  Glider->Cre->obj.top = cdata->max.y;
  FireFlyIntroTween = 0.0f;
  return;
}

void GliderFire(GLIDERSTRUCT *Glider, struct nupad_s *Pad) {
  int FireButton;
  static int LastFireButton;
  struct numtx_s Mat;
  static float FireBleepTimer;
  struct nuvec_s SpaceTargetDir;
  struct nuvec_s TornTargetDir;
  struct nuvec_s SpaceVel;
  struct nuvec_s SpacePos;
  struct nuvec_s TornFireDir;
  struct nuvec_s TornScale;
  struct nuvec_s TornPos;
  struct nuvec_s TornRot1;
  struct nuvec_s TornRot2;
  struct nuvec_s WBVelDir;
  struct nuvec_s WBScale;
  struct nuvec_s WBFirePt;
  struct nuvec_s WBPos;
  struct nuvec_s WBRot1;
  struct nuvec_s WBRot2;
  struct nuvec_s BombPos;

  FireButton = Pad->paddata & 0x40;

  switch (Level) {
  case 0x1A:
    NuMtxSetRotationZ(&Mat, (int)(Glider->TiltZ * 182.04445f));
    NuMtxRotateX(&Mat, (int)((Glider->TiltX * 2 - 10.0f) * 182.04445f));
    NuMtxRotateY(&Mat, (int)(Glider->AngleY * 182.04445f));
    NuVecMtxRotate(&SpaceTargetDir, SetNuVecPntr(0.0f, -3.5f, -133.33333f),
                   &Mat);
    NuVecMtxRotate(&GliderTargetPos, SetNuVecPntr(0.0f, -0.1f, 0.0f), &Mat);
    NuVecAdd(&GliderTargetPos, &GliderTargetPos, &Glider->Position);
    NuVecScaleAccum(0.74166667f, &GliderTargetPos, &SpaceTargetDir);
    break;
  case 0x0D:
    NuMtxSetRotationZ(&Mat, (int)(Glider->TiltZ * 182.04445f));
    NuMtxRotateX(&Mat, (int)(Glider->TiltX * 182.04445f));
    NuMtxRotateY(&Mat, (int)(Glider->AngleY * 182.04445f));
    GliderTargetPos = *(struct nuvec_s *)&Glider->Cre->mtxLOCATOR[8][0]._30;
    NuVecMtxRotate(&TornTargetDir, SetNuVecPntr(0.0f, -8.5f, -50.0f), &Mat);
    NuVecScaleAccum(1.5f, &GliderTargetPos, &TornTargetDir);
    break;
  case 0x18: default:
    NuMtxSetRotationZ(&Mat, (int)(Glider->TiltZ * 182.04445f));
    NuMtxRotateX(&Mat, (int)((Glider->TiltX + 0.0f) * 182.04445f));
    NuMtxRotateY(&Mat, (int)(Glider->AngleY * 182.04445f));
    break;
  }

  if (Glider->TerminalDive != 0) goto end;
  ProcessTimer(&Glider->FireTimer);
  if (FlyingLevelExtro != 0) goto end;
  if (WBIntroOn != 0) goto end;
  if (FlyingLevelCompleteTimer == 0.0f) goto end;
  if (Glider->AutoPilot != 0) goto end;
  if (Glider->FireTimer != 0.0f) goto end;
  if (!(Pad->paddata & 0x40) && Level != 0x12 && Level != 0x24) goto end;

  switch (Level) {
  case 0x1A:
    NuVecMtxRotate(&SpaceVel,
                   SetNuVecPntr(-SpaceFirePosA.x / 0.75f, 0.0f, -120.0f),
                   &Mat);
    NuVecMtxRotate(&SpacePos, &SpaceFirePosA, &Mat);
    NuVecAdd(&SpacePos, &SpacePos, &Glider->Position);
    NuVecScaleAccum(-0.00833333f, &SpacePos, &SpaceVel);
    AddGliderBullet(&Mat, &SpacePos, &SpaceVel, 0);

    NuVecMtxRotate(&SpaceVel,
                   SetNuVecPntr(-SpaceFirePosB.x / 0.75f, 0.0f, -120.0f),
                   &Mat);
    NuVecMtxRotate(&SpacePos, &SpaceFirePosB, &Mat);
    NuVecAdd(&SpacePos, &SpacePos, &Glider->Position);
    NuVecScaleAccum(-0.00833333f, &SpacePos, &SpaceVel);
    AddGliderBullet(&Mat, &SpacePos, &SpaceVel, 0);

    Glider->FireTimer = 0.1f;
    MyGameSfx(0xb6, NULL, 0x3fff);
    NewBuzz(&player->rumble, 3);
    break;

  case 0x0D:
    TornScale = kHalfScale;
    NuVecMtxRotate(&TornRot1, SetNuVecPntr(0.0f, 1.0f, 0.0f), &Mat);
    NuVecMtxRotate(&TornRot2, SetNuVecPntr(1.0f, 0.0f, 0.0f), &Mat);
    TornFireDir = SetNuVec(0.0f, TornFireY, -50.0f);
    NuVecMtxRotate(&TornFireDir, &TornFireDir, &Mat);
    NuMtxScale(&Mat, &TornScale);
    TornPos = *(struct nuvec_s *)&Glider->Cre->mtxLOCATOR[8][0]._30;
    NuVecScaleAccum((frand() - 0.5f) * 0.5f, &TornPos, &TornRot1);
    NuVecScaleAccum((frand() - 0.5f) * 0.5f, &TornPos, &TornRot2);
    if (AddGliderBullet(&Mat, &TornPos, &TornFireDir, 0) == 0) goto end;
    Glider->FireTimer = 0.1f;
    GameSfx(0x8b, 0);
    NewBuzz(&player->rumble, 3);
    break;

  case 0x18:
    WBScale = kHalfScale;
    WBFirePt = WeatherBossFirePoint;
    NuVecMtxRotate(&WBRot1, SetNuVecPntr(0.0f, 1.0f, 0.0f), &Mat);
    NuVecMtxRotate(&WBRot2, SetNuVecPntr(1.0f, 0.0f, 0.0f), &Mat);
    WBPos = *(struct nuvec_s *)&Glider->Cre->mtxLOCATOR[8][0]._30;
    NuVecScaleAccum((frand() - 0.5f) * 0.5f, &WBPos, &WBRot1);
    NuVecScaleAccum((frand() - 0.5f) * 0.5f, &WBPos, &WBRot2);
    WBFirePt.z -=
        (WBPos.z - WeatherBossFirePoint.z) /
        (WBBULLETSPEED - Level_GliderSpeed) * Level_GliderSpeed;
    NuVecSub(&WBVelDir, &WBFirePt, &WBPos);
    NuVecNorm(&WBVelDir, &WBVelDir);
    NuVecScale(WBBULLETSPEED, &WBVelDir, &WBVelDir);
    NuMtxScale(&Mat, &WBScale);
    if (AddGliderBullet(&Mat, &WBPos, &WBVelDir, 0) == 0) goto end;
    Glider->FireTimer = WBBULLETFIRERATE * 0.01666667f;
    GameSfx(0x55, 0);
    NewBuzz(&player->rumble, 3);
    break;

  case 0x12:
  case 0x24:
    if (Glider->TargetOn == 0) {
      Glider->TargetedTime = 0.0f;
      Glider->TargetTimer = 0.0f;
    } else {
      Glider->TargetedTime += 0.01666667f;
    }

    if (Glider->TargetOn != 0) {
      if (StillLockedOnTarget(Glider->MovingTargetPoint) == 0) {
        MyGameSfx(0x4f, NULL, 0x3fff);
        NewBuzz(&player->rumble, TORPTARGFAILBLEEPTIME);
        Glider->TargetOn = 0;
      }
    }

    if (ProcessTimer(&Glider->TargetTimer) != 0) {
      if (Glider->TargetOn == 0) {
        if (FireButton == 0) goto end;
        if (PickGliderTarget(&Glider->MovingTargetPoint,
                             &Glider->MovingTargetVel,
                             &Glider->TargetMoving) == 0)
          goto end;
        if (Level == 0x12) {
          MyGameSfx(0xbe, NULL, 0x1800);
        }
        NewBuzz(&player->rumble, TORPTARGAQUIREBLEEPTIME);
        Glider->TargetOn = 1;
        Glider->TargetTimer = Level_TargetTime;
        goto end;
      }

      if (FireButton != 0) {
        if (ProcessTimer(&FireBleepTimer) != 0) {
          FireBleepTimer = TARGETBLEEPTIME;
          if (Level == 0x12) {
            MyGameSfx(0xbe, NULL, 0x1800);
            NewBuzz(&player->rumble, TORPTARGBLEEPTIME);
          }
        }
        goto end;
      }

      Glider->TargetOn = 0;
      if (Level == 0x24) {
        NuMtxTranslate(&Mat, &Glider->Position);
        NuVecMtxTransform(&BombPos, SetNuVecPntr(-2.0f, 0.0f, 0.0f), &Mat);
        AddGliderBomb(&BombPos, &Glider->Velocity, Glider->AngleY, 0,
                      Glider->MovingTargetPoint, Glider->MovingTargetVel,
                      Glider->TargetMoving);
        NuVecMtxTransform(&BombPos, SetNuVecPntr(2.0f, 0.0f, 0.0f), &Mat);
        AddGliderBomb(&BombPos, &Glider->Velocity, Glider->AngleY, 0,
                      Glider->MovingTargetPoint, Glider->MovingTargetVel,
                      Glider->TargetMoving);
        Glider->FireTimer = TorpedoTime;
      } else {
        if (AddGliderBomb(
                (struct nuvec_s *)&Glider->Cre->mtxLOCATOR[8][0]._30,
                &Glider->Velocity, Glider->AngleY, 0,
                Glider->MovingTargetPoint, Glider->MovingTargetVel,
                Glider->TargetMoving) == 0)
          goto end;
        Glider->FireTimer = TorpedoTime;
        MyGameSfx(0xba, &Glider->Position, 0x3fff);
      }
      NewBuzz(&player->rumble, 6);
      NewRumble(&player->rumble, 0xb4);
      break;
    }

    if (FireButton == 0) {
      MyGameSfx(0x4f, NULL, 0x3fff);
      Glider->TargetOn = 0;
    }
    FireBleepTimer = TARGETBLEEPTIME;
    goto end;
  }

end:
  LastFireButton = FireButton;
  return;
}

// NGC MATCH
void DrawWeatherBossTarget(void) {
  s32 Obj;
  float Scale;

  if ((WBIntroOn == 0) && (FlyingLevelExtro == 0)) {
    Scale =
        (1.0f - WeatherBossTargetAppearTimer / WeatherBossTargetAppearTime) *
        WBTARGSCALE;
    Obj = 0x5a;
    if (ObjTab[Obj].obj.special != NULL) {
      mTEMP = WeatherBossCamMtx;
      NuVecAdd((struct nuvec_s *)&mTEMP._30, (struct nuvec_s *)&mTEMP._30,
               (struct nuvec_s *)&WeatherBossTargetPos);
      NuMtxPreScale(&mTEMP, SetNuVecPntr(Scale, Scale, 0.05f));
      NuRndrGScnObj((ObjTab[Obj].obj.scene)
                        ->gobjs[(ObjTab[Obj].obj.special)->instance->objid],
                    &mTEMP);
    }
  }
  return;
}

// NGC MATCH
void ProcessWeatherBossTarget(void) {
  float CentreY;
  float RangeY;

  CentreY = WBMAXY - WBMINY;
  RangeY = (WBMAXY + WBMINY) * 0.5f;
  if ((WBIntroOn == 0) && (FlyingLevelExtro == 0)) {
    ProcessTimer(&WeatherBossTargetAppearTimer);
  }
  WeatherBossTargetPos.x = (PlayerGlider.Position.x * WBTARGLIMX) / WBLIMX;
  WeatherBossTargetPos.y =
      ((PlayerGlider.Position.y - RangeY) * WBTARGLIMY) / (CentreY * 0.5f) +
      1.0f;
  return;
}

void ControlGliderWeatherBoss(GLIDERSTRUCT *Glider, struct nupad_s *Pad) {
  float f31;
  float f1;
  float f12;
  float f13;
  struct nuvec_s localVec;
  struct nuvec_s *pos;
  struct nuvec_s *camPos;
  float ratio;

  if (Glider->TerminalDive == 0 && WBIntroOn == 0 && FlyingLevelExtro == 0) {
    if ((Pad->paddata & 0x8) != 0) {
      if ((u32)Glider->BarrelRoll <= 1u) {
        if (Glider->Position.x > -WBLIMX) {
          Glider->BarrelRoll = 3;
          GameSfx(0x5c, &Glider->Position);
          goto skip_input;
        }
      }
    }
    if ((Pad->paddata & 0x4) != 0) {
      if ((u32)(Glider->BarrelRoll + 1) <= 1u) {
        if (Glider->Position.x < WBLIMX) {
          Glider->BarrelRoll = -3;
          GameSfx(0x5c, &Glider->Position);
        }
      }
    }
  }
skip_input:
  if (Glider->BarrelRoll != 0) {
    f31 = Rationalise360f(Glider->TiltZ);
    if (Glider->BarrelRoll == 2) {
      if (Glider->TiltZ > -45.0f) {
        Glider->BarrelRoll = 1;
      }
    }
    if (Glider->BarrelRoll == -2) {
      if (Glider->TiltZ < 45.0f) {
        Glider->BarrelRoll = -1;
      }
    }
    if ((u32)(Glider->BarrelRoll + 1) > 2u || Glider->TiltZ > 30.0f || Glider->TiltZ < -30.0f) {
      SeekHalfLife(&Glider->BarrelDelta, 6.0000005f, 0.05f, 0.016666668f);
    } else if (Glider->TiltZ > 1.0f || Glider->TiltZ < -1.0f) {
      Glider->BarrelDelta = NuFabs(Glider->TiltZ * 20.0f * 0.016666668f * 0.5f);
    } else {
      Glider->BarrelDelta = 1.0f;
    }

    if (Glider->BarrelRoll > 0) {
      f1 = Rationalise360f(f31 + Glider->BarrelDelta);
      Glider->TiltZ = f1;
      if (f31 >= 0.0f) {
        if (f1 < 0.0f) {
          Glider->BarrelRoll = 2;
        }
      }
    } else {
      f1 = Rationalise360f(f31 - Glider->BarrelDelta);
      Glider->TiltZ = f1;
      if (f31 <= 0.0f) {
        if (f1 > 0.0f) {
          Glider->BarrelRoll = -2;
        }
      }
    }

    if (Glider->BarrelRoll == 1) {
      if (f31 < 0.0f) {
        if (Glider->TiltZ >= 0.0f) {
          goto reset;
        }
      }
    }
    if (Glider->BarrelRoll == -1) {
      if (f31 > 0.0f) {
        if (Glider->TiltZ <= 0.0f) {
          goto reset;
        }
      }
    }
    goto done;
reset:
    Glider->BarrelRoll = 0;
    Glider->TiltZ = 0.0f;
  }
done:
  ProcessWeatherBossTarget();
  camPos = (struct nuvec_s *)&WeatherBossCamMtx._30;
  pos = GetWeatherBossPos();
  ratio = (pos->z - camPos->z) / WeatherBossTargetPos.z;
  NuVecScale(ratio, &localVec, &WeatherBossTargetPos);
  NuVecAdd(&WeatherBossFirePoint, &localVec, camPos);
}

void ControlGlider(GLIDERSTRUCT *Glider, struct nupad_s *Pad) {
  float AnX, AnZ;
  float RelX, RelZ, CurrentRadius;
  struct nuvec_s Ctrl;
  struct nuvec_s TempInput;
  float Mag, OldTiltZ, Temp, Delta;
  float MinDestZ, MaxDestZ;
  float f12, f1;
  s32 Turn, Ang;
  struct nuvec_s *ps;

  ps = &Glider->Position;
  RelZ = 0.0f;
  RelX = RelZ;
  CurrentRadius = RelZ;

  if (Level != 0x18) {
    ProcessTimer(&Glider->PullUpTimer);
    RelZ = ps->z - Level_GliderCentreZ;
    RelX = Glider->Position.x - Level_GliderCentreX;
    CurrentRadius = NuFsqrt(RelX * RelX + RelZ * RelZ);

    if (Level != 0x12 && Glider->InTornadoTime == 0.0f && Glider->AutoPilot == 0 &&
        CurrentRadius < Level_GliderRadius && Glider->TerminalDive == 0 &&
        Glider->ForceTurn == 0 && FlyingLevelExtro == 0) {
      if ((Pad->paddata & 0x8) != 0) {
        if ((u32)Glider->BarrelRoll <= 1u) {
          Glider->BarrelRoll = 3;
          GameSfx(0x5c, ps);
          goto skip_input;
        }
      }
      if ((Pad->paddata & 0x4) != 0) {
        if ((u32)(Glider->BarrelRoll + 1) <= 1u) {
          Glider->BarrelRoll = -3;
          GameSfx(0x5c, &Glider->Position);
        }
      }
    }
  }

skip_input:
  AnX = (float)(Pad->l_alg_x - 0x7f);
  AnZ = (float)(Pad->l_alg_y - 0x7f);

  if ((Pad->paddata & 0x4000) != 0) {
    AnZ = 127.0f;
  }
  if ((Pad->paddata & 0x1000) != 0) {
    AnZ = -127.0f;
  }
  if ((Pad->paddata & 0x2000) != 0) {
    AnX = 127.0f;
  }
  if ((Pad->paddata & 0x8000) != 0) {
    AnX = -127.0f;
  }

  if (FlyingLevelExtro || Glider->AutoPilot || Glider->InTornadoTime > 0.0f ||
      (Level == 0x18 && WBIntroOn)) {
    AnZ = 0.0f;
    AnX = AnZ;
  }

  Ctrl.x = AnX;
  Ctrl.y = 0.0f;
  Ctrl.z = -AnZ;
  Mag = NuVecMag(&Ctrl);

  if (Mag >= 32.0f) {
    Ang = NuAtan2D(Ctrl.x, Ctrl.z) & 0xFFFF;
    Temp = (Mag - 32.0f) * 1.02f / 96.0f;
    if (NuFabs(Temp) >= 1.0f) {
      TempInput.z = fsign(Temp);
    } else {
      TempInput.z = Temp;
    }
    TempInput.x = 0.0f;
    TempInput.y = 0.0f;
    NuVecRotateY(&TempInput, &TempInput, Ang);
    Glider->InputX = TempInput.x;
    Glider->InputZ = TempInput.z;
  } else {
    Glider->InputZ = 0.0f;
    Glider->InputX = 0.0f;
  }

  if (Glider->TerminalDive != 0) {
    if (Glider->TerminalDir == 0) {
      Glider->TerminalDir = ((int)(frand() * 16.0) & 2) - 1;
    }
    Glider->InputZ = 0.8f;
    Glider->InputX = (float)Glider->TerminalDir * 0.5f;
  }

  if (Level == 0x18 && WBIntroOn) {
    Glider->DestTiltX = WBIntroDestTiltX;
  } else {
    Glider->DestTiltX = -Glider->InputZ * 40.0f;
  }

  if (Glider->TerminalDive == 0 && WBIntroOn == 0) {
    MinDestZ = -100.0f;
    MaxDestZ = 100.0f;

    if (Glider->PullUpTimer != 0.0f) {
      MinDestZ = Glider->PullUpTimer * 40.0f;
    }

    if (Glider->Position.y < Level_GliderFloor + 1.3f) {
      MinDestZ = (Level_GliderFloor + 1.3f - Glider->Position.y) * 10.0f;
    }

    if (Glider->Position.y > Level_GliderCurrentCeiling - 1.3f) {
      MaxDestZ = (Glider->Position.y - (Level_GliderCurrentCeiling - 1.3f)) * -10.0f;
    } else {
      Level_GliderCurrentCeiling = Level_GliderCeiling;
    }

    if (Glider->DestTiltX > MaxDestZ) {
      Glider->DestTiltX = MaxDestZ;
    }
    if (Glider->DestTiltX < MinDestZ) {
      Glider->DestTiltX = MinDestZ;
    }
  }

  if (Level == 0x1a) {
    if (Glider->DestTiltX == 0.0f) {
      SeekAngHalfLife360f(&Glider->TiltX, Glider->DestTiltX,
                          CRASHTEROIDSSeekTiltXTimeZero, 0.016666668f);
    } else {
      SeekAngHalfLife360f(&Glider->TiltX, Glider->DestTiltX,
                          CRASHTEROIDSSeekTiltXTimeNonZero, 0.016666668f);
    }
  } else if (Level == 0x0d) {
    if (Glider->DestTiltX == 0.0f) {
      SeekAngHalfLife360f(&Glider->TiltX, Glider->DestTiltX,
                          TORNSeekTiltXTimeZero, 0.016666668f);
    } else {
      SeekAngHalfLife360f(&Glider->TiltX, Glider->DestTiltX,
                          TORNSeekTiltXTimeNonZero, 0.016666668f);
    }
  } else {
    SeekAngHalfLife360f(&Glider->TiltX, Glider->DestTiltX, 0.2f, 0.016666668f);
  }

  if (Glider->BarrelRoll != 0) {
    OldTiltZ = Rationalise360f(Glider->TiltZ);
    if (Glider->BarrelRoll == 2) {
      if (Glider->TiltZ > -45.0f) {
        Glider->BarrelRoll = 1;
      }
    }
    if (Glider->BarrelRoll == -2) {
      if (Glider->TiltZ < 45.0f) {
        Glider->BarrelRoll = -1;
      }
    }
    if ((u32)(Glider->BarrelRoll + 1) > 2u || Glider->TiltZ > 30.0f || Glider->TiltZ < -30.0f) {
      SeekHalfLife(&Glider->BarrelDelta, 6.0000005f, 0.05f, 0.016666668f);
    } else if (Glider->TiltZ > 1.0f || Glider->TiltZ < -1.0f) {
      Glider->BarrelDelta = NuFabs(Glider->TiltZ * 20.0f * 0.016666668f * 0.5f);
    } else {
      Glider->BarrelDelta = 1.0f;
    }

    if (Glider->BarrelRoll > 0) {
      f1 = Rationalise360f(OldTiltZ + Glider->BarrelDelta);
      Glider->TiltZ = f1;
      if (OldTiltZ >= 0.0f) {
        if (f1 < 0.0f) {
          Glider->BarrelRoll = 2;
        }
      }
    } else {
      f1 = Rationalise360f(OldTiltZ - Glider->BarrelDelta);
      Glider->TiltZ = f1;
      if (OldTiltZ <= 0.0f) {
        if (f1 > 0.0f) {
          Glider->BarrelRoll = -2;
        }
      }
    }

    if (Glider->BarrelRoll == 1) {
      if (OldTiltZ < 0.0f) {
        if (Glider->TiltZ >= 0.0f) {
          goto reset;
        }
      }
    }
    if (Glider->BarrelRoll == -1) {
      if (OldTiltZ > 0.0f) {
        if (Glider->TiltZ <= 0.0f) {
          goto reset;
        }
      }
    }
    goto done;
  reset:
    Glider->BarrelRoll = 0;
    Glider->TiltZ = 0.0f;
  done:
    if (Glider->BarrelRoll != 0) {
      return;
    }
  }

  Turn = 0;

  if (Glider->OverideTiltZ == 0.0f) {
    Glider->DestTiltZ = Glider->InputX * 60.0f;
  }

  if (Level == 0x18) {
    if (WBIntroOn) {
      Glider->DestTiltZ = WBIntroDestTiltZ;
    }
  }
  if (Level == 0x18) {
    Delta = Rationalise360f(Glider->RailAngle - Glider->AngleY);
    if (NuFabs(Glider->Position.x) > 10.0f) {
      f12 = Glider->Position.x;
      if (f12 > 10.0f) {
        if (Delta > 0.0f) {
          Turn = 1;
        }
      }
      if (f12 < -10.0f) {
        if (Delta < 0.0f) {
          Turn = 1;
        }
      }
    }
  } else {
    if (CurrentRadius > Level_GliderRadius || Glider->ForceTurn != 0) {
      Turn = 1;
      Delta = Rationalise360f(
          (float)NuAtan2D(RelX, RelZ) / 182.0444f - Glider->AngleY);
    } else {
      Delta = 0.0f;
    }
  }

  if (Turn != 0) {
    MinDestZ = -1000.0f;
    MaxDestZ = 1000.0f;

    if (NuFabs(Delta) < 2.0f) {
      Glider->ForceTurn = 0;
    }

    if (Delta > 90.0f) {
      MinDestZ = 60.0f;
    } else if (Delta > 0.0f) {
      MinDestZ = Delta / 1.5f;
    } else if (Delta < -90.0f) {
      MaxDestZ = -60.0f;
    } else {
      MaxDestZ = Delta / 1.5f;
    }

    if (Glider->DestTiltZ > MaxDestZ) {
      Glider->DestTiltZ = MaxDestZ;
    }
    if (Glider->DestTiltZ < MinDestZ) {
      Glider->DestTiltZ = MinDestZ;
    }
  }

  if (Glider->OverideTiltZ != 0.0f) {
    SeekAngHalfLife360f(&Glider->TiltZ, Glider->DestTiltZ, SeekTiltZOveride,
                        0.016666668f);
  } else if (Glider->DestTiltZ == 0.0f) {
    SeekAngHalfLife360f(&Glider->TiltZ, Glider->DestTiltZ, SeekTiltZZero,
                        0.016666668f);
  } else {
    SeekAngHalfLife360f(&Glider->TiltZ, Glider->DestTiltZ, SeekTiltZNonZero,
                        0.016666668f);
  }
}

void ProcessGliderMovement(GLIDERSTRUCT *Glider, float DeltaTime) {
  struct numtx_s In;
  struct numtx_s Out;
  struct nuvec_s Resolved;
  struct nuvec_s Temp;
  struct nuvec_s KeepVel;
  struct nuvec_s Target;
  float Scale;
  float Dest;

  ProcessTimer(&Glider->InTornadoTime);

  if (Glider->InTornado != 0 && Glider->LastInTornado == 0) {
    float Scale;

    Scale = frand() * 360.0f;
    KeepVel.x = 0.0f;
    KeepVel.y = frand() * 25.0f - 12.5f;
    KeepVel.z = frand() * 15.0f + 45.0f;

    Temp = KeepVel;

    if (Level != 26) {
      NuVecRotateY(&Temp, &Temp, (s32)(Scale * 182.04445f));
      NuVecScale(Glider->InTornadoScale, &Temp, &Temp);
      Glider->Velocity = Temp;
    }

    Glider->TornadoSpin =
        (frand() * 1200.0f - 600.0f) * Glider->InTornadoScale;
    GameSfx(0x5c, 0);

    if (Level != 26 && InvincibilityCHEAT == 0 && VehicleLevelImmune == 0) {
      if (Glider->InTornadoTime <= 0.0f) {
        Glider->HitPoints -= (s32)(Glider->InTornadoScale * 12.5f);
      }
      if (Glider->HitPoints < 0) {
        Glider->HitPoints = 0;
      }
    }

    NewBuzz(&player->rumble, BEENHITTORNBUZZTIME);
    NewRumble(&player->rumble, BEENHITTORNRUMTIME);

    Glider->InTornadoTime = 2.0f;
    Glider->CamTornRecoverTimer = 1.0f;
  }

  if (Glider->InTornadoTime != 0.0f) {
    Glider->AngleY = Glider->TornadoSpin * 0.016666668f *
                         Glider->InTornadoTime +
                     Glider->AngleY;
  } else if (Glider->BarrelRoll == 0) {
    if (Level == 24) {
      Glider->AngleY = 0.0f;
    } else {
      Glider->AngleY += Glider->TiltZ * 2.0f * DeltaTime;
    }
  }

  if (Level == 26) {
    NuMtxSetRotationY(&In, (int)(-Glider->AngleY * 182.04445f));
    NuMtxRotateX(&In, (int)(-Glider->TiltX * 2.0f * 182.04445f));
    NuMtxRotateZ(&In, (int)(-Glider->TiltZ * 182.04445f));

    NuMtxSetRotationZ(&Out, (int)(Glider->TiltZ * 182.04445f));
    NuMtxRotateX(&Out, (int)(Glider->TiltX * 2.0f * 182.04445f));
    NuMtxRotateY(&Out, (int)(Glider->AngleY * 182.04445f));
  } else {
    NuMtxSetRotationY(&In, (int)(-Glider->AngleY * 182.04445f));
    NuMtxRotateX(&In, (int)(-Glider->TiltX * 182.04445f));
    NuMtxRotateZ(&In, (int)(-Glider->TiltZ * 182.04445f));

    NuMtxSetRotationZ(&Out, (int)(Glider->TiltZ * 182.04445f));
    NuMtxRotateX(&Out, (int)(Glider->TiltX * 182.04445f));
    NuMtxRotateY(&Out, (int)(Glider->AngleY * 182.04445f));
  }

  ProcessTimer(&Glider->FixVelTimer);

  KeepVel = Glider->Velocity;

  if (Glider->FixVelTimer > FIXVELFADETIME) {
    Scale = 1.0f;
  } else {
    Scale = Glider->FixVelTimer / FIXVELFADETIME;
  }

  NuVecMtxRotate(&Resolved, &Glider->Velocity, &In);

  {
    float Scale;
    Scale = 1.0f - Glider->TiltX * 0.01f;

    if (Glider->BarrelRoll != 0) {
      Glider->TargetSpeed = -Level_GliderSpeed * 0.125f;
    } else {
      Glider->TargetSpeed = -Level_GliderSpeed;
    }

    SeekHalfLife(&Glider->Speed, Glider->TargetSpeed, 1.0f, 0.016666668f);

    Target.z = Glider->Speed * Scale;
    Target.x = 0.0f;
    Target.y = 0.0f;
  }

  if (Level == 24) {
    SeekHalfLife(&Resolved.x, 0.0f, 0.2f, DeltaTime);
    SeekHalfLife(&Resolved.y, 0.0f, 0.1f, DeltaTime);
    SeekHalfLife(&Resolved.z, Target.z, 0.5f, DeltaTime);
  } else {
    SeekHalfLife(&Resolved.x, 0.0f, WBFRICX, DeltaTime);
    SeekHalfLife(&Resolved.y, 0.0f, WBFRICY, DeltaTime);
    SeekHalfLife(&Resolved.z, Target.z, 0.5f, DeltaTime);
  }

  NuVecMtxRotate(&Glider->Velocity, &Resolved, &Out);

  NuVecScale(1.0f - Scale, &Glider->Velocity, &Glider->Velocity);
  NuVecScaleAccum(Scale, &Glider->Velocity, &KeepVel);

  if (Glider->AutoPilot == 0) {
    Glider->Position.x =
        Glider->Velocity.x * DeltaTime + Glider->Position.x;
    Glider->Position.y =
        Glider->Velocity.y * DeltaTime + Glider->Position.y;
    Glider->Position.z =
        Glider->Velocity.z * DeltaTime + Glider->Position.z;
  }

  if (Glider->BarrelRoll > 1) {
    Dest = -10.0f;
  } else if (Glider->BarrelRoll <= -2) {
    Dest = 10.0f;
  } else {
    Dest = 0.0f;
  }

  if (Dest == 0.0f) {
    SeekHalfLife(&Glider->BarrelSpeedX, Dest, 0.5f, DeltaTime);
  } else {
    SeekHalfLife(&Glider->BarrelSpeedX, Dest, 0.3f, DeltaTime);
  }

  Target = SetNuVec(Glider->BarrelSpeedX, 0.0f, 0.0f);
  NuVecRotateY(&Target, &Target, (s32)(Glider->AngleY * 182.04445f));

  Glider->Position.x = Target.x * DeltaTime + Glider->Position.x;
  Glider->Position.y = Target.y * DeltaTime + Glider->Position.y;
  Glider->Position.z = Target.z * DeltaTime + Glider->Position.z;
}

// NGC MATCH
static char GrabAnotherLocator(char *List, struct NUPOINTOFINTEREST_s **PList) {
  s32 i;
  s32 j;
  s32 k;

  j = (s32)(frand() * 16.0f);
  for (i = 0; i < 0x10; i++, j++) {
    if (0xf < j) {
      j -= 0x10;
    }
    if (PList[j] != NULL) {
      for (k = 0; k < 0x10; k++) {
        if ((List[k] == 0x7f) || (List[k] == j))
          break;
      }
      if ((k == 0x10) || (List[k] == 0x7f)) {
        return (char)j;
      }
    }
  }
  return 0x7f;
}

void GliderSmoke(GLIDERSTRUCT *Glider) {
  float Time;
  float Val;
  float ThisVal;
  float MaxTime;
  int i;
  struct CharacterModel *Model;

  Val = 0.0f;
  i = CRemap[0x36];
  if (i == -1) return;

  Model = &CModel[i];
  i = 0;

  if (Glider->TerminalDive != 0) {
    Glider->HitTimer = Level_DeadTime;
  }

  Time = Glider->HitTimer;
  MaxTime = Level_DeadTime - 3.0f;

  if (Time > 0.0f && Val <= MaxTime) {
    do {
      ThisVal = Time;
      if (Time > 3.0f) {
        ThisVal = 3.0f;
      }

      if (Glider->LocatorList[i] == 0x7f) {
        Glider->LocatorList[i] = GrabAnotherLocator(Glider->LocatorList, Model->pLOCATOR);
        if ((s8)Glider->LocatorList[i] == 0x7f) goto skip;
      }

      Glider->LocatorTime[i] += ThisVal;
      if (Glider->LocatorTime[i] >= 3.0f) {
        Glider->LocatorTime[i] -= 3.0f;
        AddGameDebrisRot(0x1d,
            (struct nuvec_s *)((char *)Glider->Cre + Glider->LocatorList[i] * 64 + 0x6A4),
            1, 0x4000, 0);
      }

      skip:
      Time -= 0.5f;
      Val += 0.5f;
      i++;
      if (Time <= 0.0f) break;
    } while (Val <= MaxTime);
  }

  while (i <= 15) {
    Glider->LocatorTime[i] = 0.0f;
    Glider->LocatorList[i] = 0x7f;
    i++;
  }
}

void AddGliderHitPoints(s32 points) {
  GLIDERSTRUCT *Glider = (GLIDERSTRUCT *)player->Buggy;
  Glider->HitPoints += 25;
  if (Glider->HitPoints > 100) {
    Glider->HitPoints = 100;
  }
}

// NGC MATCH
void ExplodeGlider(GLIDERSTRUCT *Glider) {
  NewBuzz(&player->rumble, BEENHITBIGBUZZTIME);
  NewRumble(&player->rumble, BEENHITBIGRUMTIME);
  if (Level == 0x1a) {
    MyGameSfx(0xb7, NULL, 0x3fff);
  } else {
    MyGameSfx(0x56, NULL, 0x3fff);
  }
  if (InvincibilityCHEAT != 0) {
    return;
  }
  Glider->Dead = 1;
  FarmResetTimer = 0x3c;
  if (TimeTrial != 0) {
    new_mode = GameMode;
    return;
  }
  if (plr_lives.count != 0) {
    plr_lives.count--;
    if (Adventure != 0) {
      LivesLost++;
      Game.lives = plr_lives.count;
    }
    new_mode = GameMode;
    return;
  }
  new_level = 0x26;
  return;
}

// NGC MATCH
void DeadGliderWB(GLIDERSTRUCT *Glider) {
  if (Glider->Dead == 0) {
    ExplodeGlider(Glider);
  }
  return;
}

// NGC MATCH
void DeadGliderCoco(GLIDERSTRUCT *Glider) {
  if (Glider->CocoDead == 0) {
    Glider->CocoDead = 1;
    Glider->CocoDeadTimer = 3.0f;
    Glider->Velocity.x = frand() - 0.5f;
    Glider->Velocity.y = (frand() - 0.5f);
    Glider->Velocity.z = (frand() - 0.5f);
    Glider->CocoDeathSpinX = (frand() - 0.5f) * 360.0f;
    Glider->CocoDeathSpinZ = (frand() - 0.5f) * 360.0f;
    NuVecNorm(&Glider->Velocity, &Glider->Velocity);
    NuVecScale(2.0f, &Glider->Velocity, &Glider->Velocity);
    AddGameDebris(0x18, &Glider->Position);
  }
  Glider->HitPoints = 0;
  NuVecScaleAccum(0.01666667f, &Glider->Position, &Glider->Velocity);
  Glider->TiltX = Glider->CocoDeathSpinX * 0.01666667f + Glider->TiltX;
  Glider->TiltZ = Glider->CocoDeathSpinZ * 0.01666667f + Glider->TiltZ;
  if (ProcessTimer(&Glider->CocoDeadTimer) != 0) {
    Glider->CocoDeadTimer = 1000000.0f;
    ExplodeGlider(Glider);
  }
  return;
}

struct nuvec_s lbl_8011833C = {0.0f, 0.0f, 1.0f};

// NGC MATCH
static s32 SafePosition(struct nuvec_s *Pos) {
  struct nuvec_s Rel;
  struct nuvec_s Temp;
  struct nuvec_s local_18;

  NuVecSub(&Rel, Pos, (struct nuvec_s *)&GameCam[0].m._30);
  if (NuVecMag(&Rel) < 10.0f) {
    return 0;
  }
  local_18 = lbl_8011833C;
  NuVecMtxRotate(&local_18, &local_18, &GameCam[0].m);
  if (NuVecDot(&Rel, &Temp) < 0.0f) {
    return 0;
  }
  return SafeFromCollisions(Pos);
}

// NGC MATCH
static ZOFFASTRUCT *FindFreeZoffa(void) {
  s32 i;

  for (i = 0; i < 4; i++) {
    if (EnemyZoffa[i].ActiveMode == 0) {
      memset(&EnemyZoffa[i], 0, 0x5b0);
      return &EnemyZoffa[i];
    }
  }
  return NULL;
}

// NGC MATCH
static ZOFFASTRUCT *InitZoffa(ZOFFASTRUCT *Zoffa, ZOFFASTART *Start) {
  s32 Indx;
  s32 Id;

  if (Zoffa == NULL) {
    Zoffa = FindFreeZoffa();
    if (Zoffa == NULL) {
      return NULL;
    }
  }

  Indx = Zoffa - EnemyZoffa;
  memset(Zoffa, 0, 0x5b0);

  Zoffa->Position.x = Start->x;
  Zoffa->Position.y = Start->y;
  Zoffa->Position.z = Start->z;
  Zoffa->AngleY = Start->Angle;
  Zoffa->ActiveMode = 1;
  Zoffa->DestAngleY = Start->Angle;
  Zoffa->NewAltitudeTarget = -100.0f;
  Zoffa->HitPoints = 2;
  Zoffa->RespawnTimer = 10.0f;

  if (Level == 0xd) {
    switch (Indx) {
      case 0: Id = 0x5b; break;
      case 1: Id = 0x5e; break;
      case 2: Id = 0x5d; break;
      default: Id = 0x5c; break;
    }
  } else if (Level == 0x12) {
    Id = 0x5f;
  } else {
    switch (Indx) {
      case 0: Id = 0x82; break;
      case 1: Id = 0x83; break;
      case 2: Id = 0x84; break;
      default: Id = 0x85; break;
    }
  }

  if (MyInitModelNew(&Zoffa->MainDraw, Id, 0x1d, 0, NULL, &Zoffa->Position) == 0) {
    return NULL;
  }
  return Zoffa;
}


// NGC MATCH
static void InitZoffaUFOs(void) {
  s32 i;

  memset(EnemyZoffa, 0, 0x16c0);
  ZoffaTeleportIndx = 0;
  for (i = 0; i < 4; i++) {
    InitZoffa(NULL, &ZoffaStartPoints[i]);
  }
  CurrentAggressor = 0;
  ZoffaCollisionCounter = 0;
  return;
}

// NGC MATCH
void DrawZoffa(ZOFFASTRUCT *Zoffa) {
  NuMtxSetRotationZ(&mTEMP, (s32)(Zoffa->TiltZ * 182.0444f));
  NuMtxRotateX(&mTEMP, (s32)(-Zoffa->TiltX * 182.0444f));
  NuMtxRotateY(&mTEMP, (s32)((Zoffa->AngleY + 180.0f) * 182.0444f));
  NuMtxTranslate(&mTEMP, &Zoffa->Position);
  Zoffa->Seen = MyDrawModelNew(&Zoffa->MainDraw, &mTEMP, Zoffa->Locators);
  return;
}

// NGC MATCH
void DrawZoffaUFOs(void) {
  s32 i;

  for (i = 0; i < 4; i++) {
    if (EnemyZoffa[i].ActiveMode != 0) {
      DrawZoffa(&EnemyZoffa[i]);
    }
  }
  return;
}

// NGC MATCH
static ZOFFASTART *FindZoffaRestartPoint(void) {
  ZOFFASTART *ret;
  s32 j;

  j = (s32)(frand() * 16.0f);
  while (j > 0x10) {
    j -= 4;
  }
  while (j < 0) {
    j += 0x10;
  }
  ret = &ZoffaStartPoints[j];
  if (SafePosition((struct nuvec_s *)ret) != 0) {
    return ret;
  }
  return NULL;
}

// NGC MATCH
static void ZoffaUFORespawn(void) {
  s32 i;
  s32 NumActive;
  ZOFFASTART *Start;

  NumActive = 0;
  for (i = 0; i < 4; i++) {
    if (EnemyZoffa[i].ActiveMode != 0) {
      NumActive++;
    }
  }
  if (NumActive < 4) {
    for (i = 0; i < 4; i++) {
      if ((EnemyZoffa[i].ActiveMode == 0) &&
          (EnemyZoffa[i].RespawnTimer == 0.0f)) {
        Start = (ZOFFASTART *)FindZoffaRestartPoint();
        if (Start == NULL) {
          return;
        }
        InitZoffa(&EnemyZoffa[i], Start);
        return;
      }
    }
  }
  return;
}

// NGC MATCH
static void ZoffaSmoke(ZOFFASTRUCT *Zoffa) {
  struct nuvec_s Temp;

  NuMtxSetRotationZ(&mTEMP, (int)(Zoffa->TiltZ * 182.04445f));
  NuMtxRotateX(&mTEMP, (int)(-Zoffa->TiltX * 182.04445f));
  NuMtxRotateY(&mTEMP, (int)((Zoffa->AngleY + 180.0f) * 182.04445f));
  NuMtxTranslate(&mTEMP, &Zoffa->Position);
  if (Zoffa->TerminalDive != 0) {
    if (Zoffa->SmokeCounter != 0) {
      vec = SetNuVecPntr(-1.0f, 0.2f, 1.0f);
      NuVecMtxTransform(&Temp, vec, &mTEMP);
      AddGameDebrisRot(0x1d, &Temp, 1, 0, 0);
      vec = SetNuVecPntr(0.0f, 1.2f, 1.0f);
      NuVecMtxTransform(&Temp, vec, &mTEMP);
      AddGameDebrisRot(0x1d, &Temp, 1, 0, 0);
      Zoffa->SmokeCounter = 0;
    } else {
      NuVecMtxTransform(&Temp, SetNuVecPntr(1.0f, 0.2f, 1.0f), &mTEMP);
      AddGameDebrisRot(0x1d, &Temp, 1, 0, 0);
      NuVecMtxTransform(&Temp, SetNuVecPntr(0.0f, -0.8f, 1.0f), &mTEMP);
      AddGameDebrisRot(0x1d, &Temp, 1, 0, 0);
      Zoffa->SmokeCounter = 1;
    }
    NuVecMtxTransform(&Temp, SetNuVecPntr(0.0f, 0.2f, 1.0f), &mTEMP);
    AddGameDebrisRot(0x45, &Temp, 1, 0, 0);
  } else {
    if (((Zoffa->HitPoints < 2) && (Zoffa->SmkTimer--, Zoffa->SmkTimer < 1)) &&
        (Level != 0x1a)) {
      NuVecMtxTransform(&Temp, SetNuVecPntr(0.0f, 0.2f, 1.0f), &mTEMP);
      AddGameDebrisRot(0x1d, &Temp, 1, 0, 0);
      Zoffa->SmkTimer = Zoffa->HitPoints;
    }
  }
  return;
}

// NGC MATCH
void TeleportManager(void) {
  struct nuvec_s Point;
  s32 Indx;
  s32 i;
  s32 j;
  struct ZOFFASTART Start;

  for (i = 0; i < 4; i++) {
    if ((EnemyZoffa[i].ActiveMode != 0) && (EnemyZoffa[i].Teleport != 0)) {
      for (j = 0; j < 2; j++) {
        Indx = j + ZoffaTeleportIndx & 3;
        NuVecMtxTransform(&Point, &TeleportPos[Indx], &GameCam[0].m);
        if (Point.y < Level_GliderFloor) {
          Point.y = Level_GliderFloor;
        }
        if (SafeFromCollisions(&Point) != 0) {
          Start.x = Point.x;
          Start.y = Point.y;
          Start.z = Point.z;
          switch (Indx) {
          case 0:
            Start.Angle = (PlayerGlider.AngleY - 135.0f);
            break;
          case 1:
            Start.Angle = (PlayerGlider.AngleY + 135.0f);
            break;
          case 2:
            Start.Angle = (PlayerGlider.AngleY - 90.0f);
            break;
          case 3:
            Start.Angle = (PlayerGlider.AngleY + 90.0f);
            break;
          }
          InitZoffa(&EnemyZoffa[i], &Start);
          EnemyZoffa[i].NoTeleportTimer = 3.0f;
          EnemyZoffa[i].KeepSameVelocityTimer = 0.8f;
          EnemyZoffa[i].Speed = 14.0f;
          NuVecMtxRotate(&EnemyZoffa[i].Velocity,
                         &TeleportVel[(j + ZoffaTeleportIndx & 3)],
                         &GameCam[0].m);
          ZoffaTeleportIndx = ZoffaTeleportIndx + 1 & 3;
          break;
        }
      }
    }
  }
}

// NGC MATCH
float GetZoffaBestTarget(float Best, struct nuvec_s **TargetPos,
                         struct nuvec_s **Vel, s32 *Moving) {
  struct nuvec_s CamDir;
  struct nuvec_s Rel;
  s32 i;
  float Dot;
  float Mag;

  NuVecMtxRotate(&CamDir, SetNuVecPntr(0.0f, 0.0f, 1.0f), &GameCam[0].m);
  for (i = 0; i < 4; i++) {
    if (EnemyZoffa[i].ActiveMode != 0) {
      NuVecSub(&Rel, &EnemyZoffa[i].Position, &GameCam[0].pos);
      Mag = NuVecMag(&Rel);
      if (Mag > 10.0f) {
        NuVecScale((1.0f / Mag), &Rel, &Rel);
        Dot = DotProduct(&CamDir, &Rel);
        if (Dot > Best) {
          *TargetPos = &EnemyZoffa[i].Position;
          *Vel = &EnemyZoffa[i].Velocity;
          *Moving = 1;
          Best = Dot;
        }
      }
    }
  }
  return Best;
}

// NGC MATCH
void MoveZoffaUFO(ZOFFASTRUCT *Zoffa) {
  struct nuvec_s delta;
  float one = 1.0f;
  float dist;
  float diff;
  float seekAng;
  float angDiff;
  struct nuvec_s Target;
  struct nuvec_s Rel;
  struct nuvec_s TempVel;
  struct numtx_s Mat;
  s32 Mode;
  s32 Aggressive;
  float Speed;
  float TargetX;
  float TargetZ;

  if (Zoffa->ActiveMode == 0) return;

  ProcessTimer(&Zoffa->NoTeleportTimer);
  ProcessTimer(&Zoffa->KeepSameVelocityTimer);
  ProcessTimer(&Zoffa->RespawnTimer);

  ZoffaSmoke(Zoffa);
  MyAnimateModelNew(&Zoffa->MainDraw, one);

  if (Zoffa->TerminalDive != 0) {
    Zoffa->Position.x += Zoffa->Velocity.x * 0.01666667f;
    Zoffa->Position.y += Zoffa->Velocity.y * 0.01666667f;
    Zoffa->Position.z += Zoffa->Velocity.z * 0.01666667f;
    Zoffa->Velocity.y -= 0.5f;

    diff = Rationalise360f(Zoffa->DestAngleY - Zoffa->AngleY);
    {
      s32 spin = 1;
      if (diff < 0.0f) spin = -1;
      Zoffa->TiltZ += (float)spin * 6.0f * 0.01666667f;
    }

    Zoffa->AngleY += Zoffa->TiltZ * 2.0f * 0.01666667f;

    if (Zoffa->Position.y < Level_GliderFloor - 10.0f) {
      AddGameDebris(0x18, &Zoffa->Position);
      MyGameSfx(0xb4, &Zoffa->Position, 0x3fff);
      Zoffa->ActiveMode = 0;
      Zoffa->RespawnTimer = 5.0f;
      AddScreenWumpa(Zoffa->Position.x, Zoffa->Position.y, Zoffa->Position.z, one);
      ClockOff();
    }
    return;
  }

  CollideGliderBullets(&Zoffa->Position, 3.0f, 1, one, 0, 0);
  if (CollideGliderBullets(&Zoffa->Position, 3.0f, 0, one, 0, 0) != 0) {
    Zoffa->HitPoints--;
    AddScreenWumpa(Zoffa->Position.x, Zoffa->Position.y, Zoffa->Position.z, one);
    ClockOff();
    if (Zoffa->HitPoints <= 0) {
      Zoffa->TerminalDive = 1;
      Zoffa->DestAngleY = Zoffa->AngleY + 90.0f;
      MyGameSfx(0xb4, &Zoffa->Position, 0x7fff);
    } else {
      MyGameSfx(0xbb, &Zoffa->Position, 0x3fff);
    }
  }

  Mode = Zoffa->ActiveMode;
  Aggressive = (Zoffa == &EnemyZoffa[CurrentAggressor]);

  NuVecSub(&Rel, &PlayerGlider.Position, &Zoffa->Position);
  dist = NuVecMag(&Rel);

  switch (Mode) {
  case 0: // Approach
    if (dist < 40.0f) {
      Zoffa->ActiveMode = 1;
    }

    Target = PlayerGlider.Position;
    break;

  case 1: // Circle
    if (dist > 80.0f) {
      Zoffa->ActiveMode = 0;
    }
    if (Aggressive && dist < 60.0f) {
      Zoffa->ActiveMode = 2;
    }

    {
      float circleAngle;
      circleAngle = Zoffa->VisibleTimer;
      circleAngle += 1.0f * 0.01666667f;
      Zoffa->VisibleTimer = circleAngle;

      Target.x = PlayerGlider.Position.x + NuFsin(circleAngle) * 50.0f;
      Target.y = PlayerGlider.Position.y;
      Target.z = PlayerGlider.Position.z + NuFcos(circleAngle) * 50.0f;
    }
    break;

  case 2: // Attack
    if (dist > 100.0f) {
      Zoffa->ActiveMode = 1;
    }

    Target = PlayerGlider.Position;
    NuVecScaleAccum(0.5f, &Target, &PlayerGlider.Velocity);

    if (ProcessTimer(&Zoffa->FireTimer) != 0) {
      if (Zoffa->Seen != 0) {
        struct nuvec_s FireDir;
        NuVecSub(&FireDir, &PlayerGlider.Position, &Zoffa->Position);
        NuVecNorm(&FireDir, &FireDir);
        NuVecScale(-120.0f, &FireDir, &FireDir);

        NuMtxSetRotationZ(&Mat, (s32)(Zoffa->TiltZ * 182.04445f));
        NuMtxRotateX(&Mat, (s32)(-Zoffa->TiltX * 182.04445f));
        NuMtxRotateY(&Mat, (s32)((Zoffa->AngleY + 180.0f) * 182.04445f));

        AddGliderBullet(&Mat, &Zoffa->Position, &FireDir, 1);
        MyGameSfx(0x60, &Zoffa->Position, 0x3fff);
      }
      Zoffa->FireTimer = 2.0f;
    }
    break;

  case 3: // Flee/teleport
    Zoffa->Teleport = 1;
    break;
  }

  if (Zoffa->TerminalDive == 0) {
    SeekHalfLife(&Zoffa->Speed, 14.0f, 0.5f, 0.01666667f);

    if (Zoffa->KeepSameVelocityTimer <= 0.0f) {
      NuVecSub(&Rel, &Target, &Zoffa->Position);
      dist = NuVecMag(&Rel);
      if (dist > 1.0f) {
        NuVecScale(one / dist, &Rel, &Rel);

        angDiff = Rationalise360f(
            NuAtan2D(Rel.x, Rel.z) / 182.04445f - Zoffa->AngleY);

        if (angDiff > 3.0f) {
          seekAng = 3.0f;
        } else if (angDiff < -3.0f) {
          seekAng = -3.0f;
        } else {
          seekAng = angDiff;
        }

        Zoffa->AngleY += seekAng * 0.01666667f;
      }

      NuVecRotateY(&TempVel, SetNuVecPntr(0.0f, 0.0f, -Zoffa->Speed), (s32)(Zoffa->AngleY * 182.04445f));
      Zoffa->Velocity = TempVel;
    }

    NuVecScaleAccum(0.01666667f, &Zoffa->Position, &Zoffa->Velocity);

    {
      float targetY;
      targetY = PlayerGlider.Position.y;
      SeekHalfLife(&Zoffa->Position.y, targetY, 0.5f, 0.01666667f);
    }

    if (Zoffa->Position.y < Level_GliderFloor) {
      Zoffa->Position.y = Level_GliderFloor;
    }

    {
      float targetTiltX;
      targetTiltX = 0.0f;
      SeekAngHalfLife360f(&Zoffa->TiltX, targetTiltX, 0.5f, 0.01666667f);
      SeekAngHalfLife360f(&Zoffa->TiltZ, 0.0f, 0.5f, 0.01666667f);
    }
  }

  if (Level != 0x1a) {
    MyGameSfxLoop(0xbc, &Zoffa->Position, 0x3fff);
  }
}


// NGC MATCH
HOVASTRUCT *FindFreeHova(void) {
  s32 i;

  for (i = 0; i < 6; i++) {
    if (HovaBlimp[i].ActiveMode == 0) {
      return &HovaBlimp[i];
    }
  }
  return NULL;
}

// NGC MATCH
void InitHova(struct nuvec_s *Pos, float AngleY) {
  HOVASTRUCT *Hova;
  TORNADOSTRUCT *Tornado;
  struct nuvec_s Vel;
  s32 HovaIndx;

  Hova = FindFreeHova();
  if (Hova == NULL) return;

  Hova->ActiveMode = 1;
  Hova->Position = *Pos;
  Hova->StartPos = *Pos;

  Hova->Position.y += frand() - 0.5f;

  Vel = SetNuVec(0.0f, (frand() - 0.5f) * 5.0f, 0.5f);

  HovaIndx = Hova - HovaBlimp;
  Hova->Velocity = Vel;
  Hova->ThrustOff = 0.1f;
  Hova->AngleY = AngleY;

  Tornado = &TornadoList[HovaIndx];
  Hova->HitPoints = 10;
  Tornado->Active = 1;

  Tornado->Position = Hova->Position;
  Tornado->Position.y = 27.0f;
  Tornado->StartPosition = Tornado->Position;

  Tornado->YAng = frand() * 360.0f;

  Tornado->Scale = 2.0f;
  Tornado->YAngInc = frand() * 40.0f + 160.0f;

  MyInitModelNew(&Hova->MainDraw, 0x43, 0x46, 0, NULL, &Hova->Position);
}


// NGC MATCH
void InitialiseHovaBlimps(void) {
  s32 i;

  memset(HovaBlimp, 0, 0x738);
  memset(TornadoList, 0, 0x660);
  InitHova(SetNuVecPntr(0.0f, 50.0f, 0.0f), 0.0f);
  InitHova(SetNuVecPntr(0.0f, 50.0f, 160.0f), 0.0f);
  InitHova(SetNuVecPntr(152.0f, 50.0f, 49.0f), 0.0f);
  InitHova(SetNuVecPntr(94.0f, 50.0f, -129.0f), 0.0f);
  InitHova(SetNuVecPntr(-94.0f, 50.0f, -129.0f), 0.0f);
  InitHova(SetNuVecPntr(-152.0f, 50.0f, 49.0f), 0.0f);
  for (i = 0; i < 0x14; i++) {
    if (jontorn[i] != -1) {
      if (jonfirst != 0) {
        DebFree(&jontorn[i]);
      }
    }
    jontorn[i] = -1;
  }
  jonfirst = 1;
}

// NGC MATCH
static void ProcessTornado(TORNADOSTRUCT *Tornado, HOVASTRUCT *Hova) {
  Tornado->YAng = Tornado->YAngInc * 0.01666667f + Tornado->YAng;
  Tornado->YAng = Rationalise360f(Tornado->YAng);
  if (Hova->ActiveMode != 0) {
    Tornado->Position = Hova->Position;
    Tornado->Position.y = 27.0f;
    Tornado->StartPosition = Tornado->Position;
  } else {
    Tornado->Active = 0;
  }
  return;
}

static struct nuvec_s lbl_801184B0 = {6.0f, 6.0f, 6.0f};

// NGC MATCH
void DrawHovaBlimp(HOVASTRUCT *Hova) {
  struct nuvec_s Scale;

  Scale = lbl_801184B0;
  NuMtxSetScale(&mTEMP, &Scale);
  NuMtxRotateY(&mTEMP, (int)((Hova->AngleY + 180.0f) * 182.0444f));
  NuMtxTranslate(&mTEMP, &Hova->Position);
  MyDrawModelNew(&Hova->MainDraw, &mTEMP, NULL);
  return;
}

// NGC MATCH
void ProcessHovaBlimp(HOVASTRUCT *Hova) {
  struct nuvec_s newPos;
  struct nuvec_s temp1;
  struct nuvec_s temp2;
  float maxDist2;
  s32 i;

  Hova->MainDraw.Anim.oldaction = Hova->MainDraw.Anim.action;
  MyAnimateModelNew(&Hova->MainDraw, 0.5f);
  SeekHalfLifeNUVEC(&Hova->Velocity, &Hova->TargetVelocity, 1.0f, 1.0f / 60.0f);

  newPos = Hova->Position;
  NuVecScaleAccum(1.0f / 60.0f, &newPos, &Hova->Velocity);

  maxDist2 = Level_GliderRadius - 15.0f;
  maxDist2 = maxDist2 * maxDist2;

  if (newPos.x * newPos.x + newPos.z * newPos.z > maxDist2 &&
      newPos.x * newPos.x + newPos.z * newPos.z >= Hova->Position.x * Hova->Position.x + Hova->Position.z * Hova->Position.z) {
    Hova->MoveTimer = 0.0f;
  } else {
    for (i = 0; i <= 5; i++) {
      if (HovaBlimp[i].ActiveMode != 0) {
        NuVecSub(&temp1, &newPos, &HovaBlimp[i].Position);
        NuVecSub(&temp2, &Hova->Position, &HovaBlimp[i].Position);
        if (temp1.x * temp1.x + temp1.z * temp1.z < 20.0f &&
            temp1.x * temp1.x + temp1.z * temp1.z < temp2.x * temp2.x + temp2.z * temp2.z) {
          Hova->MoveTimer = 0.0f;
          goto update_pos;
        }
      }
    }
  }
update_pos:
  Hova->Position = newPos;

  if (Hova->Thrust != 0) {
    if (Hova->Position.y > Hova->StartPos.y + Hova->ThrustOff) {
      Hova->Thrust = 0;
    }
  } else {
    if (Hova->Position.y < Hova->StartPos.y) {
      Hova->Thrust = 1;
      Hova->ThrustOff = frand() * 0.5f;
    }
  }

  if (Hova->Thrust != 0) {
    Hova->Velocity.y = Hova->Velocity.y + 0.05f;
  } else {
    Hova->Velocity.y = Hova->Velocity.y - 0.1f;
  }

  if (Hova->Velocity.y > 1.0f) {
    Hova->Velocity.y = 1.0f;
  }
  if (Hova->Velocity.y < -2.0f) {
    Hova->Velocity.y = -2.0f;
  }

  if (Hova->Velocity.y < 0.0f) {
    Hova->AngleY = Hova->Velocity.y * 5.0f + Hova->AngleY;
  }

  Hova->Position.y = Hova->Velocity.y * (1.0f / 60.0f) + Hova->Position.y;

  CollideGliderBullets(&Hova->Position, HovaRad, 1.0f / HovaY, 1, 1, 1);

  if (CollideGliderBullets(&Hova->Position, HovaRad, 1.0f / HovaY, 0, 1, 0) != 0) {
    Hova->HitPoints--;
    if (Hova->HitPoints <= 0) {
      Hova->Explode = 1;
    }
    MyGameSfx(0x8c, &Hova->Position, 0x3fff);
    AddScreenWumpa(1, Hova->Position.x, Hova->Position.y, Hova->Position.z);
    MyGameSfx(0x8c, &Hova->Position, 0x3fff);
    ClockOff();
  }

  if (Hova->Explode != 0) {
    MyGameSfx(0x5f, &Hova->Position, 0x3fff);
    AddGameDebris(0x17, &Hova->Position);
    AddDeb3(&Hova->Position, 6, 4, 0);
    AddDeb3(&Hova->Position, 7, 4, 0);
    AddDeb3(&Hova->Position, 8, 4, 0);
    Hova->ActiveMode = 0;
  }
}


// NGC MATCH
static s32 ProcessAllHovaBlimps(void) {
  s32 i;
  s32 Ret;

  Ret = 0;
  ACTIVEBLIMPCOUNT = 0;
  for (i = 0; i < 6; i++) {
    if (HovaBlimp[i].ActiveMode != 0) {
      Ret = 1;
      ProcessHovaBlimp(&HovaBlimp[i]);
      if (HovaBlimp[i].ActiveMode != 0) {
        ACTIVEBLIMPCOUNT++;
      }
    }
  }
  for (i = 0; i < 6; i++) {
    if (TornadoList[i].Active != 0) {
      ProcessTornado(&TornadoList[i], &HovaBlimp[i]);
    }
  }
  return Ret;
}

// NGC MATCH
void DrawAllHovaBlimps(void) {
  s32 i;
  struct nuvec_s pos;
  float dist;

  torndist = 99999.0f;
  for (i = 0; i < 6; i++) {
    if (HovaBlimp[i].ActiveMode != 0) {
      DrawHovaBlimp(&HovaBlimp[i]);
    }
  }

  jonemit += 60;
  if (jonemit > 179) {
    jonemit = 0;
  }

  for (i = 0; i <= 5; i++) {
    if (TornadoList[i].Active != 0) {
      if (jonemit == 0 && Paused == 0) {
        pos = TornadoList[i].Position;
        pos.y -= 20.0f;
        AddVariableShotDebrisEffect(GDeb[0xb4].i, &pos, 1, 0, 0);
        pos.y -= 3.0f;
        AddVariableShotDebrisEffect(GDeb[0xb8].i, &pos, 1, 0, 0);
      }
      pos = TornadoList[i].Position;
      dist = pos.z - player->obj.pos.z;
      dist = dist * dist;
      dist = (pos.x - player->obj.pos.x) * (pos.x - player->obj.pos.x) + dist;
      if (dist < torndist) {
        torndist = dist;
      }
      if (jontorn[i * 2] == -1) {
        AddDebrisEffect(&jontorn[i * 2], GDeb[0x1a8].i, pos.x, pos.y + jtemp1, pos.z);
      }
      if (jontorn[i * 2 + 1] == -1) {
        AddDebrisEffect(&jontorn[i * 2 + 1], GDeb[0x1ac].i, pos.x, pos.y + jtemp1, pos.z);
      }
    } else {
      if (jontorn[i * 2] != -1) {
        DebFree(&jontorn[i * 2]);
        jontorn[i * 2] = -1;
      }
      if (jontorn[i * 2 + 1] != -1) {
        DebFree(&jontorn[i * 2 + 1]);
        jontorn[i * 2 + 1] = -1;
      }
    }
  }
}


// NGC MATCH
SATELLITESTRUCT *InitSatellite(SPACESTATIONSTRUCT *SpaceStation, float AngleY) {
  SATELLITESTRUCT *Satellite;

  Satellite = &SatelliteList[SatelliteCharacterId++];
  if (Satellite->Active != 0) {
    return NULL;
  }
  memset(Satellite, 0, 0x124);
  if (MyInitModelNew(&Satellite->MainDraw, 0x8d, 0x1b, 0, NULL,
                     &Satellite->Position) == 0) {
    return NULL;
  }
  Satellite->SpaceStation = SpaceStation;
  Satellite->Position = SpaceStation->Position;
  Satellite->AngleY = AngleY;
  Satellite->TiltX = 0.0f;
  Satellite->TiltZ = 0.0f;
  Satellite->DestTiltX = 0.0f;
  Satellite->DestTiltZ = 0;
  Satellite->Active = 1;
  Satellite->HitPoints = 0x10;
  Satellite->Velocity = v000;
  return Satellite;
}

// NGC MATCH
void DrawSatellite(SATELLITESTRUCT *Satellite) {
  mTEMP = Satellite->SpaceStation->Matrix;
  NuMtxPreRotateY(&mTEMP, (int)(Satellite->AngleY * 182.04445f));
  NuMtxPreScale(&mTEMP, SetNuVecPntr(3.0f, 3.0f, 3.0f));
  *(struct nuvec_s *)(&mTEMP._30) = Satellite->Position;
  Satellite->Seen = MyDrawModelNew(&Satellite->MainDraw, &mTEMP, NULL);
  return;
}

// NGC MATCH
void ProcessSatellite(SATELLITESTRUCT *Satellite) {
  s32 Id;
  short Vol;
  struct nuvec_s vec;

  MyAnimateModelNew(&Satellite->MainDraw, 0.5f);
  NuVecRotateY(&vec, SetNuVecPntr(0.0f, 0.0f, 18.0f),
               ((Satellite->AngleY + Satellite->SpaceStation->SatelliteAngleY) *
                182.0444f));
  NuVecMtxTransform(&Satellite->Position, &vec,
                    &Satellite->SpaceStation->Matrix);
  CollideGliderBullets(&Satellite->Position, 3.0f, 1, 0.6666667f, 0, 1);
  if (CollideGliderBullets(&Satellite->Position, 3.0f, 0, 0.6666667f, 0, 0) !=
      0) {
    Satellite->HitPoints--;
    if (Satellite->HitPoints > 0) {
      MyGameSfx(0xbb, &Satellite->Position, 0x3fff);
    }
    AddScreenWumpa((Satellite->Position).x, (Satellite->Position).y,
                   (Satellite->Position).z, 1);
    ClockOff();
  }
  if ((Satellite->HitPoints < 1) || (Satellite->Explode != 0)) {
    Satellite->Active = 0;
    AddGameDebris(0x17, &Satellite->Position);
    MyGameSfx(0xb4, &Satellite->Position, 0x7fff);
  }
  SeekAngHalfLife360f(&Satellite->TiltX, Satellite->DestTiltX, 1.0f,
                      0.01666667f);
  SeekAngHalfLife360f(&Satellite->TiltZ, Satellite->DestTiltZ, 1.0f,
                      0.01666667f);
  Id = -1;
  Vol = 0;
  if (Level == 0x1a) {
    Vol = CRASHTEROIDSSATELLITEVOL;
    Id = 0xbd;
  }
  if (Id != -1) {
    MyGameSfxLoop(Id, &Satellite->Position, Vol);
  }
}

// NGC MATCH
void ProcessSatellites(void) {
  s32 i;

  for (i = 0; i < 9; i++) {
    if (SatelliteList[i].Active != 0) {
      ProcessSatellite(&SatelliteList[i]);
    }
  }
  return;
}

// NGC MATCH
void DrawSatellites(void) {
  s32 i;

  for (i = 0; i < 9; i++) {
    if (SatelliteList[i].Active != 0) {
      DrawSatellite(&SatelliteList[i]);
    }
  }
  return;
}

// NGC MATCH
void InitSpaceStation(struct nuvec_s *Pos, float AngleY, float TiltX,
                      float TiltZ, int Character) {
  SPACESTATIONSTRUCT *SpaceStation;
  s32 i;

  SpaceStation = &SpaceStationList[Character];
  if ((Character > 2) || (SpaceStation->Active != 0)) {
    return;
  }
  memset(SpaceStation, 0, 0xd0);
  SpaceStation->Position = *Pos;
  SpaceStation->Active = 1;
  SpaceStation->AngleY = AngleY;
  SpaceStation->HitPoints = 0x28;
  SpaceStation->Velocity = v000;
  SpaceStation->SatelliteAngleY = (float)(Character) * 40.0f;
  NuMtxSetScale(&mTEMP, SetNuVecPntr(1.0f, 1.0f, 1.0f));
  NuMtxRotateX(&mTEMP, (int)(TiltX * 182.04445f));
  NuMtxRotateZ(&mTEMP, (int)(TiltZ * 182.04445f));
  NuMtxRotateY(&mTEMP, (int)(SpaceStation->AngleY * 182.04445f));
  NuMtxTranslate(&mTEMP, &SpaceStation->Position);
  SpaceStation->Matrix = mTEMP;

  NuVecMtxTransform(&SpaceStation->AppCentre, &SPACETRAN,
                    &SpaceStation->Matrix);
  for (i = 0; i < 3; i++) {
    SpaceStation->Satellite[i] =
        InitSatellite(SpaceStation, ((float)i) * 120.0f);
  }
}

// NGC MATCH
void InitSpaceStations(void) {
  memset(SpaceStationList, 0, 0x270);
  memset(SatelliteList, 0, 0xa44);
  SatelliteCharacterId = 0;
  InitSpaceStation(SetNuVecPntr(0.0f, 0.0f, 80.0f), -0.0f, 15.0f, 20.0f, 0);
  InitSpaceStation(SetNuVecPntr(69.0f, 0.0f, -40.0f), -13.0f, -20.0f, 7.0f, 1);
  InitSpaceStation(SetNuVecPntr(-69.0f, 0.0f, -40.0f), -26.0f, 15.0f, -20.0f,
                   2);
}

// NGC MATCH
void DrawSpaceStation(SPACESTATIONSTRUCT *SpaceStation) {
  s32 i;

  SpaceStation->Seen = 0;
  i = 0x6d;
  if (ObjTab[i].obj.special != NULL) {
    mTEMP = SpaceStation->Matrix;
    SpaceStation->Seen = NuRndrGScnObj(
        ObjTab[i].obj.scene->gobjs[ObjTab[i].obj.special->instance->objid],
        &mTEMP);
  }
  if (SpaceStation->Seen != 0) {
    if (SpaceStation->ShieldDraw == 0) {
      return;
    }
    i = 0x71;
    if (SpaceStation->ShieldGreen != 0) {
      i = 0x70;
    }
    if (ObjTab[i].obj.special != NULL) {
      mTEMP = SpaceStation->ShieldMtx;
      NuRndrGScnObj(
          ObjTab[i].obj.scene->gobjs[ObjTab[i].obj.special->instance->objid],
          &mTEMP);
      NuMtxPreRotateY(&SpaceStation->ShieldMtx, 0x2000);
    }
  }
  if (SpaceStation->ShieldDraw != 0) {
    SpaceStation->ShieldDraw--;
  }
  return;
}

// NGC MATCH
void ProcessSpaceStation(SPACESTATIONSTRUCT *SpaceStation) {
  struct nuvec_s delta;
  struct nuvec_s diff;
  struct nuvec_s reflectNorm;
  struct nuvec_s reflectPos;
  struct nuvec_s TempPos;
  s32 i;
  s32 ShieldActive;
  float dist;
  float f31;
  float f30;
  float f29;
  s32 angY;
  s32 angX;
  GLIDERBULLET *Bullet;

  ShieldActive = 0;
  for (i = 0; i < 3; i++) {
    if (SpaceStation->Satellite[i] != NULL) {
      if (SpaceStation->Satellite[i]->Active != 0) {
        ShieldActive = 1;
      }
    }
  }

  GliderBulletReflectMtx = NULL;

  CollideGliderBullets(&SpaceStation->AppCentre, 18.0f, 1, 0.6666667f, 0,
                       ShieldActive);
  if (CollideGliderBullets(&SpaceStation->AppCentre, 18.0f, 0, 0.6666667f, 0,
                           ShieldActive) != 0) {
    SpaceStation->ShieldDraw = 0;
    if (ShieldActive == 0) {
      AddScreenWumpa(SpaceStation->Position.x, SpaceStation->Position.y,
                     SpaceStation->Position.z, 1);
      SpaceStation->HitPoints--;
      if (SpaceStation->HitPoints > 0) {
        MyGameSfx(0xbc, &SpaceStation->Position, 0x3fff);
      }
    } else {
      MyGameSfx(0xbe, &SpaceStation->Position, HITSHIELDVOLA);
      ClockOff();
    }
  }

  NuVecSub(&delta, &PlayerGlider.Position, &SpaceStation->AppCentre);
  diff = delta;
  diff.y /= SHIELDSCALEY;
  dist = NuVecMag(&delta);

  if (dist < 30.0f) {
    float dot;

    dot = DotProduct(&PlayerGlider.Velocity, &delta);
    if (dot < 0.0f) {
      float speed;
      speed = NuVecMag(&PlayerGlider.Velocity);
      if (speed < SPACEMINSPEEDIN) {
        speed = SPACEMINSPEEDIN;
      }

      NuVecScaleAccum(-dot / (dist * dist), &PlayerGlider.Velocity, &delta);
      NuVecNorm(&PlayerGlider.Velocity, &PlayerGlider.Velocity);
      NuVecScale(speed, &PlayerGlider.Velocity, &PlayerGlider.Velocity);

      NuVecScaleAccum(FIXVELBUMPAMOUNT / dist, &PlayerGlider.Velocity, &delta);

      {
        GLIDERSTRUCT *pg;
        pg = &PlayerGlider;
        pg->FixVelTimer = FIXVELTIME;
        pg->InTornado = 1;
        pg->InTornadoScale = SPACETORNADOSCALE;
      }

      if (Level == 0x1a) {
        MyGameSfx(0xc0, &PlayerGlider.Position, 0x7fff);
      }
    }

    {
      struct nuvec_s scaledDelta;
      struct nuvec_s rotResult;

      NuVecScale(30.0f / dist, &delta, &delta);
      scaledDelta = delta;
      scaledDelta.y *= SHIELDSCALEY;

      NuVecAdd(&reflectPos, &PlayerGlider.Position, &SpaceStation->AppCentre);

      f31 = 182.04445f;
      angY = NuAtan2D(delta.x * f31, delta.z * f31);
      NuVecRotateY(&rotResult, &delta, -angY);

      angX = NuAtan2D(rotResult.y * -1.0f, rotResult.z * f31);
      delta = *(struct nuvec_s *)&(Bullet->Mat._30);

      NuMtxSetRotationX(&mTEMP, angX);
      NuMtxRotateY(&mTEMP, angY);

      {
        struct nuvec_s scl;
        scl = *SetNuVecPntr(18.0f, 18.0f / 0.6666667f, 18.0f);
        NuMtxScale(&mTEMP, &scl);
      }

      GliderBulletReflectMtx = &mTEMP;
      NuVecScale(2.0f, &scaledDelta, &scaledDelta);
      NuVecAdd(&TempPos, &SpaceStation->AppCentre, &scaledDelta);

      AddGameDebris(0x1a, &TempPos);
      MyGameSfx(0xbe, NULL, HITSHIELDVOL);
    }
    SpaceStation->ShieldDraw = 0;
  }

  SpaceStation->SatelliteAngleY += 0.6f;
  if (SpaceStation->HitPoints <= 0 && SpaceStation->Explode == 0) {
    ;
  } else if (SpaceStation->HitPoints <= 0 || SpaceStation->Explode != 0) {
    SpaceStation->Active = 0;
    AddGameDebris(0x47, &SpaceStation->Position);
    MyGameSfx(0xbf, &SpaceStation->Position, SPACESTATIONEXPLODEVOL);
  }

  if (GliderBulletReflectMtx != NULL) {
    mTEMP = *(struct numtx_s *)GliderBulletReflectMtx;

    NuMtxPreScale(&mTEMP, SetNuVecPntr(18.0f, 18.0f, 18.0f));
    NuMtxPreRotateX(&mTEMP, -0x4000);
    NuMtxPreRotateY(&mTEMP, rand());

    *(struct nuvec_s *)&mTEMP._30 = SpaceStation->Position;
    SpaceStation->ShieldMtx = mTEMP;
    SpaceStation->ShieldDraw = 2;
  }
}


// NGC MATCH
void ProcessSpaceStations(void) {
  s32 i;

  PlayerGlider.InTornado = 0;
  for (i = 0; i < 3; i++) {
    if (SpaceStationList[i].Active != 0) {
      ProcessSpaceStation(&SpaceStationList[i]);
    }
  }
}

// NGC MATCH
void DrawSpaceStations(void) {
  s32 i;

  for (i = 0; i < 3; i++) {
    if (SpaceStationList[i].Active != 0) {
      DrawSpaceStation(&SpaceStationList[i]);
    }
  }
  return;
}

static void InitGliderBullets(void) {
  s32 i;

  for (i = 63; i >= 0; i--) {
    FreeGliderBulletList[i] = &GliderBullet[i];
  }
  for (i = 99; i >= 0; i--) {
    FreeEnemyGliderBulletList[i] = &EnemyGliderBullet[i];
  }
  FreeEnemyGliderBulletNum = 100;
  FreeGliderBulletNum = 64;
  UsedEnemyGliderBulletNum = 0;
  UsedGliderBulletNum = 0;
}

GLIDERBULLET *GrabGliderBullet(s32 Enemy) {
  GLIDERBULLET *Bullet;

  if (Enemy != 0) {
    if (FreeEnemyGliderBulletNum == 0) return 0;
    Bullet = FreeEnemyGliderBulletList[--FreeEnemyGliderBulletNum];
    UsedEnemyGliderBulletList[UsedEnemyGliderBulletNum++] = Bullet;
    return Bullet;
  } else {
    if (FreeGliderBulletNum == 0) return 0;
    Bullet = FreeGliderBulletList[--FreeGliderBulletNum];
    UsedGliderBulletList[UsedGliderBulletNum++] = Bullet;
    return Bullet;
  }
}

void FreeGliderBullet(s32 idx, s32 Enemy) {
  GLIDERBULLET *Bullet;

  if (Enemy != 0) {
    UsedEnemyGliderBulletNum--;
    Bullet = UsedEnemyGliderBulletList[idx];
    UsedEnemyGliderBulletList[idx] = UsedEnemyGliderBulletList[UsedEnemyGliderBulletNum];
    FreeEnemyGliderBulletList[FreeEnemyGliderBulletNum] = Bullet;
    FreeEnemyGliderBulletNum++;
    BulletEnemyNumFrees++;
  } else {
    UsedGliderBulletNum--;
    Bullet = UsedGliderBulletList[idx];
    UsedGliderBulletList[idx] = UsedGliderBulletList[UsedGliderBulletNum];
    FreeGliderBulletList[FreeGliderBulletNum] = Bullet;
    FreeGliderBulletNum++;
    BulletNumFrees++;
  }
}

// NGC MATCH
s32 AddGliderBullet(struct numtx_s *Mat, struct nuvec_s *Pos,
                    struct nuvec_s *Vel, s32 Enemy) {
  GLIDERBULLET *Bullet;

  Bullet = GrabGliderBullet(Enemy);
  if (Bullet != NULL) {
    if (Enemy != 0) {
      Bullet->Life = 0x78;
    } else {
      if (Level == 0xd) {
        Bullet->Life = 0x5a;
      } else if (Level == 0x1a) {
        Bullet->Life = 0x2d;
      } else {
        Bullet->Life = 0xb4;
      }
    }
    if (Enemy != 0) {
      Bullet->FirstLife = Bullet->Life - 1;
    } else {
      Bullet->FirstLife = Bullet->Life + 1;
    }
    Bullet->Mat = *Mat;
    Bullet->Mat._30 = Pos->x;
    Bullet->Mat._31 = Pos->y;
    Bullet->Mat._32 = Pos->z;
    Bullet->Vel.x = Vel->x * 0.016666668f;
    Bullet->Vel.y = Vel->y * 0.016666668f;
    Bullet->Vel.z = Vel->z * 0.016666668f;
    return 1;
  }
  return 0;
}

// NGC MATCH
s32 CollideGliderBullets(struct nuvec_s *Pos, float Radius, s32 Enemy,
                         float ScaleY, s32 HitFlag, s32 Reflect) {
  struct nuvec_s delta;
  s32 i;
  s32 hit;
  s32 Count;
  s32 EffectId;
  GLIDERBULLET **List;
  GLIDERBULLET *Bullet;
  float RadSqr;
  float ScaledRadius;
  float f30, f31;
  float f29, f28, f27;
  float f26, f25, f24, f23;
  float f22, f21, f20;

  RadSqr = Radius * Radius;
  ScaledRadius = Radius / ScaleY;
  hit = 0;

  if (Enemy != 0) {
    if (Level == 0x1a || Level == 0x12) {
      EffectId = 0x1a;
    } else {
      EffectId = 0x19;
    }
    List = UsedEnemyGliderBulletList;
    Count = UsedEnemyGliderBulletNum;
  } else {
    List = UsedGliderBulletList;
    Count = UsedGliderBulletNum;
    EffectId = 0x1a;
  }

  i = 0;
  if (Count <= 0) {
    return hit;
  }

  do {
    Bullet = List[i];
    i++;

    if (Bullet->Life >= Bullet->FirstLife) {
      goto next;
    }

    delta.x = Bullet->Mat._30 - Pos->x;
    if (delta.x > Radius || delta.x < -Radius) goto next;

    delta.y = Bullet->Mat._31 - Pos->y;
    if (delta.y > ScaledRadius || delta.y < -ScaledRadius) goto next;

    delta.z = Bullet->Mat._32 - Pos->z;
    if (delta.z > Radius || delta.z < -Radius) goto next;

    delta.y *= ScaleY;
    {
      float distSqr;
      distSqr = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
      if (!(distSqr <= RadSqr)) goto next;
    }

    hit = 1;

    if (Reflect != 0) {
      float VdotV, VdotD, DdotD;
      float disc;

      VdotV = DotProduct(&Bullet->Vel, &Bullet->Vel);
      f30 = VdotV;
      VdotD = DotProduct(&Bullet->Vel, &delta);
      f31 = VdotD + VdotD;
      DdotD = DotProduct(&delta, &delta);

      disc = f31 * f31 - 4.0f * f30 * (DdotD - RadSqr);
      disc = NuFsqrt(disc);
      disc = NuFabs(disc);
      f31 = -(f31 + disc);
      f30 = f30 + f30;

      NuVecScaleAccum(f31 / f30, &delta, &Bullet->Vel);
      NuVecAdd((struct nuvec_s *)&Bullet->Mat._30, &delta, Pos);

      {
        float newDot;
        newDot = DotProduct(&Bullet->Vel, &delta);
        f31 = newDot;
        if (f31 < 0.0f) {
          float magSqr;
          magSqr = NuVecMagSqr(&delta);
          NuVecScaleAccum(f31 * -2.0f / magSqr, &Bullet->Vel, &delta);
        }
      }

      GliderBulletReflectPos = *(struct nuvec_s *)&Bullet->Mat._30;

      {
        s32 angY;
        s32 angX;

        angY = (s32)(float)(s32)(u16)NuAtan2D(delta.x * 182.04445f, delta.z * 182.04445f);

        NuVecRotateY(&delta, &delta, -angY);

        angX = (s32)(float)(s32)(u16)NuAtan2D(delta.y * -1.0f, delta.z * 182.04445f);

        *(struct nuvec_s *)&Bullet->Mat._12 = *(struct nuvec_s *)&Bullet->Mat._30;

        NuMtxSetRotationX(Bullet, angX);
        NuMtxRotateY(Bullet, angY);
        {
          struct nuvec_s scaleVec;
          scaleVec.x = Radius;
          scaleVec.y = Radius / ScaleY;
          scaleVec.z = Radius;
          NuMtxScale(Bullet, &scaleVec);
        }
        NuMtxTranslate(Bullet, &delta);
      }

      if (Level == 0x1a) {
        if (Enemy != 0) {
          AddGameDebris(0x48, (struct nuvec_s *)&Bullet->Mat._30);
        } else {
          AddGameDebris(0x49, (struct nuvec_s *)&Bullet->Mat._30);
        }
      } else {
        AddGameDebris(EffectId, (struct nuvec_s *)&Bullet->Mat._30);
      }
    } else {
      AddGameDebris(EffectId, (struct nuvec_s *)&Bullet->Mat._30);
      i--;
      FreeGliderBullet(i, Enemy);

      if (HitFlag != 0) {
        s32 r;
        r = qrand();
        AddDeb3(Pos, r % 3 + 3, 3, 0);
      }
    }

  next:
    Count--;
  } while (Count != 0);

  return hit;
}

// NGC MATCH
void GliderBulletsHitThings(GLIDERSTRUCT *Glider) {
  struct obj_s obj;
  s32 count;
  struct nuvec_s v;
  float r;
  struct creature_s *c;
  GLIDERBULLET **bp;

  obj.bot = -0.3f;
  obj.top = 0.3f;
  obj.attack = 0;
  obj.SCALE = 1.0f;
  obj.RADIUS = 0.3f;
  obj.pLOCATOR = NULL;

  count = UsedGliderBulletNum;
  bp = UsedGliderBulletList;

  if (count > 0) {
    do {
      GLIDERBULLET *Bullet;
      Bullet = *bp;
      bp++;
      count--;

      obj.pos.x = Bullet->Mat._30;
      obj.pos.y = Bullet->Mat._31;
      obj.pos.z = Bullet->Mat._32;

      if (HitCrates(&obj, 1) != 0) {
        AddGameDebris(0x1a, &obj.pos);
        count++;
        bp--;
        AddScreenWumpa(temp_pCrate->pos.x, temp_pCrate->pos.y + 0.25f,
                       temp_pCrate->pos.z, 1);
        FreeGliderBullet(count, 0);
        if (temp_crate_type == 5) {
          Glider->HitPoints += 0x32;
          if (Glider->HitPoints > 100) {
            Glider->HitPoints = 100;
          }
        }
      } else if (HitCrateBalloons(&obj.pos, obj.RADIUS) != NULL) {
        AddGameDebris(0x1a, &obj.pos);
        count++;
        bp--;
        AddScreenWumpa(temp_pCrate->pos.x,
                       temp_pCrate->pos.y + CRATEBALLOONOFFSET,
                       temp_pCrate->pos.z, 1);
        FreeGliderBullet(count, 0);
        temp_pCrate->flags = (temp_pCrate->flags | 0x400) ^ 0x400;
      } else if (HitItems(&obj) != 0) {
        AddGameDebris(0x1a, &obj.pos);
        count++;
        bp--;
        FreeGliderBullet(count, 0);
      } else {
        c = FindClock();
        if (c != NULL) {
          v.x = c->obj.pos.x;
          v.y = c->obj.pos.y + CRATEBALLOONOFFSET;
          v.z = c->obj.pos.z;
          r = obj.RADIUS + CRATEBALLOONRADIUS;
          if (NuVecDistSqr(&obj.pos, &v, NULL) < (r * r)) {
            AddGameDebris(0x1a, &obj.pos);
            count++;
            FreeGliderBullet(count, 0);
            bp--;
            StartTimeTrial(&c->obj.pos, 1);
          }
        }
      }
    } while (count != 0);
  }
}


// NGC MATCH
void GliderBombsHitThings(GLIDERSTRUCT *Glider) {
  struct obj_s obj;
  s32 i;
  float r;
  struct nuvec_s v;
  struct creature_s *c;

  obj.bot = -0.3f;
  obj.top = 0.3f;
  obj.attack = 0;
  obj.SCALE = 1.0f;
  obj.RADIUS = 0.3f;
  obj.pLOCATOR = NULL;
  for (i = 0; i < 10; i++) {
    if (GliderBombs[i].Life != 0) {
      obj.pos = GliderBombs[i].Pos;
      if (HitCrates(&obj, 1) != 0) {
        AddGameDebris(0x1a, &obj.pos);
        AddScreenWumpa(temp_pCrate->pos.x, (temp_pCrate->pos.y + 0.25f),
                       temp_pCrate->pos.z, 1);
        GliderBombs[i].Life = 0;
        if (temp_crate_type == 5) {
          Glider->HitPoints += 0x32;
          if (Glider->HitPoints > 100) {
            Glider->HitPoints = 100;
          }
        }
      } else {
        if (HitCrateBalloons(&obj.pos, obj.RADIUS) != NULL) {
          AddGameDebris(0x1a, &obj.pos);
          AddScreenWumpa(temp_pCrate->pos.x,
                         temp_pCrate->pos.y + CRATEBALLOONOFFSET,
                         temp_pCrate->pos.z, 1);
          GliderBombs[i].Life = 0;
          temp_pCrate->flags = (temp_pCrate->flags | 0x400) ^ 0x400;
        } else {
          if (HitItems(&obj) != 0) {
            AddGameDebris(0x1a, &obj.pos);
            GliderBombs[i].Life = 0;
          } else {
            c = FindClock();
            if (c != NULL) {
              v.x = c->obj.pos.x;
              v.y = c->obj.pos.y + CRATEBALLOONOFFSET;
              v.z = c->obj.pos.z;
              r = (obj.RADIUS + CRATEBALLOONRADIUS);
              if (NuVecDistSqr(&obj.pos, &v, NULL) < (r * r)) {
                AddGameDebris(0x1a, &obj.pos);
                GliderBombs[i].Life = 0;
                StartTimeTrial(&c->obj.pos, 1);
              }
            }
          }
        }
      }
    }
  }
}

// NGC MATCH
void DrawGliderBullets(void) {
  s32 i;
  GLIDERBULLET **bp;

  if (Level == 0x1A) {
    bp = UsedGliderBulletList - 1;
    for (i = 0; i < UsedGliderBulletNum; i++) {
      NuRndrGScnObj(ObjTab[0x6F].obj.scene->gobjs[ObjTab[0x6F].obj.special->instance->objid],
                    *++bp);
    }
    if (UsedEnemyGliderBulletNum > 0) {
      bp = UsedEnemyGliderBulletList - 1;
      for (i = 0; i < UsedEnemyGliderBulletNum; i++) {
        NuRndrGScnObj(ObjTab[0x6E].obj.scene->gobjs[ObjTab[0x6E].obj.special->instance->objid],
                      *++bp);
      }
    }
  } else {
    if (UsedGliderBulletNum > 0) {
      bp = UsedGliderBulletList - 1;
      for (i = 0; i < UsedGliderBulletNum; i++) {
        NuRndrGScnObj(ObjTab[0].obj.scene->gobjs[ObjTab[0].obj.special->instance->objid],
                      *++bp);
      }
    }
    if (Level == 0x24) {
      if (UsedEnemyGliderBulletNum > 0) {
        bp = UsedEnemyGliderBulletList - 1;
        for (i = 0; i < UsedEnemyGliderBulletNum; i++) {
          NuRndrGScnObj(ObjTab[0xE].obj.scene->gobjs[ObjTab[0xE].obj.special->instance->objid],
                        *++bp);
        }
      }
    } else {
      if (UsedEnemyGliderBulletNum > 0) {
        bp = UsedEnemyGliderBulletList - 1;
        for (i = 0; i < UsedEnemyGliderBulletNum; i++) {
          NuRndrGScnObj(ObjTab[0xE].obj.scene->gobjs[ObjTab[0xE].obj.special->instance->objid],
                        *++bp);
        }
      }
    }
  }
}


static void ProcessGliderBullets(void) {
    int i;
    int j;
    int k;
    int MaxI;
    int loop;
    int EffectId;
    struct nuvec_s vec;
    GLIDERBULLET *Bullet;
    GLIDERBULLET **List;

    List = UsedGliderBulletList;
    MaxI = UsedGliderBulletNum;

    if (Level == 0x1a) {
        EffectId = -1;
    } else {
        EffectId = GDeb[0xB0].i;
    }

    for (loop = 0; loop <= 1; loop++) {
        j = 0;
        for (i = 0; i < MaxI; i++) {
            Bullet = List[j];

            Bullet->Mat._30 += Bullet->Vel.x;
            Bullet->Mat._31 += Bullet->Vel.y;
            Bullet->Mat._32 += Bullet->Vel.z;

            vec.x = Bullet->Mat._30;
            vec.y = Bullet->Mat._31;
            vec.z = Bullet->Mat._32;

            if (EffectId != -1 && Paused == 0) {
                float r = 0.25f;
                for (k = 4; k != 0; k--) {
                    AddVariableShotDebrisEffect(EffectId, &vec, 1, 0, 0);
                    vec.x -= Bullet->Vel.x * r;
                    vec.y -= Bullet->Vel.y * r;
                    vec.z -= Bullet->Vel.z * r;
                }
            }

            Bullet->Life--;
            if (Bullet->Life == 0) {
                FreeGliderBullet(j, loop);
            } else {
                j++;
            }
        }

        EffectId = -1;
        List = UsedEnemyGliderBulletList;
        MaxI = UsedEnemyGliderBulletNum;
    }
}

// NGC MATCH
void InitPlane(struct nuvec_s *Pos, float Vel, float Timer) {
  memset(&PlayerPlane, 0, 0x104);
  PlayerPlane.Active = 1;
  if (MyInitModelNew(&PlayerPlane.MainDraw, 0x51, 0x22, 0, NULL,
                     &PlayerPlane.Pos) == 0) {
    PlayerPlane.Active = 0;
  }
  PlayerPlane.Pos = *Pos;
  PlayerPlane.Vel = SetNuVec(0.0f, 0.0f, -Vel);
  PlayerPlane.ActionStatus = 0;
  PlayerPlane.ActionTimer = Timer;
}

// NGC MATCH
void DrawPlane(struct PLANESTRUCT *Plane) {
  static struct nuvec_s Scale = {1.0f, 1.0f, 1.0f};

  if (Plane->Active != 0) {
    NuMtxSetScale(&mTEMP, &Scale);
    NuMtxTranslate(&mTEMP, &Plane->Pos);
    MyDrawModelNew(&Plane->MainDraw, &mTEMP, NULL);
  }
  return;
}

// NGC MATCH
void MovePlane(PLANESTRUCT *Plane) {
  s32 Action;
  s32 GrabGlider;

  GrabGlider = 0;
  if (Plane->Active != 0) {
    Action = Plane->ActionStatus;
    switch (Action) {
    case 0:
      GrabGlider = 1;
      Level_GliderCurrentCeiling = 36.0f;
      if (ProcessTimer(&Plane->ActionTimer) != 0) {
        Plane->ActionStatus = 1;
      }
      break;
    case 1:
      Plane->ActionStatus = 2;
      Plane->ActionTimer = 1.0f;
      PlayerGlider.ForceTurn = Action;
      GameSfx(0x82, &player->obj.pos);
      NewRumble(&player->rumble, 0x7f);
      NewBuzz(&player->rumble, 6);
      break;
    case 2:
      if (ProcessTimer(&Plane->ActionTimer) != 0) {
        Plane->ActionStatus = 3;
      }
      break;
    case 3:
      Plane->Active = 0;
      break;
    }
    NuVecScaleAccum(0.01666667f, &Plane->Pos, &Plane->Vel);
    PlayerGlider.AutoPilot = GrabGlider;
    if (GrabGlider != 0) {
      NuVecAdd(&PlayerGlider.Position, &Plane->Pos, &ZepplinGliderOffset);
      GliderAutoPosition = PlayerGlider.Position;
      PlayerGlider.TiltX = 0.0f;
      PlayerGlider.TiltZ = 0.0f;
      PlayerGlider.Velocity = (Plane->Vel);
    }
    MyAnimateModelNew(&Plane->MainDraw, 0.5f);
  }
}

// NGC MATCH
void FarmReset(void) {
  if ((new_mode == -1) && (new_level == -1)) {
    FarmResetTimer = 0;
    Level_GliderSpeed = 13.0f;
    Level_GliderFloor = 16.0f;
    Level_GliderCeiling = 66.0f;
    Level_GliderRadius = 177.5;
    Level_DeadTime = 8.0f;
    *(struct nuvec_s *)&GameCam[0].m._30 = FarmCameraStart;
    Level_GliderCentreX = 0.0f;
    Level_GliderCentreZ = 0.0f;
    InitGlider(&PlayerGlider, NULL, 0.0f);
    PlayerGlider.AutoPilot = 1;
    InitZoffaUFOs();
    InitGliderBullets();
    InitialiseHovaBlimps();
    InitPlane(&FarmZepplinStart, FarmZepplinSpeed, FarmZepplinTimer);
    GliderCamHighTimer = 1.0f;
    ResetGameCameras(GameCam, 1);
    ACTIVEBLIMPCOUNT = 6;
    ResetTimeTrial();
  }
  return;
}

// NGC MATCH
void ProcessFarmLevel(struct nupad_s *Pad) {
  s32 i;

  FarmFrame++;
  if (GliderCamHighTimer > 0.0f) {
    GliderCamHighTimer -= 0.01666667f;
  } else {
    GliderCamHighTimer = 0.0f;
  }
  if (FarmResetTimer != 0) {
    FarmResetTimer--;
    if (FarmResetTimer == 0) {
      FarmReset();
    }
  }
  TeleportManager();
  LastNumZoffasFiring = NumZoffasFiring;
  NumZoffasFiring = 0;
  for (i = 0; i < 4; i++) {
    if (EnemyZoffa[i].ActiveMode == 0) {
      if (PlayerGlider.AutoPilot == 0) {
        EnemyZoffa[i].RespawnTimer -= 0.016666668f;
        if (EnemyZoffa[i].RespawnTimer < 0.0f) {
          EnemyZoffa[i].RespawnTimer = 0;
        }
      }
    } else {
      MoveZoffaUFO(&EnemyZoffa[i]);
    }
  }
  ZoffaUFORespawn();
  ProcessAllHovaBlimps();
  CheckGliderCollisions();
  CheckForPotentialMidAirCollisions();
  MovePlane(&PlayerPlane);
  ProcessGliderBullets();
}

// NGC MATCH
void DrawFarmLevelExtra(void) {
  DrawZoffaUFOs();
  DrawPlane(&PlayerPlane);
  DrawAllHovaBlimps();
  DrawGliderBullets();
  DrawGliderTarget();
  return;
}

// NGC MATCH
void GliderCamWeatherBoss(struct cammtx_s *CamMtx) {
  struct nuvec_s delta;
  struct nuvec_s CamPos;
  struct nuvec_s Interest;
  struct nuvec_s SplinePos;
  struct nuvec_s SplineInterest;
  struct nuvec_s SplineClosest;
  float f31;
  float dist;
  float tween;
  float VehCamX_360;
  float VehCamY_361;

  CamPos = PlayerGlider.Position;
  if (WBIntroOn == 0) {
    ProcessTimer(&WBIntroTweenTimer);
  }

  SplinePos = PlayerGlider.Position;
  Interest = PlayerGlider.Position;
  SplineClosest = PlayerGlider.Position;
  SplinePos.y += WBCamDist;
  Interest.y += WBCamDist;
  FindSplineClosestPointAndDist((struct MYSPLINE *)&WeatherBossCamSpline, 0x300, &CamPos, &SplineClosest, 0, 0);

  tween = WBIntroTweenTimer / WBIntroTweenTime;
  SplineInterest.x = (PlayerGlider.Position.x - SplineClosest.x) * 0.8f + SplineClosest.x;
  SplineInterest.y = SplineClosest.y;
  SplineInterest.z = PlayerGlider.Position.z;

  NuVecScale(tween, &CamPos, &SplinePos);
  NuVecScaleAccum(1.0f - WBIntroTweenTimer / WBIntroTweenTime, &CamPos, &SplineInterest);

  NuVecScale(WBIntroTweenTimer / WBIntroTweenTime, &Interest, &Interest);
  NuVecScaleAccum(1.0f - WBIntroTweenTimer / WBIntroTweenTime, &Interest, &SplineClosest);

  delta.x = CamPos.x - Interest.x;
  delta.y = CamPos.y - Interest.y;
  delta.z = CamPos.z - Interest.z;

  dist = NuFsqrt(delta.x * delta.x + delta.z * delta.z);

  {
    s32 angX;
    s32 angY;

    f31 = 182.04445f;
    angX = NuAtani((s32)(-delta.y * f31), (s32)(dist * f31));
    CamMtx->xrot = angX;
    angY = NuAtani((s32)(delta.x * f31), (s32)(delta.z * f31));
    angY ^= 0x8000;

    {
      s32 tmp;
      tmp = CamMtx->xrot ^ 0x8000;
      VehCamY_361 = (float)(s32)(u16)tmp / f31;
      VehCamX_360 = (float)(s32)(u16)angY / f31;
    }

    NuMtxSetRotationX(CamMtx, (s32)(VehCamX_360 * f31));
    NuMtxRotateY(CamMtx, (s32)(VehCamY_361 * f31));
    NuMtxTranslate(CamMtx, &Interest);
  }

  *(struct nuvec_s *)&CamMtx->pos = Interest;
  WeatherBossCamMtx = CamMtx->m;
}

// NGC MATCH
void GliderCam(struct cammtx_s *CamMtx) {
  struct nuvec_s delta;
  struct nuvec_s Interest;
  struct nuvec_s CamPos;
  struct nuvec_s Ideal;
  struct nuvec_s CamTarget;
  struct nuvec_s TempCam;
  static struct nuvec_s LastVel;
  float f31;
  float f30;
  float dist;
  float VehCamX_360;
  float VehCamY_361;
  float VehCamX_365;
  float VehCamY_366;
  float idx;
  float idy;
  float idz;
  struct nuvec_s PlayerPos;

  if (Level == 0x18) {
    GliderCamWeatherBoss(CamMtx);
    return;
  }

  Interest = GliderIntroInterest;
  CamPos.x = CamMtx->m._30;
  CamPos.y = CamMtx->m._31;
  CamPos.z = CamMtx->m._32;
  CamTarget = CamPos;

  SeekHalfLifeNUVEC(&CamTarget, &GliderIntroCamPos, SeekGliderCamTime, 0.01666667f);

  if ((Level == 0x12 || Level == 0x1a) && FireFlyIntroOn != 0) {
    goto intro_path;
  }

  {
    struct nuvec_s PlayerPos;

    f31 = PlayerGlider.AngleY;
    PlayerPos = PlayerGlider.Position;
    PlayerPos.y += 1.0f;

    if (Level == 0xd) {
      Ideal = TORNADOCAMIDEAL;
      PlayerPos.y += TORNADOINTERESTY;
    } else if (Level == 0x1a) {
      Ideal = SetNuVec(idx, idy, idz);
    } else {
      Ideal = SetNuVec(-3.0f, 5.0f, 20.0f);
    }

    f30 = 182.04445f;
    NuVecRotateX(&Ideal, &Ideal, (s32)(f31 * f30));

    if (FlyingLevelExtro != 0) {
      SeekAngLimHalfLife360f(&ExtroAngOff, ExtroTargAng, 0.01666667f, 60.0f);
      dist = NuFabs(ExtroAngOff);
      NuVecScale(1.0f - dist / 90.0f, &TempCam, &Ideal);
      dist = NuFabs(ExtroAngOff);
      NuVecScaleAccum(dist / 90.0f, &TempCam, &ExtroIdealPos);

      {
        float abs_diff;
        abs_diff = NuFabs(ExtroAngOff - ExtroTargAng);
        abs_diff = Rationalise360f(abs_diff);
        abs_diff = NuFabs(abs_diff);

        if (abs_diff < ExtroChangeVal) {
          NuVecScaleAccum(0.01666667f, &ExtroStillCamPos, &ExtroStillVelocity);
          NuVecScaleAccum(0.01666667f, &ExtroStillInterest, &ExtroStillVelocity);

          CamTarget = ExtroStillCamPos;
          PlayerPos = ExtroStillInterest;

          SeekHalfLifeNUVEC(&ExtroStillVelocity, &v000, ExtroSeekZeroTime, 0.01666667f);
          FlyingLevelVictoryDance = 1;
          Ideal = CamTarget;
        } else {
          NuVecRotateY(&TempCam, &Ideal, (s32)((PlayerGlider.AngleY + ExtroAngOff) * f30));
          NuVecAdd(&TempCam, &TempCam, &PlayerPos);

          ExtroStillCamPos = TempCam;
          ExtroStillInterest = PlayerPos;
          ExtroStillVelocity = PlayerGlider.Velocity;
          FlyingLevelCompleteTimer = ExtroStillToEndTime;
          Ideal = TempCam;
        }
      }
    } else {
      NuVecRotateY(&TempCam, &Ideal, (s32)(PlayerGlider.AngleY * f30));
      ExtroAngOff = -3.0f;
      NuVecAdd(&TempCam, &TempCam, &PlayerPos);
      Ideal = TempCam;
    }

    CamPos.x = CamMtx->m._30;
    CamPos.y = CamMtx->m._31;
    CamPos.z = CamMtx->m._32;

    if (PlayerGlider.InTornado != 0 || PlayerGlider.TerminalDive != 0 ||
        PlayerGlider.TerminalDir != 0) {
      SeekHalfLifeNUVEC(&LastVel, SetNuVecPntr(0.0f, 0.0f, 0.0f), 3.0f, 0.01666667f);
      NuVecAdd(&CamPos, &CamPos, &LastVel);
    } else {
      if (PlayerGlider.AutoPilot != 0) {
        f31 = CameraSeekSpeedAutoPilot;
      } else {
        f31 = SeekGliderCamTime;
      }

      LastVel = CamPos;
      SeekHalfLifeNUVEC(&CamPos, &Ideal, f31, 0.01666667f);

      if (Level == 0x12 || Level == 0x1a) {
        if (FireFlyIntroTween != 0.0f && FFTWEENTIME != 0.0f) {
          f31 = FireFlyIntroTween / FFTWEENTIME;
          f30 = 1.0f - f31;

          NuVecScale(f30, &PlayerPos, &PlayerPos);
          NuVecScaleAccum(f31, &PlayerPos, &Interest);
          NuVecScale(f30, &CamPos, &CamPos);
          NuVecScaleAccum(f31, &CamPos, &CamTarget);
          ProcessTimer(&FireFlyIntroTween);
        }
      }

      if (Level == 0x12) {
        if (CamPos.y >= Level_GliderCeiling - 8.0f) {
          CamPos.y = Level_GliderCeiling - 8.0f;
        }
      }

      NuVecSub(&LastVel, &CamPos, &LastVel);
    }
    goto end_cam;
  }

intro_path:
  PlayerPos = Interest;
  CamPos = CamTarget;
  CamTarget = CamTarget;
  FireFlyIntroTween = FFTWEENTIME;

end_cam:
  delta.x = CamPos.x - PlayerPos.x;
  delta.y = CamPos.y - PlayerPos.y;
  delta.z = CamPos.z - PlayerPos.z;

  dist = NuFsqrt(delta.x * delta.x + delta.z * delta.z);

  {
    s32 angX;
    s32 angY;

    f31 = 182.04445f;
    angX = NuAtani((s32)(-delta.y * f31), (s32)(dist * f31));
    CamMtx->xrot = angX;
    angY = NuAtani((s32)(delta.x * f31), (s32)(delta.z * f31));
    angY ^= 0x8000;

    {
      s32 cvx;
      s32 cvy;
      cvx = CamMtx->xrot ^ 0x8000;
      VehCamY_366 = (float)(s32)(u16)cvx / f31;
      VehCamX_365 = (float)(s32)(u16)angY / f31;
    }

    NuMtxSetRotationX(CamMtx, (s32)(VehCamX_365 * f31));
    NuMtxRotateY(CamMtx, (s32)(VehCamY_366 * f31));
    NuMtxTranslate(CamMtx, &CamPos);

    NuMtxRotateY(&GliderInvMtx, (s32)(-VehCamY_366 * f31));
    NuMtxSetRotationX(&GliderInvMtx, (s32)(-VehCamX_365 * f31));
    NuMtxTranslate(&GliderInvMtx, &CamPos);
  }

  *(struct nuvec_s *)&CamMtx->pos = CamPos;
}

// NGC MATCH
ASTEROIDSTRUCT *FindFreeAsteroid(s32 Priority) {
  s32 i;
  s32 p;

  for (i = 0; i < 100; i++) {
    if (AsteroidList[i].Active == 0) {
      return &AsteroidList[i];
    }
  }

  for (p = 0; p < Priority; p++) {
    for (i = 0; i < 100; i++) {
      if (AsteroidList[i].HitPoints <= p) {
        if (AsteroidList[i].Seen == 0) {
          return &AsteroidList[i];
        }
      }
    }
  }

  return NULL;
}

// NGC MATCH
struct nuvec_s *GetRandomGliderLevelEdgePoint(struct nuvec_s *Out) {
  struct nuvec_s pos;
  struct nuvec_s local;

  pos.x = 0.0f;
  pos.y = 0.0f;
  pos.z = Level_GliderRadius;
  local = pos;

  NuVecRotateY(&local, &local, (s32)(frand() * 360.0f * 182.04445f));

  Out->x = local.x;
  local.y = frand() * (Level_GliderCeiling - Level_GliderFloor) + Level_GliderFloor;
  Out->z = local.z;
  Out->y = local.y;
  return Out;
}

// NGC MATCH
void InitAsteroid(s32 Type) {
  ASTEROIDSTRUCT *ast;
  struct nuvec_s pos;
  struct nuvec_s dir;
  struct nuvec_s vel;
  struct nuvec_s scl;
  float angX, angY;
  float speed;

  ast = FindFreeAsteroid(3);
  if (ast == NULL) return;
  if (ast->Active != 0) return;

  memset(ast, 0, 0x4c);

  GetRandomGliderLevelEdgePoint(&pos);
  ast->Position = pos;

  GetRandomGliderLevelEdgePoint(&dir);
  NuVecSub(&dir, &dir, &ast->Position);
  NuVecNorm(&dir, &dir);
  NuVecScale(0.5f, &ast->Velocity, &dir);

  ast->AngleX = frand() * 360.0f;
  ast->AngleY = frand() * 360.0f;
  speed = (frand() - 0.5f) * 2.0f * 5.0f;
  ast->AngVelX = speed;
  speed = (frand() - 0.5f) * 2.0f * 5.0f;

  scl = SetNuVec(speed, speed, speed);
  ast->Scale = scl;
  ast->Stealth = Type;
  ast->Active = 1;
  ast->HitPoints = 3;
  ast->AngVelY = speed;
  NuVecScale(5.0f, &ast->Velocity, &ast->Velocity);
}


// NGC MATCH
void DrawSpaceArenaLevelExtra(void) {
  DrawGliderBullets();
  DrawAsteroids();
  DrawSatellites();
  DrawSpaceStations();
  DrawZoffaUFOs();
  DrawGliderTarget();
  return;
}

// NGC MATCH
void ProcessSpaceArenaLevel(struct nupad_s *Pad) {
  s32 i;

  if (CRASHTEROIDSAMBIENTVOL != 0) {
    MyGameSfxLoop(0xc1, NULL, CRASHTEROIDSAMBIENTVOL);
  }
  ActiveAsteroidListNum = 0;
  TeleportManager();
  LastNumZoffasFiring = NumZoffasFiring;
  NumZoffasFiring = 0;
  for (i = 0; i < 4; i++) {
    if (EnemyZoffa[i].ActiveMode == 0) {
      if (PlayerGlider.AutoPilot == 0) {
        EnemyZoffa[i].RespawnTimer -= 0.016666668f;
        if (EnemyZoffa[i].RespawnTimer < 0.0f) {
          EnemyZoffa[i].RespawnTimer = 0.0f;
        }
      }
    } else {
      MoveZoffaUFO(&EnemyZoffa[i]);
    }
  }
  ZoffaUFORespawn();
  ProcessGliderBullets();
  ProcessAsteroids();
  ProcessSatellites();
  ProcessSpaceStations();
  CheckGliderCollisions();
  CheckForPotentialMidAirCollisions();
}

// NGC MATCH
void DrawExtraCreatures(void) {
  switch (Level) {
  case 13:
    DrawFarmLevelExtra();
    return;
  case 18:
    DrawFireFlyLevelExtra();
    return;
  case 36:
    DrawWeatherResearchLevelExtra();
    return;
  case 24:
    DrawWeatherBossLevelExtra();
    return;
  case 26:
    DrawSpaceArenaLevelExtra();
    return;
  case 3:
    DrawWesternArenaLevelExtra();
    return;
  case 22:
    DrawFireBossLevelExtra();
    return;
  case 21:
    DrawEarthBossLevelExtra();
    return;
  }
}

// NGC MATCH
void InitAsteroids(void) {
  s32 i;

  memset(&AsteroidList, 0, 0x1db0);
  for (i = 0; i < 0x1e; i++) {
    InitAsteroid(0);
  }
}

// NGC MATCH
void DrawAsteroid(ASTEROIDSTRUCT *Asteroid) {
  if (Asteroid->Stealth != 0) {
    NuMtxSetScale(&mTEMP, SetNuVecPntr(0.0f, 0.0f, 0.0f));
    NuMtxTranslate(&mTEMP, &Asteroid->Position);
  } else {
    NuMtxSetScale(&mTEMP, &Asteroid->Scale);
    NuMtxRotateY(&mTEMP, (s32)(Asteroid->AngleY * 182.04445f));
    NuMtxRotateX(&mTEMP, (s32)(Asteroid->AngleX * 182.04445f));
    NuMtxTranslate(&mTEMP, &Asteroid->Position);
  }
  Asteroid->Seen = 0;
  if (ObjTab[0x50].obj.special != NULL) {
    Asteroid->Seen =
        NuRndrGScnObj((ObjTab[0x50].obj.scene)
                          ->gobjs[(ObjTab[0x50].obj.special)->instance->objid],
                      &mTEMP);
  }
}

// NGC MATCH
void ProcessAsteroids(void) {
  s32 i;

  if (AsteroidDebug == -1) {
    for (i = 0; i < 100; i++) {
      if (AsteroidList[i].Active != 0) {
        ProcessAsteroid(&AsteroidList[i]);
      }
    }
  } else if (AsteroidList[AsteroidDebug].Active != 0) {
    ProcessAsteroid(&AsteroidList[AsteroidDebug]);
  }
}

// NGC MATCH
void DrawAsteroids(void) {
  s32 i;

  for (i = 0; i < 100; i++) {
    if (AsteroidList[i].Active != 0) {
      DrawAsteroid(&AsteroidList[i]);
    }
  }
}

// NGC MATCH
static s32 CheckSphereCollide(struct nuvec_s *A, struct nuvec_s *B,
                              float CombinedRadius, float YScale) {
  struct nuvec_s Rel;

  Rel.y = (A->y - B->y) * YScale;
  Rel.x = A->x - B->x;
  Rel.z = A->z - B->z;
  if (Rel.x * Rel.x + Rel.y * Rel.y + Rel.z * Rel.z <=
      CombinedRadius * CombinedRadius) {
    return 1;
  }
  return 0;
}

// NGC MATCH
s32 PossIntersect(struct nuvec_s *PosA, struct nuvec_s *VelA, struct nuvec_s *PosB,
                  struct nuvec_s *VelB, float Radius, float MaxTime) {
  struct nuvec_s A, AV;
  float MagA, MagVelA, Angle, Time;

  A.x = PosA->x - PosB->x;
  A.y = PosA->y - PosB->y;
  A.z = PosA->z - PosB->z;
  AV.x = VelB->x - VelA->x;
  AV.y = VelB->y - VelA->y;
  AV.z = VelB->z - VelA->z;

  MagA = NuVecMag(&A);

  if (MagA < Radius) {
    if (DotProduct(VelA, VelB) <= 0.0f) return 1;
    return -1;
  }

  MagVelA = NuVecMag(VelA);
  NuVecMag(&AV);

  Angle = -DotProduct(&A, VelA) / (MagA * MagVelA);
  if (Angle < 0.866f) return 0;

  Time = DotProduct(&A, &A) / DotProduct(&A, &AV);
  if (Time <= 0.0f) return 0;
  if (Time >= MaxTime) return 0;

  if (DotProduct(VelA, VelB) <= 0.0f) return 1;
  return -1;
}


// NGC MATCH
static s32 SafeFromCollisions(struct nuvec_s *Pos) {
  s32 i;

  if (CheckSphereCollide(Pos, &PlayerGlider.Position, 3.0f, 1.0f) != 0) {
    return 0;
  }
  for (i = 0; i < 4; i++) {
    if ((EnemyZoffa[i].ActiveMode != 0) &&
        (CheckSphereCollide(&EnemyZoffa[i].Position, Pos, 3.0f, 1.0f) != 0))
      return 0;
  }
  return 1;
}

// NGC MATCH
s32 LinesIntersectEllipse(struct nuvec_s *Points, s32 NumPoints,
                          struct nuvec_s *Centre, float RadiusY, float Radius) {
    struct nuvec_s ScaledPoints[10];
    struct nuvec_s ToCenter;
    struct nuvec_s ToCenter2;
    struct nuvec_s Edge;
    s32 result = 0;
    s32 i;
    float dot;
    float Radius2;
    float perpDist;

    for (i = 0; i < NumPoints; i++) {
        ScaledPoints[i].x = Points[i].x;
        ScaledPoints[i].y = Centre->y + (Points[i].y - Centre->y) / RadiusY;
        ScaledPoints[i].z = Points[i].z;
    }

    ScaledPoints[NumPoints] = ScaledPoints[0];

    Radius2 = Radius * Radius;

    for (i = 0; i < NumPoints; i++) {
        NuVecSub(&Edge, &ScaledPoints[i + 1], &ScaledPoints[i]);
        NuVecSub(&ToCenter, Centre, &ScaledPoints[i]);
        dot = DotProduct(&ToCenter, &Edge);

        if (dot < 0.0f) {
            if (NuVecMagSqr(&ToCenter) < Radius2) {
                result |= (1 << i);
            }
        } else {
            NuVecSub(&ToCenter2, Centre, &ScaledPoints[i + 1]);
            if (DotProduct(&ToCenter2, &Edge) > 0.0f) {
                if (NuVecMagSqr(&ToCenter2) < Radius2) {
                    result |= (1 << i);
                }
            } else {
                float toCenterMagSqr = NuVecMagSqr(&ToCenter);
                float edgeMagSqr = NuVecMagSqr(&Edge);
                perpDist = toCenterMagSqr - (dot * dot) / edgeMagSqr;
                if (perpDist < Radius2) {
                    result |= (1 << i);
                }
            }
        }
    }

    return result;
}


// NGC MATCH
static void GenerateGliderLines(GLIDERSTRUCT *Glider, struct nuvec_s *Lines) {
  struct numtx_s Mat;
  s32 i;

  NuMtxSetRotationZ(&Mat, (Glider->TiltZ * 182.04445f));
  NuMtxRotateX(&Mat, ((Glider->TiltX + 15.0f) * 182.04445f));
  NuMtxRotateY(&Mat, (Glider->AngleY * 182.04445f));
  NuMtxTranslate(&Mat, &Glider->Position);
  for (i = 0; i < 5; i++) {
    NuVecMtxTransform(&Lines[i], &GliderCollPoints[i], &Mat);
  }
  Lines[5] = *Lines;
}

// NGC MATCH
void CheckGliderCollisions(void) {
  struct nuvec_s diff, diff2, diff3, diff4;
  static struct nuvec_s NullVec = {0.0f, 0.0f, 0.0f};
  GLIDERSTRUCT *Glider;
  s32 i;
  float dist;

  Glider = (GLIDERSTRUCT *)player->Buggy;
  if (Glider == NULL) return;
  if (Glider->Dead != 0) return;

  // Check Zoffa collisions
  for (i = 0; i < 4; i++) {
    if (EnemyZoffa[i].ActiveMode != 0) {
      dist = NuVecDistSqr(&Glider->Position, &EnemyZoffa[i].Position, &diff);
      if (dist < 25.0f) {
        NuVecSub(&diff, &Glider->Position, &EnemyZoffa[i].Position);
        NuVecNorm(&diff, &diff);
        NuVecScale(5.0f, &diff, &diff);
        NuVecAdd(&Glider->Velocity, &Glider->Velocity, &diff);

        if (InvincibilityCHEAT == 0 && VehicleLevelImmune == 0) {
          Glider->HitPoints -= 10;
          if (Glider->HitPoints < 0) {
            Glider->HitPoints = 0;
          }
        }

        NewBuzz(&player->rumble, 6);
        NewRumble(&player->rumble, 0xb4);
        MyGameSfx(0xb7, NULL, 0x3fff);
      }
    }
  }

  // Check Space Stations
  for (i = 0; i < 3; i++) {
    if (SpaceStationList[i].Active != 0) {
      dist = NuVecDistSqr(&Glider->Position, &SpaceStationList[i].Position, &diff2);
      if (dist < 400.0f) {
        NuVecSub(&diff2, &Glider->Position, &SpaceStationList[i].Position);
        NuVecNorm(&diff2, &diff2);
        NuVecScale(20.0f, &diff2, &diff2);
        NuVecAdd(&Glider->Velocity, &Glider->Velocity, &diff2);
      }
    }
  }

  // Check Satellites
  for (i = 0; i < 9; i++) {
    if (SatelliteList[i].Active != 0) {
      dist = NuVecDistSqr(&Glider->Position, &SatelliteList[i].Position, &diff3);
      if (dist < 9.0f) {
        NuVecSub(&diff3, &Glider->Position, &SatelliteList[i].Position);
        NuVecNorm(&diff3, &diff3);
        NuVecScale(3.0f, &diff3, &diff3);
        NuVecAdd(&Glider->Velocity, &Glider->Velocity, &diff3);

        if (InvincibilityCHEAT == 0 && VehicleLevelImmune == 0) {
          Glider->HitPoints -= 5;
          if (Glider->HitPoints < 0) {
            Glider->HitPoints = 0;
          }
        }
      }
    }
  }

  // Check Big Guns
  for (i = 0; i < 12; i++) {
    if (BigGunList[i].Active != 0) {
      dist = NuVecDistSqr(&Glider->Position, &BigGunList[i].Position, &diff4);
      if (dist < 16.0f) {
        NuVecSub(&diff4, &Glider->Position, &BigGunList[i].Position);
        NuVecNorm(&diff4, &diff4);
        NuVecScale(4.0f, &diff4, &diff4);
        NuVecAdd(&Glider->Velocity, &Glider->Velocity, &diff4);
      }
    }
  }

  // Check Gun Boats
  for (i = 0; i < 4; i++) {
    if (GunBoatList[i].Active != 0) {
      dist = NuVecDistSqr(&Glider->Position, &GunBoatList[i].Position, &diff);
      if (dist < 25.0f) {
        NuVecSub(&diff, &Glider->Position, &GunBoatList[i].Position);
        NuVecNorm(&diff, &diff);
        NuVecScale(5.0f, &diff, &diff);
        NuVecAdd(&Glider->Velocity, &Glider->Velocity, &diff);
      }
    }
  }

  // Check BattleShips
  for (i = 0; i < 6; i++) {
    if (BattleShipList[i].Active != 0) {
      dist = NuVecDistSqr(&Glider->Position, &BattleShipList[i].Position, &diff2);
      if (dist < 100.0f) {
        NuVecSub(&diff2, &Glider->Position, &BattleShipList[i].Position);
        NuVecNorm(&diff2, &diff2);
        NuVecScale(10.0f, &diff2, &diff2);
        NuVecAdd(&Glider->Velocity, &Glider->Velocity, &diff2);
      }
    }
  }

  // Check Hova Blimps
  for (i = 0; i < 6; i++) {
    if (HovaBlimp[i].ActiveMode != 0) {
      dist = NuVecDistSqr(&Glider->Position, &HovaBlimp[i].Position, &diff3);
      if (dist < 36.0f) {
        NuVecSub(&diff3, &Glider->Position, &HovaBlimp[i].Position);
        NuVecNorm(&diff3, &diff3);
        NuVecScale(6.0f, &diff3, &diff3);
        NuVecAdd(&Glider->Velocity, &Glider->Velocity, &diff3);

        if (InvincibilityCHEAT == 0 && VehicleLevelImmune == 0) {
          Glider->HitPoints -= 10;
          if (Glider->HitPoints < 0) {
            Glider->HitPoints = 0;
          }
        }
      }
    }
  }

  // Check Asteroids
  for (i = 0; i < 100; i++) {
    if (AsteroidList[i].Active != 0) {
      dist = NuVecDistSqr(&Glider->Position, &AsteroidList[i].Position, &diff4);
      if (dist < AsteroidList[i].CentreDist2 * AsteroidList[i].CentreDist2) {
        NuVecSub(&diff4, &Glider->Position, &AsteroidList[i].Position);
        NuVecNorm(&diff4, &diff4);
        NuVecScale(AsteroidList[i].CentreDist2, &diff4, &diff4);
        NuVecAdd(&Glider->Velocity, &Glider->Velocity, &diff4);

        if (InvincibilityCHEAT == 0 && VehicleLevelImmune == 0) {
          Glider->HitPoints -= 5;
          if (Glider->HitPoints < 0) {
            Glider->HitPoints = 0;
          }
        }
      }
    }
  }

  // Check Tornado collisions
  for (i = 0; i < 6; i++) {
    if (TornadoList[i].Active != 0) {
      dist = NuVecDistSqr(&Glider->Position, &TornadoList[i].Position, &diff);
      if (dist < 100.0f) {
        Glider->InTornado = 1;
        Glider->InTornadoScale = 1.0f;
      }
    }
  }
}


// NGC MATCH
s32 GetCurrentFarmObjectives(void) {
  s32 i;
  s32 j = 0;

  for (i = 0; i < 6; i++) {
    if (HovaBlimp[i].ActiveMode != 0) {
      j++;
    }
  }
  return j;
}

// NGC MATCH
void InitGunBoat(struct nuvec_s *Pos, float AngleY, s32 Character) {
  GUNBOATSTRUCT *GunBoat;
  s32 CharacterId;

  GunBoat = &GunBoatList[Character];
  if (GunBoat->Active != 0) {
    return;
  }
  memset(&GunBoatList[Character], 0, 0x134);
  GunBoat->Character = Character;
  GunBoat->Position = *Pos;
  GunBoat->AngleY = AngleY;
  GunBoat->Action = 1;
  GunBoat->LastAction = -1;
  GunBoat->FireAngleX = 45.0;
  switch (Character) {
  case 0:
    CharacterId = 0x68;
    break;
  case 1:
    CharacterId = 0x69;
    break;
  case 2:
    CharacterId = 0x67;
    break;
  default:
    CharacterId = 0x66;
    break;
  }
  if (MyInitModelNew(&GunBoat->MainDraw, CharacterId, 0x22, 0, NULL,
                     &GunBoat->Position) == 0) {
    GunBoat->Character = -1;
  }
  GunBoat->Active = 1;
}

// NGC MATCH
void InitGunBoats(void) {
  memset(&GunBoatList, 0, 0x4d0);
  InitGunBoat(SetNuVecPntr(-24.0f, 0.0f, 50.0f), 0.0f, 0);
  InitGunBoat(SetNuVecPntr(36.0f, 0.0f, -15.0f), 0.0f, 1);
  InitGunBoat(SetNuVecPntr(93.0f, 0.0f, 43.0f), 0.0f, 2);
  InitGunBoat(SetNuVecPntr(21.0f, 0.0f, 106.0f), 0.0f, 3);
}

// NGC MATCH
void DrawGunBoat(GUNBOATSTRUCT *GunBoat) {
  struct nuvec_s ScaleVec = {1.5f, 1.5f, 1.5f};

  NuMtxSetScale(&mTEMP, &ScaleVec);
  NuMtxRotateX(&mTEMP, (int)(GunBoat->FireAngleX * 182.04445f));
  NuMtxRotateY(&mTEMP,
               (int)((GunBoat->AngleY + GunBoat->FireAngleY) * 182.04445f));
  NuMtxTranslate(&mTEMP, &GunBoat->Position);
  GunBoat->Seen = MyDrawModelNew(&GunBoat->MainDraw, &mTEMP, NULL);
}

// NGC MATCH
float GetGunBoatBestTarget(float Best, struct nuvec_s **TargetPos,
                           struct nuvec_s **Vel, s32 *Moving) {
  struct nuvec_s CamDir;
  struct nuvec_s Rel;
  s32 i;
  float Mag;
  float Dot;

  NuVecMtxRotate(&CamDir, SetNuVecPntr(0.0f, 0.0f, 1.0f), &GameCam[0].m);
  for (i = 0; i < 4; i++) {
    if (GunBoatList[i].Active != 0) {
      if (GunBoatList[i].Action != 2) {
        NuVecSub(&Rel, &GunBoatList[i].Position, &GameCam[0].pos);
        Mag = NuVecMag(&Rel);
        if (Mag > 10.0f) {
          NuVecScale((1.0f / Mag), &Rel, &Rel);
          Dot = DotProduct(&CamDir, &Rel);
          if (Dot > Best) {
            *TargetPos = &GunBoatList[i].Position;
            *Vel = &v000;
            *Moving = 0;
            Best = Dot;
          }
        }
      }
    }
  }
  return Best;
}

// NGC MATCH
void ProcessGunBoat(GUNBOATSTRUCT *GunBoat) {
  struct nuvec_s delta;
  struct nuvec_s diff;
  struct nuvec_s RotResult;
  struct numtx_s Mat;
  struct nuvec_s Vel;
  struct nuvec_s Pos;
  struct nuvec_s DirVec;
  float f31;
  float f30;
  float f29;
  float f28;
  float dist;
  float seekAng;
  float seekRate;
  s32 angY;

  MyAnimateModelNew(&GunBoat->MainDraw, 0.5f);

  if (GunBoat->Action != 2) {
    SetNuVecPntr(1.5f, 1.5f, 1.5f);
    CollideGliderBombs(&GunBoat->Position, NULL, 0, 5.0f);

    if (CollideGliderBombs(&GunBoat->Position, NULL, 0, 5.0f) != 0) {
      AddGameDebris(0x43, &GliderBombCollisionPos);
      GunBoat->Action = 2;
      AddScreenWumpa(GunBoat->Position.x, GunBoat->Position.y,
                     GunBoat->Position.z, 1);
      MyGameSfx(0xb5, &GunBoat->Position, GUNBOATVOLUME);
      ClockOff();
    }
  }

  if (GunBoat->LastAction != GunBoat->Action) {
    switch (GunBoat->Action) {
    case 1:
      GunBoat->BurstTimer = 5.0f;
      break;
    case 2:
      GunBoat->SunkTimer = 3.0f;
      break;
    }
  }

  GunBoat->LastAction = GunBoat->Action;
  dist = NuVecDist(&PlayerGlider.Position + 0x124, &GunBoat->Position, &diff);

  if (PlayerGlider.AutoPilot != 0) {
    diff.y -= 20.0f;
  }

  NuVecScaleAccum(dist / 300.0f, &diff, &PlayerGlider.Velocity + 0x130);

  switch (GunBoat->Action) {
  case 0:
    if (dist <= 200.0f) {
      GunBoat->Action = 1;
    }
    break;

  case 1:
    if (dist > 300.0f) {
      GunBoat->Action = 0;
      break;
    }

    f29 = 182.04445f;

    NuMtxSetRotationY(&Mat, (s32)(-GunBoat->AngleY * f29));
    NuVecMtxRotate(&RotResult, &diff, &Mat);

    f31 = 182.04445f;
    angY = NuAtan2D(RotResult.x * f31, RotResult.z * f31);
    angY ^= 0x8000;

    {
      float angFloat;
      angFloat = (float)(s32)(u16)angY;
      f28 = angFloat / f29;
    }

    {
      float negAng;
      negAng = -f28;
      NuVecRotateY(&RotResult, &RotResult, (s32)(negAng * f29));
    }

    {
      s32 angXi;
      angXi = (s32)NuAtan2D(RotResult.y * -1.0f, RotResult.z * f31);
      angXi ^= 0x8000;
    }

    {
      float seekYaw;
      seekYaw = f28 - GunBoat->AngleY;
      seekYaw = Rationalise360f(seekYaw);

      if (seekYaw > 5.0f) {
        seekRate = 0.5f;
      } else if (seekYaw < -5.0f) {
        seekRate = -0.5f;
      } else {
        seekRate = seekYaw * 0.5f / 5.0f;
      }

      GunBoat->FireAngMomY = seekRate;
      GunBoat->AngleY = seekRate * 0.01666667f + GunBoat->AngleY;
    }

    {
      float elev;
      elev = f28 - GunBoat->FireAngleX;
      elev = Rationalise360f(elev);

      if (elev > 5.0f) {
        seekRate = 0.5f;
      } else if (elev < -5.0f) {
        seekRate = -0.5f;
      } else {
        seekRate = elev * 0.5f / 5.0f;
      }

      GunBoat->FireAngMomX = seekRate;
      GunBoat->FireAngleX = seekRate * 0.01666667f + GunBoat->FireAngleX;
    }

    if (ProcessTimer(&GunBoat->BurstTimer) != 0) {
      GunBoat->FireFrame = 0;
      GunBoat->BurstTimer = 1.0f;
    }

    if (GunBoat->BurstTimer >= 0.5f) {
      GunBoat->FireFrame--;
      if (GunBoat->FireFrame < 0) {
        if (frand() - 0.5f + GunBoat->AngleY + GunBoat->FireAngleX) {
          ;
        }

        NuMtxSetRotationX(&Mat, (s32)((frand() - 0.5f) * 30.0f + GunBoat->AngleY + GunBoat->FireAngleX) * f29);
        NuMtxRotateY(&Mat, (s32)((frand() - 0.5f) * 30.0f + GunBoat->AngleY) * f29);

        SetNuVec(0.0f, 0.0f, -200.0f);
        NuVecMtxRotate(&Vel, &Vel, &Mat);
        NuVecScaleAccum(3.0f, &diff, &Vel);

        if (AddGliderBullet(&Mat, &diff, &Vel, 1) != 0) {
          GunBoat->FireFrame = 6;
          MyGameSfx(0x60, &GunBoat->Position, 0x3fff);
        }
      }
    }
    break;

  case 2:
    if (ProcessTimer(&GunBoat->SunkTimer) != 0) {
      GunBoat->Position.y += 1.0f;
      if (GunBoat->Position.y > 0.0f) {
        GunBoat->Position.y = 0.0f;
        GunBoat->Action = 0;
      }
    } else {
      GunBoat->Position.y -= 1.0f;
    }

    if ((jonframe1 & 3) == 0) {
      DirVec.x = GunBoat->Position.x;
      DirVec.y = 0.0f;
      DirVec.z = GunBoat->Position.z;
      AddVariableShotDebrisEffect(GDeb[0x850 / sizeof(struct gdeb_s)].i, &DirVec,
                                  1, 0, 0);
    }
    break;
  }
}


// NGC MATCH
void ProcessGunBoats(void) {
  s32 i;

  for (i = 0; i < 4; i++) {
    if (GunBoatList[i].Active != 0) {
      ProcessGunBoat(&GunBoatList[i]);
    }
  }
}

// NGC MATCH
void InitGliderBombs(void) {
  memset(&GliderBombs, 0, 0x2a8);
  memset(EnemyGliderBombs, 0, 0x7f8);
}

static GLIDERBOMBSTRUCT *GrabGliderBomb(s32 Enemy) {
  GLIDERBOMBSTRUCT *bombs;
  s32 max;
  s32 i;

  if (Enemy) {
    bombs = EnemyGliderBombs;
    max = 30;
  } else {
    bombs = GliderBombs;
    max = 10;
  }

  for (i = 0; i < max; i++) {
    if (bombs[i].Life == 0) {
      return &bombs[i];
    }
  }
  return NULL;
}

static s32 AddGliderBomb(struct nuvec_s *Pos, struct nuvec_s *Vel, float AngY,
                         s32 Enemy, struct nuvec_s *TargetPoint,
                         struct nuvec_s *TargetVel, s32 Moving) {
  GLIDERBOMBSTRUCT *Bomb;

  Bomb = GrabGliderBomb(Enemy);
  if (Bomb != NULL) {
    Bomb->Life = 600;
    Bomb->Pos = *Pos;
    Bomb->Vel = *Vel;
    NuVecScale(1.0f / 60.0f, &Bomb->Vel, Vel);
    Bomb->AngY = AngY;
    if (Enemy) {
      Bomb->Gravity = -1.0f / 300.0f;
    } else {
      Bomb->Gravity = -1.0f / 300.0f;
      if (Level == 0x24) {
        Bomb->DropTimer = 0.0f;
        Bomb->TargetMoving = Enemy;
        Bomb->TargetPntr = TargetPoint;
        Bomb->TargetVelPntr = &v000;
      } else if (Moving) {
        Bomb->TargetMoving = 1;
        Bomb->DropTimer = 0.35f;
        Bomb->TargetPntr = TargetPoint;
        Bomb->TargetVelPntr = TargetVel;
      } else {
        Bomb->DropTimer = 0.35f;
        Bomb->Target = *TargetPoint;
        Bomb->TargetPntr = &Bomb->Target;
        Bomb->TargetVelPntr = &v000;
        Bomb->TargetMoving = Moving;
      }
    }
    return 1;
  }
  return 0;
}

// NGC MATCH
static void DrawGliderBombs(void) {
  struct numtx_s Mat;
  GLIDERBOMBSTRUCT *Bombs;
  s32 i, j, count;
  s32 Obj;
  float horiz;

  Bombs = GliderBombs;
  Obj = 0x4D;
  count = 10;
  if (ObjTab[0x4D].obj.special == NULL) return;

  for (i = 0; i < 2; i++) {
    for (j = 0; j < count; j++) {
      if (Bombs[j].Life != 0) {
        horiz = NuFsqrt(Bombs[j].Vel.x * Bombs[j].Vel.x + Bombs[j].Vel.z * Bombs[j].Vel.z);
        NuMtxSetRotationX(&Mat, NuAtan2D(Bombs[j].Vel.y * 100.0f, horiz * 100.0f));
        NuMtxRotateY(&Mat, (s32)(Bombs[j].AngY * 182.04445f));
        NuMtxTranslate(&Mat, &Bombs[j].Pos);
        NuRndrGScnObj(ObjTab[Obj].obj.scene->gobjs[ObjTab[Obj].obj.special->instance->objid], &Mat);
      }
    }
    Obj = 0x4D;
    Bombs = EnemyGliderBombs;
    count = 0x1e;
  }
}

// NGC MATCH
s32 CollideGliderBombs(struct nuvec_s *Pos, struct nuvec_s *Extent, s32 Enemy,
                       float Angle) {
    s32 i;
    s32 NumBombs;
    GLIDERBOMBSTRUCT *Bombs;
    GLIDERBOMBSTRUCT *Bomb;
    struct nuvec_s delta;
    float maxExt;
    float negMax;

    maxExt = Extent->y;
    if (Extent->x > maxExt) {
        maxExt = Extent->x;
    }
    if (Extent->z > maxExt) {
        maxExt = Extent->z;
    }
    negMax = -maxExt;

    if (Enemy != 0) {
        NumBombs = 30;
        Bombs = EnemyGliderBombs;
    } else {
        NumBombs = 10;
        Bombs = GliderBombs;
    }

    for (i = 0; i < NumBombs; i++) {
        Bomb = &Bombs[i];
        if (Bomb->Life != 0) {
            delta.x = Bomb->Pos.x - Pos->x;
            if (delta.x <= maxExt && delta.x >= negMax) {
                delta.y = Bomb->Pos.y - Pos->y;
                if (delta.y <= maxExt && delta.y >= negMax) {
                    delta.z = Bomb->Pos.z - Pos->z;
                    if (delta.z <= maxExt && delta.z >= negMax) {
                        NuVecRotateY(&delta, &delta,
                                     (int)(-Angle * 182.04445f));

                        delta.y /= Extent->y;
                        delta.x /= Extent->x;
                        delta.z /= Extent->z;

                        if (delta.y * delta.y + delta.x * delta.x +
                                delta.z * delta.z <=
                            1.0f) {
                            Bomb->Life = 0;
                            GliderBombCollisionPos = Bomb->Pos;
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}


// NGC MATCH
void InitBigGun(struct nuvec_s *Pos, float AngleY, int Type, float MinX,
                float MaxX, float MainY, float DeviationY) {
  BIGGUNSTRUCT *BigGun;
  struct nuvec_s vec;

  BigGun = &BigGunList[BigGunIndx++];
  if (BigGun->Active != 0) {
    return;
  }
  memset(BigGun, 0, 0x154);
  BigGun->FireAngMainY = MainY;
  BigGun->FireAngMinX = MinX;
  BigGun->FireAngMaxX = MaxX;
  BigGun->FireAngDeviationY = DeviationY;
  BigGun->Position = *Pos;
  BigGun->Action = 1;
  BigGun->LastAction = -1;
  BigGun->AngleY = BigGun->FireAngMainY;
  BigGun->FireAngleX = 0.0f;
  BigGun->FireAngleY = 0.0f;
  BigGun->Type = Type;
  NuVecSub(&vec, &BigGun->Position, SetNuVecPntr(0.0f, 7.75f, 11.17f));
  NuVecNorm(&vec, &vec);
  NuVecScale(1.5, &vec, &vec);
  NuVecAdd(&BigGun->TerrPos, &BigGun->Position, &vec);
  switch (Type) {
  case 0:
    BigGun->HitPoints = 6;
    BigGun->ExplosionEffect = 0x36;
    break;
  case 1:
    BigGun->HitPoints = 2;
    BigGun->ExplosionEffect = 0x37;
    break;
  case 2:
    BigGun->HitPoints = Type;
    BigGun->ExplosionEffect = 0x38;
    break;
  case 3:
    BigGun->HitPoints = 6;
    BigGun->ExplosionEffect = 0x39;
    break;
  case 4:
    BigGun->HitPoints = Type;
    BigGun->ExplosionEffect = 0x3a;
    break;
  case 5:
    BigGun->HitPoints = 4;
    BigGun->ExplosionEffect = 0x3b;
    break;
  }
  BigGun->Active = 1;
}

// NGC MATCH
void InitBigGuns(void) {
  memset(BigGunList, 0, 0xff0);
  BigGunIndx = 0;
  InitBigGun(SetNuVecPntr(0.0, 18.86, 30.58), 0.0, 0, -65.0, -10.0, 0.0, 60.0);
  InitBigGun(SetNuVecPntr(0.0, -1.68, 16.72), 0.0, 4, 10.0, 80.0, 0.0, 60.0);
  InitBigGun(SetNuVecPntr(-10.83, 6.32, 34.15), 0.0, 5, -65.0, -10.0, -90.0,
             60.0);
  InitBigGun(SetNuVecPntr(10.83, 6.32, 34.15), 0.0, 5, -65.0, -10.0, 90.0,
             60.0);
  InitBigGun(SetNuVecPntr(0.03, 23.71, 2.48), 0.0, 0, -65.0, -10.0, 180.0,
             60.0);
  InitBigGun(SetNuVecPntr(0.0, -25.5, -5.97), 0.0, 3, -30.0, 30.0, 180.0, 60.0);
  InitBigGun(SetNuVecPntr(10.0, 9.89, 14.46), 0.0, 1, -30.0, 30.0, 90.0, 30.0);
  InitBigGun(SetNuVecPntr(9.5, 9.89, 11.25), 0.0, 1, -30.0, 30.0, 90.0, 30.0);
  InitBigGun(SetNuVecPntr(9.0, 9.89, 7.84), 0.0, 1, -30.0, 30.0, 90.0, 30.0);
  InitBigGun(SetNuVecPntr(-10.0, 9.89, 14.46), 0.0, 2, -30.0, 30.0, -90.0,
             30.0);
  InitBigGun(SetNuVecPntr(-9.5, 9.89, 11.25), 0.0, 2, -30.0, 30.0, -90.0, 30.0);
  InitBigGun(SetNuVecPntr(-9.0, 9.89, 7.84), 0.0, 2, -30.0, 30.0, -90.0, 30.0);
}

// NGC MATCH
void DrawBigGun(BIGGUNSTRUCT *BigGun) {
  struct nuvec_s ScaleVec = {1.5f, 1.5, 1.5f};

  NuMtxSetScale(&mTEMP, &ScaleVec);
  NuMtxRotateX(&mTEMP, (int)(BigGun->FireAngleX * 182.04445f));
  NuMtxRotateY(&mTEMP,
               (int)((BigGun->AngleY + BigGun->FireAngleY) * 182.04445f));
  NuMtxTranslate(&mTEMP, &BigGun->Position);
}

// NGC MATCH
float GetBigGunBestTarget(float Best, struct nuvec_s **TargetPos,
                          struct nuvec_s **Vel, s32 *Moving) {
  struct nuvec_s CamDir;
  struct nuvec_s Rel;
  struct nuvec_s ShootDir;
  s32 i;
  float Dot;
  float Mag;

  NuVecMtxRotate(&CamDir, SetNuVecPntr(0.0f, 0.0f, 1.0f), &GameCam[0].m);
  for (i = 0; i < 0xd; i++) {
    if (BigGunList[i].Active != 0) {
      if (BigGunList[i].HitPoints > 0) {
        NuVecSub(&Rel, &BigGunList[i].Position, &GameCam[0].pos);
        Mag = NuVecMag(&Rel);
        if (Mag > 10.0f) {
          NuVecScale((1.0f / Mag), &Rel, &Rel);
          Dot = DotProduct(&CamDir, &Rel);
          if (Dot > Best) {
            NuVecSub(&ShootDir, &PlayerGlider.ApparentPosition, &BigGunList[i].Position);
            NuVecNorm(&ShootDir, &ShootDir);
            NuVecScale(30.0f, &ShootDir, &ShootDir);
            if (NewRayCast(&BigGunList[i].Position, &ShootDir, 0.0f) == 0) {
              *TargetPos = &BigGunList[i].Position;
              Best = Dot;
              *Vel = &v000;
              *Moving = 0;
            }
          }
        }
      }
    }
  }
  return Best;
}

// NGC MATCH
void ProcessBigGun(BIGGUNSTRUCT *BigGun) {
  struct nuvec_s delta;
  struct nuvec_s diff;
  struct nuvec_s RotResult;
  struct numtx_s Mat;
  struct nuvec_s Vel;
  struct nuvec_s Pos;
  float f31;
  float f30;
  float f29;
  float f28;
  float dist;
  float seekRate;
  s32 angY;

  SetNuVecPntr(1.0f, 1.0f, 1.0f);
  CollideGliderBombs(&BigGun->Position, NULL, 0, 4.0f);

  if (CollideGliderBombs(&BigGun->Position, NULL, 0, 4.0f) != 0) {
    AddGameDebris(0x1a, &BigGun->Position);
    if (BigGun->HitPoints > 0) {
      BigGun->Action = 2;
    }
  }

  if (BigGun->LastAction != BigGun->Action) {
    switch (BigGun->Action) {
    case 1:
      BigGun->BurstTimer = 4.0f;
      break;
    case 2:
      if (TimeTrial != 0) {
        BigGun->HitPoints = 0;
        if (TimeTrial != 0) {
          TimeTrialWait += 5.0f;
        }
      }
      break;
    }
  }

  BigGun->LastAction = BigGun->Action;
  dist = NuVecDist(&PlayerGlider.Position + 0x124, &BigGun->Position, &diff);

  NuVecScaleAccum(dist / 250.0f, &diff, &PlayerGlider.Velocity + 0x130);

  switch (BigGun->Action) {
  case 0:
    BigGun->Action = 1;
    break;

  case 1:
    f29 = 182.04445f;

    NuMtxSetRotationY(&Mat, (s32)(-BigGun->AngleY * f29));
    NuVecMtxRotate(&RotResult, &diff, &Mat);

    f31 = 182.04445f;
    angY = NuAtan2D(RotResult.x * f31, RotResult.z * f31);
    angY ^= 0x8000;

    {
      float angFloat;
      angFloat = (float)(s32)(u16)angY;
      LimitAng360f(&BigGun->FireAngMinX);
      f28 = angFloat / f29;
    }

    {
      float negAng;
      negAng = -f28;
      NuVecRotateY(&RotResult, &RotResult, (s32)(negAng * f29));
    }

    {
      s32 angXi;
      angXi = (s32)NuAtan2D(RotResult.y * -1.0f, RotResult.z * f31);
      angXi ^= 0x8000;
      LimitAng360f(&BigGun->FireAngMaxX);
    }

    {
      float seekYaw;
      seekYaw = f28 - BigGun->AngleY;
      seekYaw = Rationalise360f(seekYaw);

      if (seekYaw > 5.0f) {
        seekRate = 0.5f;
      } else if (seekYaw < -5.0f) {
        seekRate = -0.5f;
      } else {
        seekRate = seekYaw * 0.5f / 5.0f;
      }

      BigGun->FireAngMomY = seekRate;
      BigGun->AngleY = seekRate * 0.01666667f + BigGun->AngleY;
    }

    {
      float elev;
      elev = f28 - BigGun->FireAngleX;
      elev = Rationalise360f(elev);

      if (elev > 5.0f) {
        seekRate = 0.5f;
      } else if (elev < -5.0f) {
        seekRate = -0.5f;
      } else {
        seekRate = elev * 0.5f / 5.0f;
      }

      BigGun->FireAngMomX = seekRate;
      BigGun->FireAngleX = seekRate * 0.01666667f + BigGun->FireAngleX;
    }

    if (ProcessTimer(&BigGun->BurstTimer) != 0) {
      if (BigGun->Seen != 0) {
        BigGun->FireFrame = 0;
        BigGun->BurstTimer = 5.0f;
      }
    }

    if (BigGun->BurstTimer >= 1.0f) {
      BigGun->FireFrame--;
      if (BigGun->FireFrame < 0) {
        Pos = diff;

        NuMtxSetRotationX(&Mat, (s32)((frand() - 0.5f) * 30.0f + BigGun->FireAngleX + BigGun->AngleY) * f29);
        NuMtxRotateY(&Mat, (s32)((frand() - 0.5f) * 30.0f + BigGun->AngleY + BigGun->FireAngleX) * f29);

        Vel = SetNuVec(0.0f, 0.0f, -180.0f);
        NuVecMtxRotate(&Vel, &Vel, &Mat);
        NuVecScaleAccum(3.0f, &Pos, &Vel);

        if (AddGliderBullet(&Mat, &Pos, &Vel, 1) != 0) {
          BigGun->FireFrame = 5;
          MyGameSfx(0x60, &Pos, 0x3fff);
        }
      }
    }
    break;

  case 2:
    {
      struct nuvec_s debrisPos;

      debrisPos = BigGun->Position;
      AddGameDebrisRot(BigGun->ExplosionEffect, &debrisPos, 1, 0, 0);
    }
    break;
  }
}


// NGC MATCH
void ProcessBigGuns(void) {
  s32 i;

  for (i = 0; i < 0xc; i++) {
    if (BigGunList[i].Active != 0) {
      ProcessBigGun(&BigGunList[i]);
    }
  }
}

// NGC MATCH
void DrawBigGuns(void) {
  s32 i;

  for (i = 0; i < 0xc; i++) {
    if (BigGunList[i].Active != 0) {
      DrawBigGun(&BigGunList[i]);
    }
  }
}

// NGC MATCH
s32 CollideWithBattleShip(struct nuvec_s *ObjPos, BATTLESHIPSTRUCT *Ship,
                          float Rad) {
  struct nuvec_s *Scale;
  struct nuvec_s *Pos;
  float Max;
  float NegMax;
  struct nuvec_s Rel;

  Scale = &BattleShipCollScale;
  Pos = &Ship->Position;
  if (Scale->x > Scale->y) {
    Max = Scale->x;
  } else {
    Max = Scale->y;
  }
  if (Scale->z > Max) {
    Max = Scale->z;
  }
  NegMax = -Max;
  Rel.x = ObjPos->x - Pos->x;
  if ((Rel.x <= Max) && (Rel.x >= NegMax)) {
    Rel.y = ObjPos->y - Pos->y;
    if ((Rel.y <= Max) && (Rel.y >= NegMax)) {
      Rel.z = ObjPos->z - Pos->z;
      if ((Rel.z <= Max) && (Rel.z >= NegMax)) {
        NuVecRotateY(&Rel, &Rel, (s32)(-Ship->AngleY * 182.04445f));
        Rel.x /= Scale->x;
        Rel.y /= Scale->y;
        Rel.z /= Scale->z;
        if (Rel.x * Rel.x + Rel.y * Rel.y + Rel.z * Rel.z <= 1.0f) {
          if (InvincibilityCHEAT != 0) {
            Ship->KillMeNow = 1;
          }
          return 1;
        }
      }
    }
  }
  return 0;
}

// NGC MATCH
s32 CollideWithBattleShips(struct nuvec_s *ObjPos, float Rad) {
  s32 i;

  for (i = 0; i < 6; i++) {
    if ((BattleShipList[i].Active != 0 &&
         (CollideWithBattleShip(ObjPos, &BattleShipList[i], Rad) != 0))) {
      return 1;
    }
  }
  return 0;
}

// NGC MATCH
void InitBattleShip(struct nuvec_s *Pos, float AngleY, int Character) {
  BATTLESHIPSTRUCT *BattleShip;

  BattleShip = &BattleShipList[Character];
  if (BattleShip->Active == 0) {
    memset(&BattleShipList[Character], 0, 0x554);
    if (MyInitModelNew(&BattleShip->MainDraw, 0x65, 0x22, 0, NULL,
                       &BattleShip->Position) != 0) {
      BattleShip->Position = *Pos;
      BattleShip->DestY = Pos->y;
      BattleShip->AngleY = AngleY;
      BattleShip->TiltX = 0.0f;
      BattleShip->TiltZ = 0.0f;
      BattleShip->DestTiltX = 0.0f;
      BattleShip->DestTiltZ = 0.0f;
      BattleShip->Seek = 1.0f;
      BattleShip->Active = 1;
      BattleShip->HitPoints = 3;
      BattleShip->FireTimer[0] = frand() * 4.0f;
      BattleShip->FireTimer[1] = (frand() * 4.0f);
      BattleShip->GooScale = SetNuVec(0.0f, 0.0f, 0.0f);
      BattleShip->GooTimer.x = (float)(Character) / 6.0f;
      BattleShip->GooTimer.y = ((float)(Character + 1) / 6.0f);
      BattleShip->GooSpeed =
          (BattleShip->GooTimer.x * 0.1f + 0.2f) * 0.01666667f;
      if (1.0f <= BattleShip->GooTimer.y) {
        BattleShip->GooTimer.y = (BattleShip->GooTimer.y - 1.0f);
      }
      BattleShip->GooTimer.z = (float)(Character + 2) / 6.0f;
      if (1.0f <= BattleShip->GooTimer.z) {
        BattleShip->GooTimer.z = (BattleShip->GooTimer.z - 1.0f);
      }
    }
  }
}

// NGC MATCH
void InitBattleShips(void) {
  memset(BattleShipList, 0, 0x1ff8);
  InitBattleShip(SetNuVecPntr(-7.0f, 0.0f, 15.0f), -150.0f, 0);
  InitBattleShip(SetNuVecPntr(-7.33, 0.0f, 73.5f), -45.0f, 1);
  InitBattleShip(SetNuVecPntr(31.0f, 0.0f, 109.0f), -35.0f, 2);
  InitBattleShip(SetNuVecPntr(94.0f, 0.0f, 54.0f), 90.0f, 3);
  InitBattleShip(SetNuVecPntr(60.0f, 0.0f, -6.0f), 160.0f, 4);
  InitBattleShip(SetNuVecPntr(20.0f, 0.0f, -3.0f), -150.0f, 5);
}

// NGC MATCH
void DrawBattleShip(BATTLESHIPSTRUCT *BattleShip) {
  NuMtxSetRotationX(&mTEMP, (int)(BattleShip->TiltX * 182.04445f));
  NuMtxRotateZ(&mTEMP, (int)(BattleShip->TiltZ * 182.04445f));
  NuMtxRotateY(&mTEMP, (int)(BattleShip->AngleY * 182.04445f));
  NuMtxTranslate(&mTEMP, &BattleShip->Position);
  BattleShip->Seen =
      MyDrawModelNew(&BattleShip->MainDraw, &mTEMP, BattleShip->Locator);
}

// NGC MATCH
float GetBattleShipBestTarget(float Best, struct nuvec_s **TargetPos,
                              struct nuvec_s **Vel, s32 *Moving) {
  struct nuvec_s CamDir;
  struct nuvec_s Rel;
  s32 i;
  float Dot;
  float Mag;

  NuVecMtxRotate(&CamDir, SetNuVecPntr(0.0f, 0.0f, 1.0f), &GameCam[0].m);
  for (i = 0; i < 6; i++) {
    if (BattleShipList[i].Active != 0) {
      if (BattleShipList[i].Seen > 0) {
        NuVecSub(&Rel, &BattleShipList[i].Position, &GameCam[0].pos);
        Mag = NuVecMag(&Rel);
        if (Mag > 10.0f) {
          NuVecScale((1.0f / Mag), &Rel, &Rel);
          Dot = DotProduct(&CamDir, &Rel);
          if (Dot > Best) {
            *TargetPos = &BattleShipList[i].Position;
            *Vel = &v000;
            *Moving = 0;
            Best = Dot;
          }
        }
      }
    }
  }
  return Best;
}

// NGC MATCH
void ProcessBattleShips(void) {
  s32 i;

  for (i = 0; i < 6; i++) {
    if (BattleShipList[i].Active != 0) {
      ProcessBattleShip(&BattleShipList[i]);
    }
  }
}

// NGC MATCH
struct nuvec_s *GetWeatherBossPos(void) { return &WeatherBoss.Position; }

// NGC MATCH
void InitWeatherBoss_a(void) {
  BOSSSTRUCT *Boss;
  struct nuvec_s Start;
  u16 StartAngle;

  Boss = &WeatherBoss;
  memset(&WeatherBoss, 0, 0x65c);
  if ((MyInitModelNew(&Boss->MainDraw, 0x86, 0x32, 0, NULL, &Boss->Position) !=
       0) &&
      (MyInitModelNew(&Boss->BonesDraw, 0xbe, 0xe, 0, NULL, &Boss->Position) !=
       0)) {
    BazookaIconOn = 0;
    Boss->Active = 1;
    Boss->ChestSoundBTimer = 100000.0f;
    Boss->HitPoints[0] = 100;
    Boss->HitPoints[1] = 100;
    Boss->HitPoints[2] = 100;
    Boss->HitPoints[3] = 100;
    Boss->Action = 0;
    Boss->OldAction = -1;
    Boss->MainSpline.Spline = SplTab[0x19].spl;
    if (SplTab[0x19].spl != NULL) {
      Boss->MainSpline.Cur = 0.0f;
      Boss->MainSpline.Inc = 0.002f;
      Boss->MainSpline.Nex = 0.0f;
      PointAlongSpline(SplTab[0x19].spl, 0.0f, &Start, &StartAngle, NULL);
      Boss->AngleY = StartAngle / 182.04445f;
      Boss->MainSpline.CurPos = Start;
      Boss->MainSpline.NexPos = Start;
      Boss->MainSpline.LookaheadDist = 128.0f;
      Boss->Position = Start;
      Boss->BaseAngleY = Boss->AngleY;
      Boss->DestAngleY = Boss->AngleY;
      Boss->Position = SetNuVec(0.0f, 37.0f, 0.0f);
      Boss->Active = 1;
      Boss->PossYDest = Boss->Position.y;
      CamOveride = &Boss->Position;
      Boss->Distance = -96.0f;
      Boss->DistanceDest = -96.0f;
    }
  }
}

// NGC MATCH
void InitWeatherBoss(void) { InitWeatherBoss_a(); }

// NGC MATCH
void ProcessBazookaToken(void) {
  struct nuvec_s Pos;
  float f31;

  if (BazookaIconOn == 0) return;

  f31 = WeatherBoss.ActionTimer / WeatherBoss.Locator[WeatherBoss.Action]._01;
  NuVecScale(1.0f - f31, &Pos, &BazookaTokenPos);
  NuVecScaleAccum(f31, &Pos, &PlayerGlider.ApparentPosition);
  Pos.y = f31 * 3.0f + Pos.y;
  BazookaTokenCurrentPos = Pos;
  MyAnimateModelNew(&IconMainDraw, 0.5f);
}


// NGC MATCH
void DrawBazookaToken(void) {
  if (BazookaIconOn != 0) {
    NuMtxSetTranslation(&mTEMP, &BazookaTokenCurrentPos);
    MyDrawModelNew(&IconMainDraw, &mTEMP, NULL);
  }
}

// NGC MATCH
s32 WeatherBossNextAction(void) {
  s32 NextAction;
  s32 Action;
  s32 i;

  if (WeatherBoss.Action == 0) {
    NextAction = 0;
    Action = WeatherBoss.LastNonSeekAction;
    i = 1;

    do {
      Action++;
      if (Action > 4) {
        Action = 1;
      }

      if (WeatherBoss.HitPoints[Action - 1] > 0) {
        NextAction = Action;
      }

      if (i++ > 3) {
        return NextAction;
      }

    } while (NextAction == 0);

    return NextAction;
  }

  WeatherBoss.LastNonSeekAction = WeatherBoss.Action;
  return 0;
}

// NGC MATCH
void ProcessWeatherBoss_a(struct BOSSSTRUCT *Boss) {
  s32 Type;
  s32 VulnerableOn;
  s32 LastOn;
  s32 Percent;
  static s32 LastVulnerableOn_523;
  static s32 WasLastOn_522;
  static struct nuvec_s VulnerablePos_524;
  struct nuvec_s Vel;
  struct nuvec_s FirePos;
  struct nuvec_s FirePos2;
  struct nuvec_s GliderPos;
  struct nuvec_s Vel2;
  struct nuvec_s local_90;
  struct nuvec_s local_80;
  struct nuvec_s nStack_70;
  struct nuvec_s local_60;
  struct nuvec_s *VulnerableA;
  struct nuvec_s *VulnerableB;
  float VulnerableRad;
  s32 VulnerableSection;

  Type = GetCurrentWeatherBossObjectives();
  Boss->HitSoundFrame--;
  if (Boss->HitSoundFrame < 0) {
    Boss->HitSoundFrame = 0;
  }
  if ((Boss->ChestSoundBTimer < 100.0f) &&
      (ProcessTimer(&Boss->ChestSoundBTimer) != 0)) {
    Boss->ChestSoundBTimer = 100000.0f;
    ElectricalPosition = PlayerGlider.Position;
    MyGameSfx(0xb4, &ElectricalPosition, CHESTATTACKBVOL);
  }
  if ((Type < 0x32) && (ProcessTimer(&WeatherBossSkeletonGlitchTimer) != 0)) {
    WeatherBossSkeletonTimer = frand() * 0.5f + WBSKELTIME;
    WeatherBossSkeletonGlitchTimer =
        (frand() * 0.5f + 0.15f) + ((frand() * 2.0f) * (float)(Type)) / 50.0f;
  }
  ProcessTimer(&WeatherBossSkeletonTimer);
  LastOn = 0;
  MyAnimateModelNew(&Boss->MainDraw, 0.5f);
  MyAnimateModelNew(&Boss->BonesDraw, 0.1f);
  if (Boss->OldAction != Boss->Action) {
    switch (Boss->Action) {
    case 0:
      Boss->DestAngleY = Boss->BaseAngleY;
      if (WBIntroOn == 0) {
        Boss->DistanceDest = 96.0f;
      }
      Boss->PossYDest = 37.0f;
      MyChangeAnim(&Boss->MainDraw, 0x22);
      VulnerableA = NULL;
      Boss->ActionTimer = 3.0f;
      Boss->NextAction = WeatherBossNextAction();
      switch (Boss->NextAction) {
      case 1:
        SetNewMaskStuff(0, &Boss->Position, &WBMASKONHIGH, 0.0f, 360.0f, 0.0f,
                        0, 1, WBLOLOSCALE, WBLOLOTILTX);
        SetNewMaskStuff(0, (struct nuvec_s *)&Boss->Locator[0]._30, &WBMASKLEFT,
                        WBMASKLEFTRAD, 360.0f, 0.5f, 0, 0, WBLOLOSCALE,
                        WBLOLOTILTX);
        break;
      case 3:
        SetNewMaskStuff(0, &Boss->Position, &WBMASKONHIGH, 0.0f, 360.0f, 0.0f,
                        0, 1, WBLOLOSCALE, WBLOLOTILTX);
        SetNewMaskStuff(0, (struct nuvec_s *)&Boss->Locator[1]._30,
                        &WBMASKRIGHT, WBMASKRIGHTRAD, 360.0f, 0.5f, 0, 0,
                        WBLOLOSCALE, WBLOLOTILTX);
        break;
      case 2:
        SetNewMaskStuff(0, &Boss->Position, &WBMASKONHIGH, 0.0f, 360.0f, 0.0f,
                        0, 1, WBLOLOSCALE, WBLOLOTILTX);
        SetNewMaskStuff(0, (struct nuvec_s *)&Boss->Locator[2]._30, &WBMASKEYES,
                        WBMASKEYESRAD, 360.0f, 0.5f, 0, 0, WBLOLOSCALE,
                        WBLOLOTILTX);
        break;
      case 4:
        SetNewMaskStuff(0, &Boss->Position, &WBMASKONHIGH, 0.0f, 360.0f, 0.0f,
                        0, 1, WBLOLOSCALE, WBLOLOTILTX);
        SetNewMaskStuff(0, (struct nuvec_s *)&Boss->Locator[4]._30,
                        &WBMASKCHEST, WBMASKCHESTRAD, 360.0f, 0.5f, 0, 0,
                        WBLOLOSCALE, WBLOLOTILTX);
        break;
      }
      break;
    case 5:
      MyGameSfx(0xb6, NULL, CHESTATTACKAVOL);
      FlyingLevelVictoryDance = 1;
      Boss->DistanceDest = 0.0f;
      MyResetAnimPacket(&Boss->BonesDraw, 0xe);
      Boss->ActionTimer = 3.0f;
      BazookaTokenPos = Boss->Position;
      BazookaIconOn = (Game.hub[Hub].flags ^ 4) >> 2 & 1; // check
      BazookaTokenCurrentPos = Boss->Position;
      VulnerableA = NULL;
      MyInitModelNew(&IconMainDraw, 0xa2, 0x22, 0, NULL,
                     &BazookaTokenCurrentPos);
      break;
    case 1:
      WasLastOn_522 = 0;
      Boss->DistanceDest = 96.0f;
      Boss->PossYDest = 37.0f;
      WasLastOn_522 = 0;
      MyChangeAnim(&Boss->MainDraw, 0x32);
      break;
    case 3:
      Boss->DistanceDest = 96.0f;
      Boss->FireTimer = 0.0f;
      Boss->PossYDest = 37.0f;
      MyChangeAnim(&Boss->MainDraw, 0x39);
      break;
    case 2:
      Boss->FireSide = 0;
      Boss->DistanceDest = 32.0f;
      Boss->FireTimer = 0.0f;
      MyChangeAnim(&Boss->MainDraw, 0x1b);
      break;
    case 4:
      Boss->Unleashed = 0;
      Boss->DistanceDest = 128.0f;
      Boss->PossYDest = 37.0f;
      MyChangeAnim(&Boss->MainDraw, 0);
      break;
    }
  }
  Boss->OldAction = Boss->Action;
  switch (Boss->Action) {
  case 5:
    if ((Boss->BonesDraw.Anim.flags & 1) != 0) {
      ChrisBigBossDead = 1;
      Boss->Active = 0;
      BazookaIconOn = 0;
      return;
    }
    break;
  case 0:
    if ((WBIntroOn == 0) && (ProcessTimer(&Boss->ActionTimer) != 0)) {
      Boss->Action = Boss->NextAction;
      SetNewMaskStuff(0, &Boss->Position, &WBMASKONHIGH, 2.0f, 360.0f, 4.0f, 0,
                      1, WBLOLOSCALE, WBLOLOTILTX);
    }
    break;
  case 1:
    if (((Boss->MainDraw.Anim.action == 0x32) &&
         (Boss->MainDraw.Anim.anim_time >= WBLeftStartFrame)) &&
        (Boss->MainDraw.Anim.anim_time <= WBLeftStopFrame)) {
      VulnerableB = VulnerableA = (struct nuvec_s *)&Boss->Locator[0]._30;
      VulnerableRad = 9.0f;
      VulnerableSection = 0;
    } else {
      VulnerableA = NULL;
    }
    if ((Boss->MainDraw.Anim.action == 0x32) &&
        (Boss->MainDraw.Anim.anim_time <= WBLeftStopTurnFrame)) {
      Boss->DestAngleY =
          PlayerGlider.Position.x * WBANGSCALE + Boss->BaseAngleY + WBANGOFF;
    } else {
      Boss->DestAngleY = Boss->BaseAngleY;
    }
    if (((Boss->MainDraw.Anim.action == 0x32) &&
         (Boss->MainDraw.Anim.anim_time >= WBLeftStartFrame)) &&
        (Boss->MainDraw.Anim.anim_time <= WBLeftStopFrame)) {
      NuVecRotateY(&Vel, SetNuVecPntr(0.0f, 0.0f, -110.0f),
                   (s32)((Boss->AngleY + 45.0f) * 182.04445f));
      NuVecMtxTransform(&FirePos, &WBLeftFirePos, Boss->Locator);
      Vel.z -= Level_GliderSpeed;
      FireWBBolt(&FirePos, &Vel, 2, 5.0f, Boss->Action);
      if (WasLastOn_522 == 0) {
        ElectricalPosition = PlayerGlider.Position;
        MyGameSfx(0xb3, &ElectricalPosition, BALLATTACKVOL);
      }
      LastOn = 1;
    } else {
      if (WasLastOn_522 != 0) {
        GliderPos = PlayerGlider.Position;
        NuVecMtxTransform(&FirePos2, &WBLeftFirePos, Boss->Locator);
        NuVecSub(&Vel2, &GliderPos, &FirePos2);
        NuVecNorm(&Vel2, &Vel2);
        NuVecScale(WBSNOWBALLSPEED, &Vel2, &Vel2);
        Vel2.z -= Level_GliderSpeed;
        FireWBBolt(&FirePos2, &Vel2, 2, 5.0f, Boss->Action);
      }
    }
    if (((Boss->MainDraw.Anim.flags & 1) != 0) ||
        (Boss->HitPoints[Boss->Action + -1] < 1)) {
      Boss->Action = WeatherBossNextAction();
      Boss->DestAngleY = Boss->BaseAngleY;
    }
    break;
  case 3:
    if ((Boss->MainDraw.Anim.action == 0x39) &&
        (Boss->MainDraw.Anim.anim_time <= 246.0f)) {
      Boss->DestAngleY =
          PlayerGlider.Position.x * WBANGSCALE2 + Boss->BaseAngleY + WBANGOFF2;
    } else {
      Boss->DestAngleY = Boss->BaseAngleY;
    }
    if (Boss->MainDraw.Anim.action != 0x39) {
      VulnerableA = NULL;
    } else {
      if ((Boss->MainDraw.Anim.anim_time >= 60.0f) &&
          (Boss->MainDraw.Anim.anim_time <= 247.0f)) {
        local_90 = *(struct nuvec_s *)&Boss->Locator[1]._30;
        local_80 = PlayerGlider.Position;
        Boss->FireTimer = 0.25f;
        NuVecSub(&nStack_70, &local_80, &local_90);
        NuVecNorm(&nStack_70, &nStack_70);
        NuVecScale(WBSNOWCONESPEED, &nStack_70, &nStack_70);
        nStack_70.z -= Level_GliderSpeed;
        FireWBBolt(&local_90, &nStack_70, 3, 1.5f, Boss->Action);
        ElectricalPosition = PlayerGlider.Position;
        MyGameSfxLoop(0xb7, &ElectricalPosition, BEAMVOL);
      }
      if (((Boss->MainDraw.Anim.action == 0x39) &&
           (Boss->MainDraw.Anim.anim_time >= 60.0f)) &&
          (Boss->MainDraw.Anim.anim_time <= 247.0f)) {
        VulnerableA = (struct nuvec_s *)&Boss->Locator[1]._30;
        VulnerableSection = 2;
        VulnerableB = VulnerableA;
        VulnerableRad = 9.0f;
      } else {
        VulnerableA = NULL;
      }
    }
    if (((Boss->MainDraw.Anim.flags & 1) != 0) ||
        (Boss->HitPoints[Boss->Action + -1] < 1)) {
      Boss->Action = WeatherBossNextAction();
      Boss->DestAngleY = Boss->BaseAngleY;
    }
    break;
  case 2:
    Boss->PossYDest = PlayerGlider.Position.y - 3.5f;
    Boss->DestAngleY = PlayerGlider.Position.x * WBANGSCALE3 + Boss->BaseAngleY;
    if (((Boss->MainDraw.Anim.action == 0x1b) &&
         (Boss->MainDraw.Anim.anim_time >= 46.0f)) &&
        (Boss->MainDraw.Anim.anim_time <= EYESTOPTIME)) {
      VulnerableA = (struct nuvec_s *)&Boss->Locator[2]._30;
      VulnerableB = (struct nuvec_s *)&Boss->Locator[3]._30;
      VulnerableSection = 1;
      VulnerableRad = 8.0f;
      if (ProcessTimer(&Boss->FireTimer) != 0) {
        Boss->FireTimer = 0.25f;
        local_60 = *(struct nuvec_s *)&Boss->Locator[Boss->FireSide + 2]._30;
        Percent = (frand() < 0.5f) ? 0 : 1;
        FireWBBolt(&local_60,
                   SetNuVecPntr((frandPN() * EYEBOLTFIRESCALEX),
                                frandPN() * EYEBOLTFIRESCALEY + EYEBOLTFIREY,
                                EYEBOLTFIRESPEED - Level_GliderSpeed),
                   Percent, 8.0f, Boss->Action);
        Boss->FireSide = Boss->FireSide + 1U & 1;
        MyGameSfx(0x85, NULL, EYEATTACKVOL);
      }
    }
    if (((Boss->MainDraw.Anim.flags & 1) != 0) ||
        (Boss->HitPoints[Boss->Action + -1] < 1)) {
      Boss->Action = WeatherBossNextAction();
    }
    break;
  case 4:
    if (((Boss->Unleashed == 0) && (Boss->MainDraw.Anim.action == 0)) &&
        (Boss->MainDraw.Anim.anim_time >= 48.0f)) {
      local_60 = *(struct nuvec_s *)&Boss->Locator[4]._30;
      local_60.z += 10.0f;
      Boss->Unleashed = 1;
      VulnerableA = VulnerableB = (struct nuvec_s *)&Boss->Locator[4]._30;
      VulnerableRad = 15.0f;
      VulnerableSection = 3;
      UnleashLighteningHail(&local_60, (s32)(frand() * 3.0f));
      MyGameSfx(0xb6, NULL, CHESTATTACKAVOL);
      Boss->ChestSoundBTimer = CHESTATTACKSOUNDTIME;
    }
    if (((Boss->MainDraw.Anim.flags & 1) != 0) ||
        (Boss->HitPoints[Boss->Action + -1] < 1)) {
      Boss->Action = WeatherBossNextAction();
    }
    break;
  }
  VulnerableOn = 0;
  SeekAngHalfLife360f(&Boss->AngleY, Boss->DestAngleY, 0.5f, 0.016666668f);
  SeekHalfLifeLim(&Boss->Distance, Boss->DistanceDest, WBDISTANCESPEED,
                  WBDISTANCETIME, 0.016666668f);
  SeekHalfLifeLim(&Boss->Position.y, Boss->PossYDest, WBDISTANCEYSPEED,
                  WBDISTANCEYTIME, 0.016666668f);
  Boss->Position.z = PlayerGlider.Position.z - Boss->Distance;
  if ((VulnerableA != NULL) && (0 < Boss->HitPoints[VulnerableSection])) {
    VulnerableOn = 1;
    NuVecScale(0.5f, &Vel, VulnerableA);
    NuVecScaleAccum(0.5f, &Vel, VulnerableB);
    VulnerablePos_524 = Vel;
    if (CollideGliderBullets(&Vel, VulnerableRad, 0, 1.0f, 0, 0) != 0) {
      if (Boss->HitSoundFrame == 0) {
        MyGameSfx(0xb8, NULL, EYEATTACKVOL);
      }
      if (WeatherBossSkeletonTimer == 0.0f) {
        WeatherBossSkeletonTimer = frand() * WBSKELTIMERAND + WBSKELTIME;
      }
      Boss->HitPoints[VulnerableSection]--;
      if (Boss->HitPoints[VulnerableSection] < 0) {
        Boss->HitPoints[VulnerableSection] = 0;
      }
    }
  }
  if (VulnerableOn != 0) {
    if (LastVulnerableOn_523 == 0) {
      SetNewMaskStuff(1, (struct nuvec_s *)&WeatherBossCamMtx._30,
                      &WBAKUMASKONHIGH, 0.0f, -360.0f, 0.0f, 0, 1, WBLOLOSCALE,
                      90.0f);
      SetNewMaskStuff(1, &VulnerablePos_524, SetNuVecPntr(0.0f, 0.0f, 10.0f),
                      VulnerableRad, -360.0f, 0.5f, 0, 0, WBAKUAKUSCALE, 90.0f);
    }
  } else if (LastVulnerableOn_523 != 0) {
    SetNewMaskStuff(1, (struct nuvec_s *)&WeatherBossCamMtx._30,
                    &WBAKUMASKONHIGH, 0.0f, -360.0f, 0.5f, 0, 1, WBLOLOSCALE,
                    90.0f);
  }
  LastVulnerableOn_523 = VulnerableOn;
  if ((Boss->Dead == 0) && (Boss->HitPoints[0] + Boss->HitPoints[1] +
                                Boss->HitPoints[2] + Boss->HitPoints[3] <
                            1)) {
    WeatherBossDead = 1;
    Boss->Dead = 1;
    WeatherBossSkeletonTimer = 0.0f;
    Boss->Action = 5;
  }
  WasLastOn_522 = LastOn;
}

// NGC MATCH
void ProcessWeatherBoss(void) {
  if (WeatherBoss.Active != 0) {
    ProcessWeatherBoss_a(&WeatherBoss);
  }
}

// NGC MATCH
void DrawWeatherBoss_a(BOSSSTRUCT *Boss) {
  struct nuvec_s Pos;

  Pos = (Boss->Position);
  NuMtxSetScale(&mTEMP, SetNuVecPntr(WBSCALE, WBSCALE, WBSCALE));
  NuMtxRotateY(&mTEMP, (Boss->AngleY * 182.04445f));
  NuMtxTranslate(&mTEMP, &Pos);
  if (Boss->Action == 5) {
    Boss->Seen = MyDrawModelNew(&Boss->BonesDraw, &mTEMP, Boss->Locator);
  } else {
    Boss->Seen = MyDrawModelNew(&Boss->MainDraw, &mTEMP, Boss->Locator);
  }
}

// NGC MATCH
void DrawWeatherBoss(void) {
  if (WeatherBoss.Active != 0) {
    DrawWeatherBoss_a(&WeatherBoss);
  }
}

// NGC MATCH
void DrawGliderTarget(void) {
  struct nuvec_s Centre;
  struct nuvec_s nStack_20;
  s32 Obj = 0x72;

  NuVecSub(&nStack_20, &GliderTargetPos, &GameCam[0].pos);
  NuVecScale(5.0f / NuVecMag(&nStack_20), &Centre, &nStack_20);
  NuVecAdd(&Centre, &Centre, (struct nuvec_s *)&GameCam[0].m._30);
  if (ObjTab[Obj].obj.special != NULL) {
    mTEMP = GameCam[0].m;
    NuMtxPreScale(&mTEMP, SetNuVecPntr(0.2f, 0.2f, 0.0f));
    *(struct nuvec_s *)&mTEMP._30 = Centre;
    NuRndrGScnObj((ObjTab[Obj].obj.scene)
                      ->gobjs[(ObjTab[Obj].obj.special)->instance->objid],
                  &mTEMP);
  }
}

// NGC MATCH
void DrawTorpedoTarget(void) {
  struct nuvec_s ScaledPos;
  struct nuvec_s ProjPos[4];
  struct nuvec_s Centre;
  struct nuvec_s Corner[4];
  struct nuvec_s Direction;
  struct nuvec_s Delta;
  struct nuvec_s *Target;
  struct nuvec_s *Vel;
  s32 Moving;
  s32 Obj;
  s32 Angle;
  s32 i;
  float ratio;
  float MiddleScale;
  float RocketScale;

  if (PlayerGlider.TerminalDive != 0) return;
  if (Level == 0x12) {
    if (GetCurrentLevelObjectives() == 0) return;
  }

  if (PlayerGlider.TargetOn == 0) {
    Obj = 0x5a;
    if (Level == 0x24) Obj = 0x5e;
    if (PickGliderTarget(&Target, &Vel, &Moving) == 0) return;
    if (ObjTab[Obj].obj.special == NULL) return;

    NuVecSub(&Centre, Target, &GameCam[0].pos);
    NuVecScale(10.0f / NuVecMag(&Centre), &Centre, &Centre);
    NuVecAdd(&Centre, &Centre, (struct nuvec_s *)&GameCam[0].m._30);
    mTEMP = GameCam[0].m;
    NuMtxPreScale(&mTEMP, SetNuVecPntr(BIGSCALE, BIGSCALE, 0.0f));
    *(struct nuvec_s *)&mTEMP._30 = Centre;
    NuRndrGScnObj(
        (ObjTab[Obj].obj.scene)
            ->gobjs[(ObjTab[Obj].obj.special)->instance->objid],
        &mTEMP);
    return;
  }

  NuVecSub(&Centre, PlayerGlider.MovingTargetPoint, &GameCam[0].pos);
  NuVecScale(10.0f / NuVecMag(&Centre), &ScaledPos, &Centre);
  NuVecAdd(&ScaledPos, &ScaledPos, (struct nuvec_s *)&GameCam[0].m._30);

  NuVecMtxTransform(&Corner[0], SetNuVecPntr(-4.5f, 3.5f, 10.0f), &GameCam[0].m);
  NuVecMtxTransform(&Corner[1], SetNuVecPntr(4.5f, 3.5f, 10.0f), &GameCam[0].m);
  NuVecMtxTransform(&Corner[2], SetNuVecPntr(4.5f, -3.5f, 10.0f), &GameCam[0].m);
  NuVecMtxTransform(&Corner[3], SetNuVecPntr(-4.5f, -3.5f, 10.0f), &GameCam[0].m);

  i = 0;
  do {
    s32 next = (i + 1) & 3;
    float dot;
    struct nuvec_s *pProj = &ProjPos[i];
    NuVecSub(&Direction, &Corner[next], &Corner[i]);
    NuVecSub(&Delta, &ScaledPos, &Corner[i]);
    ProjPos[i] = Corner[i];
    dot = DotProduct(&Direction, &Delta);
    NuVecScaleAccum(dot / NuVecMagSqr(&Direction), pProj, &Direction);
    i++;
  } while (i <= 3);

  ratio = PlayerGlider.TargetTimer / Level_TargetTime;
  MiddleScale = ratio * (BIGSCALE - SMALLSCALE) + SMALLSCALE;
  RocketScale = ratio * (BIGSCALE - SMALLSCALE) * 4.0f + SMALLSCALE;

  if (ratio == 0.0f) {
    Obj = 0x5b;
    if (GliderFrameCounter & 2) Obj = 0x5a;
  } else {
    Obj = 0x5a;
  }
  if (Level == 0x24) Obj += 4;

  if (ObjTab[Obj].obj.special != NULL) {
    mTEMP = GameCam[0].m;
    NuMtxPreScale(&mTEMP, SetNuVecPntr(MiddleScale, MiddleScale, 0.0f));
    *(struct nuvec_s *)&mTEMP._30 = ScaledPos;
    NuRndrGScnObj(
        (ObjTab[Obj].obj.scene)
            ->gobjs[(ObjTab[Obj].obj.special)->instance->objid],
        &mTEMP);
  }

  Angle = 0;
  if (ratio == 0.0f) {
    Obj = 0x5d;
    if (GliderFrameCounter & 2) Obj = 0x5c;
  } else {
    Obj = 0x5c;
  }
  if (Level == 0x24) Obj += 4;

  if (ObjTab[Obj].obj.special == NULL) return;

  for (i = 0; i <= 3; i++) {
    s32 NextAngle = Angle - 0x4000;
    mTEMP = GameCam[0].m;
    NuMtxPreScale(&mTEMP, SetNuVecPntr(RocketScale, RocketScale, 0.0f));
    NuVecScale(ratio, (struct nuvec_s *)&mTEMP._30, &ProjPos[i]);
    NuVecScaleAccum(1.0f - ratio, (struct nuvec_s *)&mTEMP._30, &ScaledPos);
    NuMtxPreRotateZ(&mTEMP, Angle);
    NuRndrGScnObj(
        (ObjTab[Obj].obj.scene)
            ->gobjs[(ObjTab[Obj].obj.special)->instance->objid],
        &mTEMP);
    Angle = (s16)NextAngle;
  }
}

// NGC MATCH
void FireFlyReset(s32 PlayerDead) {
  Level_DeadTime = 8.0f;
  Level_GliderSpeed = 8.0f;
  Level_GliderFloor = 3.0f;
  Level_GliderCurrentCeiling = 25.0f;
  Level_GliderCeiling = 25.0f;
  Level_GliderRadius = 90.0f;
  Level_GliderCentreX = 40.32f;
  Level_GliderCentreZ = 52.09f;
  Level_TargetTime = 1.3f;
  GliderFrameCounter = 0;
  FireFlyIntroOn = 0;
  FireFlyIntroAction = 0;
  LevelResetTimer = 0;
  FlyingLevelVictoryDanceTimer = FIREFLYVICDANCETIME;
  FireFlyIntroOldAction = -1;
  InitGlider(&PlayerGlider, &FireFlyStart, FireFlyStartAngle);
  GameCam[0].m._30 = FireFlyStart.x + 0.0f;
  GameCam[0].m._31 = FireFlyStart.y + 2.0f;
  GameCam[0].m._32 = FireFlyStart.z + 5.0f;
  GliderCamHighTimer = 0.0f;
  ResetGameCameras(GameCam, 1);
  InitZoffaUFOs();
  InitGunBoats();
  InitBattleShips();
  InitGliderBullets();
  InitGliderBombs();
}


// NGC MATCH
void DrawFireFlyLevelExtra(void) {
  s32 i;

  for (i = 0; i < 4; i++) {
    if (GunBoatList[i].Active != 0) {
      DrawGunBoat(&GunBoatList[i]);
    }
  }
  for (i = 0; i < 6; i++) {
    if (BattleShipList[i].Active != 0) {
      DrawBattleShip(&BattleShipList[i]);
    }
  }
  DrawGliderBombs();
  DrawGliderBullets();
  DrawZoffaUFOs();
  DrawTorpedoTarget();
}

// NGC MATCH
void ProcessCrashteroidsIntro(void) { ProcessFireFlyIntro(); }

// NGC MATCH
void ProcessFireFlyIntro(void) {
  s32 Done;
  float RelAng;
  float ThisPanSeekSpeed;
  float Temp;
  struct nuvec_s Dir;
  struct nuvec_s Dir2;
  struct nuvec_s Rel;

  Dir2 = SetNuVec(1.0f, 0.0f, 1.0f);
  NuVecNorm(&Dir, &Dir2);
  if (Level == 0x12) {
    MatchTimer = 0.0f;
    MatchMaxDist = 4.0f;
    MatchMinDist = 4.0f;
    PrePanTime = 2.0f;
    PanSeekSpeed = 2.0f;
    MinPanSeekSpeed = 0.3f;
    FFINTROHEIGHT = 2.5f;
    FFINTROCAMYOFF = 4.0f;
  } else if (Level == 0x1a) {
    MatchMaxDist = 3.0f;
    MatchMinDist = 3.0f;
    MatchTimer = 0.0f;
    PrePanTime = 3.5f;
    PanSeekSpeed = 1.0f;
    MinPanSeekSpeed = 0.3f;
    FFINTROHEIGHT = -2.5f;
    FFINTROCAMYOFF = 1.75f;
  }
  if (FireFlyIntroOn != 0) {
    NuVecScale(Level_GliderSpeed, &PlayerGlider.Velocity, &Dir2);
    NuVecScaleAccum(0.01666667, &PlayerGlider.Position, &PlayerGlider.Velocity);
  }
  GliderIntroInterest = PlayerGlider.Position;
  if (FireFlyIntroAction != FireFlyIntroOldAction) {
    switch (FireFlyIntroAction) {
    case 0:
      PlayerGlider.AutoPilot = 1;
      FireFlyIntroOn = 1;
      GliderIntroCamPos =
          SetNuVec(40.0f, (FFINTROCAMYOFF + 18.0f) - FFINTROHEIGHT, 40.0f);
      *(struct nuvec_s *)&GameCam[0].m._30 = GliderIntroCamPos;
      break;
    case 1:
      Timer_549 = MatchTimer;
      break;
    case 2:
      Timer_549 = PrePanTime;
      break;
    case 3:
      CamAngY_550 = (float)(GameCam[0].yrot) / 182.04445f + 180.0f;
      break;
    case 4:
      PlayerGlider.AutoPilot = 0;
      FireFlyIntroOn = 0;
      break;
    }
  }
  FireFlyIntroOldAction = FireFlyIntroAction;
  switch (FireFlyIntroAction) {
  case 0:
    NuVecSub(&Rel, &PlayerGlider.Position, &GliderIntroCamPos);
    Rel.y = 0.0f;
    Temp = NuVecMag(&Rel);
    CamAngY_550 = PlayerGlider.AngleY + 180.0f;
    if (Temp < MatchMaxDist) {
      FireFlyIntroAction = 1;
    }
    break;
  case 1:
    GliderIntroCamPos.x = PlayerGlider.Position.x;
    GliderIntroCamPos.z = PlayerGlider.Position.z;
    NuVecScaleAccum((Timer_549 / MatchTimer) * (MatchMaxDist - MatchMinDist) +
                        MatchMinDist, &GliderIntroCamPos, &Dir);
    if (ProcessTimer(&Timer_549) != 0) {
      FireFlyIntroAction = 3;
    }
    break;
  case 2:
    GliderIntroCamPos = PlayerGlider.Position;
    NuVecScaleAccum(MatchMinDist, &GliderIntroCamPos, &Dir);
    if (ProcessTimer(&Timer_549) != 0) {
      FireFlyIntroAction = 3;
    }
    break;
  case 3:
    Done = 0;
    RelAng = Rationalise360f(PlayerGlider.AngleY - CamAngY_550);
    Temp = NuFabs(RelAng);
    if (Temp > 90.0f) {
      Temp = (180.0f - Temp);
    }
    ThisPanSeekSpeed = (PanSeekSpeed * Temp) / 90.0f;
    if (ThisPanSeekSpeed < MinPanSeekSpeed) {
      ThisPanSeekSpeed = MinPanSeekSpeed;
    }
    Temp = NuFabs(RelAng);
    if (Temp < ThisPanSeekSpeed) {
      Done = 1;
      RelAng = 0.0f;
    } else {
      RelAng = (RelAng - ThisPanSeekSpeed);
    }
    CamAngY_550 = Rationalise360f(PlayerGlider.AngleY - RelAng);
    if (Done != 0) {
      FireFlyIntroAction = 4;
    }
    break;
  case 4:
    FireFlyIntroAction = 5;
    break;
  }
  GliderIntroInterest.y += 1.0f;
  if (FireFlyIntroAction != 0) {
    GliderIntroCamPos = PlayerGlider.Position;
    NuVecScale(MatchMinDist, &Rel, &Dir);
    NuVecRotateY(
        &Rel, &Rel,
        (s32)(((CamAngY_550 - FireFlyStartAngle) - 180.0f) * 182.04445f));
    NuVecAdd(&GliderIntroCamPos, &Rel, &PlayerGlider.Position);
    RelAng = NuFabs(Rationalise360f(PlayerGlider.AngleY - CamAngY_550));
    GliderIntroCamPos.y += FFINTROCAMYOFF - (RelAng * FFINTROHEIGHT) / 180.0f;
  }
}

// NGC MATCH
void ProcessFireFlyLevel(struct nupad_s *Pad) {
  s32 i;

  TeleportManager();
  LastNumZoffasFiring = NumZoffasFiring;
  NumZoffasFiring = 0;
  for (i = 0; i < 4; i++) {
    if (EnemyZoffa[i].ActiveMode == 0) {
      if (PlayerGlider.AutoPilot == 0) {
        EnemyZoffa[i].RespawnTimer -= 0.016666668f;
        if (EnemyZoffa[i].RespawnTimer < 0.0f) {
          EnemyZoffa[i].RespawnTimer = 0;
        }
      }
    } else {
      MoveZoffaUFO(&EnemyZoffa[i]);
    }
  }
  ZoffaUFORespawn();
  CheckGliderCollisions();
  CheckForPotentialMidAirCollisions();
  ProcessGliderBombs();
  ProcessBattleShips();
  ProcessGunBoats();
  ProcessGliderBullets();
  GliderFrameCounter++;
}

// NGC MATCH
s32 GetCurrentFireFlyObjectives(void) {
  s32 i;
  s32 j;

  for (i = 0, j = 0; i < 6; i++) {
    if ((BattleShipList[i].Active != 0) && (0 < BattleShipList[i].HitPoints)) {
      j++;
    }
  }
  return j;
}

// NGC MATCH
void WeatherResearchReset(void) {
  struct nuvec_s Start;

  Start = WRStartPos;
  LevelResetTimer = 0;
  Level_GliderSpeed = 16.0f;
  Level_GliderCeiling = 60.0f;
  Level_GliderFloor = -60.0f;
  Level_GliderCurrentCeiling = 60.0f;
  Level_GliderRadius = 100.0f;
  Level_GliderCentreX = 0.32f;
  Level_GliderCentreZ = 0.09f;
  Level_DeadTime = 8.0f;
  Level_TargetTime = 0.7f;
  InitGlider(&PlayerGlider, &Start, WRStartAng);
  GameCam[0].m._30 = Start.x + 0.0f;
  GameCam[0].m._31 = Start.y + 2.0f;
  GameCam[0].m._32 = Start.z + 5.0f;
  GliderCamHighTimer = 0.0f;
  ResetGameCameras(GameCam, 1);
  InitGliderBullets();
  InitBigGuns();
  InitGliderBombs();
}

// NGC MATCH
void DrawWeatherResearchLevelExtra(void) {
  DrawGliderBullets();
  DrawBigGuns();
  DrawGliderBombs();
  DrawTorpedoTarget();
}

// NGC MATCH
void ProcessWeatherResearchLevel(struct nupad_s *Pad) {
  ProcessGliderBullets();
  ProcessGliderBombs();
  ProcessBigGuns();
}

// NGC MATCH
s32 GetCurrentWeatherResearchObjectives(void) {
  s32 i;
  s32 j;

  for (i = 0, j = 0; i < 0xc; i++) {
    if ((BigGunList[i].Active != 0) && (BigGunList[i].Action != 2)) {
      j++;
    }
  }
  return j;
}

// NGC MATCH
void SpaceArenaReset(s32 PlayerDead) {
  Level_DeadTime = 8.0f;
  Level_GliderFloor = -30.0f;
  Level_GliderCurrentCeiling = 35.0f;
  Level_GliderRadius = 180.0f;
  Level_GliderCentreX = 0.0f;
  Level_GliderCentreZ = 0.0f;
  FireFlyIntroOn = 0;
  LevelResetTimer = 0;
  FireFlyIntroAction = 0;
  FireFlyIntroOldAction = -1;
  Level_GliderSpeed = 8.0f;
  Level_GliderCeiling = 35.0f;
  InitGlider(&PlayerGlider, &CrashteroidsStart, CrashteroidsStartAngle);
  GameCam[0].m._30 = CrashteroidsStart.x + 0.0f;
  GameCam[0].m._31 = CrashteroidsStart.y + 2.0f;
  GameCam[0].m._32 = CrashteroidsStart.z + 5.0f;
  GliderCamHighTimer = 0.0f;
  ResetGameCameras(GameCam, 1);
  InitZoffaUFOs();
  InitSpaceStations();
  InitAsteroids();
  InitGliderBullets();
}


// NGC MATCH
s32 GetCurrentSpaceArenaObjectives(void) {
  s32 j;
  s32 i;

  for (i = 0, j = 0; i < 3; i++) {
    if (SpaceStationList[i].Active != 0) {
      j++;
    }
  }
  return j;
}

// NGC MATCH
s32 CollidePlayerPoint(struct nuvec_s *Pos, float Rad2, s32 Hits) {
  s32 OldHits;
  float TimerInc;

  OldHits = PlayerGlider.HitPoints;
  if (NuVecDistSqr(Pos, &PlayerGlider.Position, NULL) <= Rad2) {
    TimerInc = (float)Hits * 0.125f;
    if ((InvincibilityCHEAT == 0) && (VehicleLevelImmune == 0)) {
      PlayerGlider.HitPoints = PlayerGlider.HitPoints - Hits;
    }
    PlayerGlider.HitTimer += TimerInc;
    if (Hits != 0) {
      NewBuzz(&player->rumble, (BEENHITBIGBUZZTIME * 0x19) / Hits);
      NewRumble(&player->rumble, (BEENHITBIGRUMTIME * 0x19) / Hits);
    }
    if (4.5f <= PlayerGlider.HitTimer) {
      PlayerGlider.HitTimer = 4.5f;
    }
    if ((PlayerGlider.NextHitSoundTimer == 0.0f) ||
        (((PlayerGlider.HitPoints < 1 && (OldHits > 0)) || (4 < Hits)))) {
      MyGameSfx(0x5a, &PlayerGlider.Position, 0x7fff);
      PlayerGlider.NextHitSoundTimer = GLIDERHITSOUNDFREQUENCY;
    } else {
      MyGameSfx(GLIDERHITSOUNDOTHERID, &PlayerGlider.Position, 0x4fff);
    }
    if (PlayerGlider.HitPoints < 1) {
      PlayerGlider.HitPoints = 0;
      PlayerGlider.TerminalDive = 1;
      PlayerGlider.HitTimer = Level_DeadTime;
    }
    return 1;
  }
  return 0;
}

// NGC MATCH
void InitLighteningHail(void) {
  memset(&HailList, 0, 0x438);
  NuMtxSetScale(&BoltMtxC, SetNuVecPntr(0.5f, 1.5f, 0.5f));
}

// NGC MATCH
void DrawLighteningHail(void) {
  s32 i;
  LIGHTENINGHAIL *Hail;

  if (FlyingLevelVictoryDance != 0) {
    return;
  }
  Hail = HailList;
  for (i = 0; i < 0x1e; i++, Hail++) {
    if (Hail->Mode != 0) {
      if (Hail->Mode == 1) {
        if (Paused == 0) {
          AddVariableShotDebrisEffect(GDeb[0x93].i, &Hail->Position, 1, 0, 0);
        }
      } else if (Hail->Mode == 3) {
        if (Paused == 0) {
          AddVariableShotDebrisEffect(GDeb[0x93].i, &Hail->Position, 1, 0, 0);
        }
      }
    }
  }
}

// NGC MATCH
void ProcessLighteningHail(void) {
  s32 i;
  LIGHTENINGHAIL *Hail;

  if (FlyingLevelVictoryDance != 0) {
    return;
  }
  Hail = HailList;
  for (i = 0; i < 0x1e; i++, Hail++) {
    if (Hail->Mode != 0) {
      if (ProcessTimer(&Hail->Timer) != 0) {
        Hail->Timer = HAILHIDDENTIME;
        Hail->Mode++;
        if (Hail->Mode == 3) {
          Hail->Position.x = Hail->FallX;
          Hail->Position.y = 46.0f;
          Hail->Position.z = PlayerGlider.Position.z;
          Hail->Timer = 1.5f;
          Hail->Velocity.x = 0.0f;
          Hail->Velocity.y = (-HailFallSpeed * 0.016666668f);
          Hail->Velocity.z = (-Level_GliderSpeed * 0.016666668f);
        } else if (Hail->Mode == 4) {
          Hail->Mode = 0;
        }
      } else if (Hail->Mode == 3) {
        if (CollidePlayerPoint(&Hail->Position, HAILCOLLRAD, 5) != 0) {
          Hail->Mode = 0;
        }
      }
      NuVecAdd(&Hail->Position, &Hail->Position, &Hail->Velocity);
    }
  }
}

// NGC MATCH
void UnleashLighteningHail(struct nuvec_s *Pos, s32 AttNum) {
    struct nuvec_s vel;
    float upSpeed;
    float Timer;
    float FallX;
    float timeIncrement;
    float xIncrement;
    float fallXIncrement;
    float count;
    s32 SkipLeft;
    s32 i;

    upSpeed = HAILFLYUPSPEED * (1.0f / 60.0f);
    memset(&HailList, 0, 0x438);
    NuVecScale(1.0f / 60.0f, &vel, &LighteningHailVel);

    Timer = 2.0f;
    FallX = LighteningFallXMax;

    switch (AttNum) {
    case 0:
        SkipLeft = SkipLeftA;
        break;
    case 1:
        SkipLeft = SkipLeftB;
        break;
    default:
        SkipLeft = SkipLeftC;
        break;
    }

    count = (float)(SkipNum + 29);
    LighteningHailVel.z = -Level_GliderSpeed;

    timeIncrement = HAILTIMERANGE / count;
    xIncrement = (-vel.x - vel.x) / count;
    fallXIncrement = (LighteningFallXMin - LighteningFallXMax) / count;

    for (i = 0; i <= 29; i++) {
        HailList[i].Mode = 1;
        HailList[i].Timer = Timer;
        HailList[i].Position = *Pos;
        HailList[i].Velocity = vel;
        NuVecNorm(&HailList[i].Velocity, &HailList[i].Velocity);
        NuVecScale(upSpeed, &HailList[i].Velocity, &HailList[i].Velocity);
        HailList[i].FallX = FallX;

        if (i == SkipLeft) {
            float skip = (float)SkipNum;
            vel.x += xIncrement * skip;
            Timer += timeIncrement * skip;
            FallX += fallXIncrement * skip;
        } else {
            vel.x += xIncrement;
            Timer += timeIncrement;
            FallX += fallXIncrement;
        }
    }
}


// NGC MATCH
void InitWBBolts(void) {
  memset(BoltList, 0, 0x1680);
  NuMtxSetScale(&BoltMtxA, SetNuVecPntr(1.5f, 1.5f, 1.5f));
  NuMtxSetScale(&BoltMtxB, SetNuVecPntr(8.0f, 8.0f, 8.0f));
}

// NGC MATCH
void DrawWBBolts(void) {
    s32 i;
    s32 debType;
    WBBOLT *Bolt;
    struct nugscn_s **scn;
    void *objPtr;
    s32 idx;

    Bolt = BoltList;
    for (i = 0; i < 0x78; i++) {
        if (Bolt->Mode != 0) {
            if (Bolt->Type == 2) {
                if (Paused == 0) {
                    AddVariableShotDebrisEffect(GDeb[0x91].i, &Bolt->Position,
                                                1, 0, 0);
                }
                {
                    float s = Bolt->Scale * 8.0f;
                    NuMtxSetScale(&BoltMtxB, SetNuVecPntr(s, s, s));
                }
                scn = ObjTab[0xBD].scene;
                if (scn != NULL) {
                    *(struct nuvec_s *)&BoltMtxB._30 = Bolt->Position;
                    objPtr = *(void **)&ObjTab[0xBD].obj;
                    idx = *(s32 *)((u8 *)*(void **)((u8 *)scn + 0x40) + 0x40);
                    NuRndrGScnObj(
                        ((void **)(*(void **)((u8 *)objPtr + 0x14)))[idx],
                        &BoltMtxB);
                }
            } else {
                if (Paused == 0) {
                    debType = 0x92;
                    if (Bolt->Owner == 2) {
                        debType = 0x94;
                    }
                    AddVariableShotDebrisEffect(GDeb[debType].i,
                                                &Bolt->Position, 1, 0, 0);
                }
                if (Bolt->Owner == 2) {
                    scn = ObjTab[0xBD].scene;
                    if (scn != NULL) {
                        *(struct nuvec_s *)&BoltMtxA._30 = Bolt->Position;
                        objPtr = *(void **)&ObjTab[0xBD].obj;
                        idx = *(s32 *)((u8 *)*(void **)((u8 *)scn + 0x40) +
                                       0x40);
                        NuRndrGScnObj(
                            ((void **)(*(void **)((u8 *)objPtr + 0x14)))[idx],
                            &BoltMtxA);
                    }
                }
            }
        }
        Bolt++;
    }
}


// NGC MATCH
struct WBBOLT *FindFreeWBBoltOfType(s32 Type) {
  s32 i;
  struct WBBOLT *Bolt;

  Bolt = BoltList;
  for (i = 0; i < 0x78; i++, Bolt++) {
    if ((Bolt->Mode != 0 && (Bolt->Type == Type))) {
      return Bolt;
    }
  }
  return NULL;
}

// NGC MATCH
struct WBBOLT *FindFreeWBBolt(void) {
  s32 i;
  struct WBBOLT *Bolt;

  Bolt = BoltList;
  for (i = 0; i < 0x78; i++, Bolt++) {
    if (Bolt->Mode == 0) {
      return Bolt;
    }
  }
  return NULL;
}

// NGC MATCH
struct nuvec_s *FireWBBolt(struct nuvec_s *Pos, struct nuvec_s *Vel, int Type,
                           float Life, int Owner) {
  WBBOLT *Bolt;
  s32 Given;
  float OldScale;

  Bolt = NULL;
  Given = 0;
  if (Type == 2) {
    Bolt = FindFreeWBBoltOfType(2);
  }
  if (Bolt != NULL) {
    Given = 1;
  } else {
    Bolt = FindFreeWBBolt();
  }
  if (Bolt != NULL) {
    if (Given != NULL) {
      OldScale = Bolt->Scale;
    } else if (Type == 2) {
      OldScale = 0.0f;
    } else {
      OldScale = 1.0f;
    }
    memset(Bolt, 0, 0x30);
    Bolt->Owner = Owner;
    Bolt->Position = *Pos;
    NuVecScale(0.01666667f, &Bolt->Velocity, Vel);
    Bolt->Life = Life;
    Bolt->Type = Type;
    Bolt->Mode = 2;
    Bolt->Scale = OldScale + (0.01666667f / WBTYPE2SCALETIME);
    if (Bolt->Scale >= 1.0f) {
      Bolt->Scale = 1.0f;
    }
    if (Bolt->Type == 0) {
      Bolt->SeekSpeed = BOLTHOMESEEKSPEED * 0.01666667f;
    } else {
      Bolt->SeekSpeed = 0.0f;
    }
    return &Bolt->Position;
  }
  return NULL;
}

// NGC MATCH
void ProcessWBBolts(void) {
  s32 i;
  WBBOLT *Bolt;
  struct nuvec_s Temp;
  float Scale;

  Bolt = BoltList;
  for (i = 0; i < 0x78; i++, Bolt++) {
    if (Bolt->Mode != 0) {
      if (ProcessTimer(&Bolt->Life) != 0) {
        Bolt->Mode = 0;
      }
      if (Bolt->SeekSpeed != 0.0f) {
        NuVecSub(&Temp, &PlayerGlider.Position, &Bolt->Position);
        Temp.z = 0.0f;
        Scale = NuVecMag(&Temp);
        if (Scale <= Bolt->SeekSpeed) {
          Bolt->Position.x = PlayerGlider.Position.x;
          Bolt->Position.y = PlayerGlider.Position.y;
          Bolt->Velocity.x = 0.0f;
          Bolt->Velocity.y = 0.0f;
        } else {
          NuVecScale(Bolt->SeekSpeed / Scale, &Temp, &Temp);
          Temp.z = Bolt->Velocity.z;
          SeekHalfLifeNUVEC(&Bolt->Velocity, &Temp, BOLTHOMESEEKTIME,
                            0.01666667f);
        }
      }
      NuVecAdd(&Bolt->Position, &Bolt->Position, &Bolt->Velocity);
      switch (Bolt->Type) {
      case 2:
        if (CollidePlayerPoint(&Bolt->Position, 9.0f, 0x19) != 0) {
          Bolt->Mode = 0;
        }
        break;
      default:
        if (Bolt->Type < 2) {
          if (CollidePlayerPoint(&Bolt->Position, 2.25f, 5) != 0) {
            Bolt->Mode = 0;
          }
        } else {
          if (CollidePlayerPoint(&Bolt->Position, 2.25f, 1) != 0) {
            Bolt->Mode = 0;
          }
        }
        break;
      }
    }
  }
}

// NGC MATCH
void SetWeatherStartPos(struct creature_s *Cre) {
  PlayerGlider.Position = WeatherStartPos = Cre->obj.pos;
}

// NGC MATCH
void WeatherBossReset(void) {
  struct nuvec_s Start = {0.0f, 38.0f, 0.0f};
  struct nuvec_s Temp;
  short Temps;
  struct MYSPLINE *Spline;

  ChrisBigBossDead = 0;
  WeatherBossDead = 0;
  LevelResetTimer = 0;
  WBIntroTweenTimer = 0.0f;
  Level_GliderFloor = 18.0f;
  Level_GliderCeiling = 60.0f;
  Level_GliderCurrentCeiling = 60.0f;
  Level_GliderRadius = 100000.0f;
  Level_DeadTime = 8.0f;
  WeatherBossSkeletonGlitchTimer = 0.0f;
  WeatherBossSkeletonTimer = 0.0f;
  AtmosphericPressureHackedZ = 0.0f;
  Level_GliderSpeed = 0.0f;
  Level_GliderCentreX = 0.0f;
  Level_GliderCentreZ = 0.0f;
  InitGlider(&PlayerGlider, &Start, 0.0f);
  GameCam[0].m._30 = Start.x + 0.0f;
  GameCam[0].m._31 = Start.y + 2.0f;
  GameCam[0].m._32 = Start.z + 5.0f;
  GliderCamHighTimer = 0.0f;
  ResetGameCameras(GameCam, 1);
  WeatherBossCamSpline.Spline = SplTab[0x1a].spl;
  if (SplTab[0x1a].spl != NULL) {
    WeatherBossCamSpline.Inc = 0.000019999999f;
    WeatherBossCamSpline.Cur = 0.0f;
    WeatherBossCamSpline.Nex = 0.0f;
    PointAlongSpline(SplTab[0x1a].spl, 0.0f, &Temp, &Temps, NULL);
    WeatherBossCamSpline.CurPos = Temp;
    WeatherBossCamSpline.NexPos = Temp;
    InitVehMasks();
    InitVehMask(0, 0x58);
    InitVehMask(1, 3);
    InitWeatherBossTarget();
    InitGliderBullets();
    InitWeatherBoss();
    InitLighteningHail();
    InitWBBolts();
    InitWBIntro();
  }
}

// NGC MATCH
void InitWBIntro(void) {
  short Temps;

  WBIntroOn = 0;
  WeatherBossIntroSpline.Spline = SplTab[0x48].spl;
  if (SplTab[0x48].spl != NULL) {
    WeatherBossIntroSpline.Cur = 0.0f;
    WeatherBossIntroSpline.Inc = 0.000019999999f;
    WeatherBossIntroSpline.Nex = 0.0f;
    PointAlongSpline(WeatherBossIntroSpline.Spline, 0.0f, &WBIntroGliderPos,
                     &Temps, NULL);
    WeatherBossIntroSpline.NexPos = WeatherBossIntroSpline.CurPos =
        WBIntroGliderPos;
    WeatherBossIntroSpline.LookaheadDist = 0.0f;
    WeatherBoss.Distance = WBBOSSINTRODIST;
    WeatherBossTargetAppearTimer = WeatherBossTargetAppearTime;
    WBIntroTweenTimer = WBIntroTweenTime;
    WeatherBoss.DistanceDest = WBBOSSINTRODIST;
    WBIntroOn = 1;
  }
}

// NGC MATCH
void DrawWeatherBossLevelExtra(void) {
  DrawGliderBullets();
  DrawWeatherBoss();
  DrawWeatherBossTarget();
  DrawLighteningHail();
  DrawWBBolts();
  DrawVehMasks();
  DrawBazookaToken();
}

// NGC MATCH
void ProcessWeatherBossLevel(struct nupad_s *Pad) {
  ProcessWBIntro();
  if (WBIntroOn == 0) {
    AtmosphericPressureHackedZ -= 1.6f;
    if (AtmosphericPressureHackedZ < -1200.0f) {
      AtmosphericPressureHackedZ += 1200.0f;
    }
  } else {
    AtmosphericPressureHackedZ = WBIntroGliderPos.z;
    if (WBIntroGliderPos.z < -1200.0f) {
      AtmosphericPressureHackedZ += 1200.0f;
    }
    if (AtmosphericPressureHackedZ > 0.0f) {
      AtmosphericPressureHackedZ -= 1200.0f;
    }
  }
  ProcessGliderBullets();
  ProcessWeatherBoss();
  ProcessLighteningHail();
  ProcessWBBolts();
  ProcessVehMasks();
  ProcessBazookaToken();
}

// NGC MATCH
void ProcessWBIntro(void) {
  struct nuvec_s IntroPos;
  struct nuvec_s Target;
  struct nuvec_s Dest;

  if (WBIntroOn != 0) {
    IntroPos = WBIntroGliderPos;
    IntroPos.z -= WBINTROLOOKAHEAD;
    FindSplineClosestPointAndDist(&WeatherBossIntroSpline, 0x300, &IntroPos,
                                  &Target, 0, 0);
    NuVecSub(&Dest, &Target, &WBIntroGliderPos);
    NuVecScale((96.0f / NuFabs(Dest.z)), &Dest, &Dest);
    NuVecScaleAccum(0.01666667f, &WBIntroGliderPos, &Dest);
    WBIntroDestTiltZ = Dest.x * WBITILTZSCALE;
    WBIntroDestTiltX = Dest.y * WBITILTXSCALE;
    PlayerGlider.Velocity = Dest;
    if (WeatherBossIntroSpline.Nex >= 0.99f) {
      WBIntroOn = 0;
      WeatherBoss.DistanceDest = 96.0f;
    }
  }
}

// NGC MATCH
s32 GetCurrentWeatherBossObjectives(void) {
  s32 j;
  s32 i;

  if (WeatherBoss.Active != 0) {
    for (i = 0, j = 0; i < 4; i++) {
      if (WeatherBoss.HitPoints[i] > 0) {
        j += WeatherBoss.HitPoints[i];
      }
    }
    i = j + 3;
    if (i < 0) {
      i = j + 6;
    }
    return i >> 2;
  }
  return 0;
}

// NGC MATCH
void ProcessVehicleLevel(struct nupad_s *Pad) {
  switch (Level) {
  case 13:
    ProcessFarmLevel(Pad);
    break;
  case 18:
    ProcessFireFlyLevel(Pad);
    break;
  case 36:
    ProcessWeatherResearchLevel(Pad);
    break;
  case 24:
    ProcessWeatherBossLevel(Pad);
    break;
  case 26:
    ProcessSpaceArenaLevel(Pad);
    break;
  case 3:
    ProcessWesternArenaLevel(Pad);
    break;
  case 21:
    ProcessEarthBossLevel(Pad);
    break;
  case 22:
    ProcessFireBossLevel(Pad);
    break;
  }
  switch (Level) {
  case 0x18:
  case 0xD:
  case 0x12:
  case 0x1A:
  case 0x24:
    if (GetCurrentLevelObjectives() == 0) {
      VehicleLevelImmune = 1;
      FlyingLevelExtro = 1;
      ProcessTimer(&FlyingLevelCompleteTimer);
    }
    break;
  }
  return;
}

// NGC MATCH
void EarthBossReset(void) {
  EarthBossDeathTimer = 0;
  EarthBossJustEntered = 0;
  EarthBossDeathEffect = 0;
  EarthBossVortexOpen = 0;
  ChrisBigBossDead = 0;
  RumbleDisplayMode = 0;
  InitVehMasks();
  InitVehMask(0, 0x55);
  InitVehMask(1, 3);
  InitEarthBoss();
  InitJeepRocks();
  InitRumblePanel();
}

// NGC MATCH
void ResetVehicleLevel(s32 PlayerDead) {
  FlyingLevelCompleteTimer = 1.5f;
  FireFlyIntroOldAction = -1;
  VehicleLevelImmune = 0;
  FlyingLevelExtro = 0;
  FlyingLevelVictoryDance = 0;
  FlyingLevelVictoryDanceTimer = 0;
  FireFlyIntroTween = 0.0f;
  FireFlyIntroAction = 0;
  FireFlyIntroOn = 0;
  GliderIntroInterest = v000;
  GliderIntroCamPos = v000;
  switch (Level) {
  case 13:
    FarmReset();
    break;
  case 18:
    FireFlyReset(PlayerDead);
    break;
  case 36:
    WeatherResearchReset();
    break;
  case 26:
    SpaceArenaReset(PlayerDead);
    break;
  case 24:
    WeatherBossReset();
    break;
  case 3:
    WesternArenaReset(PlayerDead);
    break;
  case 22:
    FireBossReset(PlayerDead);
    break;
  case 21:
    EarthBossReset();
    break;
  }
  if (Level == 0x12) {
    ProcessFireFlyIntro();
  }
  if (Level == 0x1a) {
    ProcessCrashteroidsIntro();
  }
}

// NGC MATCH
s32 GetMaxLevelObjectives(void) {
  switch (Level) {
  case 18:
  case 13:
    return 6;
  case 24:
    return 0x64;
  case 36:
    return 0xC;
  case 22:
    return GetTotalFireBossObjectives();
  case 26:
  case 21:
    return 3;
  case 25:
    return GetTotalSpaceBossObjectives();
  default:
    return 1;
  }
}

// NGC MATCH
s32 GetCurrentLevelObjectives(void) {
  switch (Level) {
  case 13:
    return GetCurrentFarmObjectives();
  case 18:
    return GetCurrentFireFlyObjectives();
  case 26:
    return GetCurrentSpaceArenaObjectives();
  case 24:
    return GetCurrentWeatherBossObjectives();
  case 36:
    return GetCurrentWeatherResearchObjectives();
  case 22:
    return GetCurrentFireBossObjectives();
  case 21:
    return GetCurrentRumbleObjectives();
  case 25:
    return GetCurrentSpaceBossObjectives();
  default:
    return 0;
  }
}

s32 TryUnembeddPointDir(struct nuvec_s *pos, struct nuvec_s *dir1,
                       struct nuvec_s *dir2, s16 *handle, float radius);
s32 TryUnembeddPointSafe(struct nuvec_s *pos, struct nuvec_s *target,
                         s16 *handle, float radius);

// NGC MATCH
void UnembedRayCastAtlas(struct ATLASSTRUCT *Atlas, short *TerrHandle) {
  struct nuvec_s Up = {0.0f, 1.0f, 0.0f};

  if (TryUnembeddPointDir(&Atlas->Position, &ShadNorm, &Up, TerrHandle,
                          Atlas->Radius) == 0) {
    TryUnembeddPointSafe(&Atlas->Position, &Atlas->OldPosition, TerrHandle,
                         Atlas->Radius);
  }
}

s32 TryUnembeddPointDirSimple(struct nuvec_s *pos, struct nuvec_s *dir,
                             s16 *handle, s32 iterations, float radius,
                             float step);

// NGC MATCH
s32 UnembedRayCastAtlasSimple(struct ATLASSTRUCT *Atlas, short *TerrHandle) {
  struct nuvec_s Up = {0.0f, 1.0f, 0.0f};

  return TryUnembeddPointDirSimple(&Atlas->Position, &Up, TerrHandle, 5,
                                   Atlas->Radius, 0.03f);
}

// NGC MATCH
void AdjustAtlasRotations(struct ATLASSTRUCT *Atlas, struct nuvec_s *Velocity,
                          struct nuvec_s *Normal, float dt) {
    struct nuvec_s vel;
    struct numtx_s mtx;
    struct numtx_s mtxInv;
    struct nuquat_s quat;
    float yaw;
    float pitch;
    float rollScale;
    s32 yawInt;
    s32 pitchInt;

    vel = *Velocity;

    if (NuFabs(Normal->x) < 0.01f && NuFabs(Normal->z) < 0.01f) {
        yaw = 0.0f;
    } else {
        yaw = (float)(-(s32)(s32)(u16)NuAtan2D(Normal->x, Normal->z)) /
              182.04445f;
    }

    {
        float xzMag;

        xzMag = NuFsqrt(Normal->z * Normal->z + Normal->x * Normal->x);
        pitch = (float)(-(s32)(s32)(u16)NuAtan2D(xzMag, Normal->y)) /
                182.04445f;
    }

    NuVecScaleAccum(-DotProduct(&vel, Normal), &vel, Normal);

    yawInt = (s32)(yaw * 182.04445f);
    NuVecRotateY(&vel, &vel, yawInt);

    pitchInt = (s32)(pitch * 182.04445f);
    NuVecRotateX(&vel, &vel, pitchInt);

    rollScale = NuVecMag(&vel) * 180.0f;
    rollScale /= dt * 3.1415927f;

    {
        float roll;
        roll = (float)(-(s32)(s32)(u16)NuAtan2D(vel.x, vel.z)) / 182.04445f;

        NuMtxSetRotationY(&mtx, yawInt);
        NuMtxRotateX(&mtx, pitchInt);

        NuMtxRotateY(&mtx, (s32)(-roll * 182.04445f));
        NuMtxInv(&mtxInv, &mtx);

        NuMtxRotateX(&mtx, (s32)(rollScale * 182.04445f));
        NuMtxMul(&mtx, &mtx, &mtxInv);
        NuMtxToQuat(&mtx, &quat);
        NuQuatMul(&Atlas->Quat, &quat, &Atlas->Quat);
    }
}

// NGC MATCH
void TerrainAtlas(struct ATLASSTRUCT *Atlas) {
  struct nuvec_s delta;
  struct nuvec_s Normal;
  struct nuvec_s RayStart;
  struct nuvec_s RayEnd;
  float dist;
  float Height;
  s32 PlatId;

  RayStart = Atlas->Position;
  RayStart.y += Atlas->Radius + 1.0f;
  RayEnd = Atlas->Position;
  RayEnd.y -= Atlas->Radius + 1.0f;

  PlatId = NewRayCast(&RayStart, &RayEnd, 0.0f);
  if (PlatId != -1) {
    if (Atlas->Position.y < Height + Atlas->Radius) {
      Atlas->Position.y = Height + Atlas->Radius;
      if (Atlas->Velocity.y < 0.0f) {
        Atlas->Velocity.y = -Atlas->Velocity.y * 0.3f;
      }

      // Friction
      SeekHalfLife(&Atlas->Velocity.x, 0.0f, 0.5f, 0.01666667f);
      SeekHalfLife(&Atlas->Velocity.z, 0.0f, 0.5f, 0.01666667f);
    }
  }

  // Gravity
  Atlas->Velocity.y -= 0.3f;

  // Apply velocity
  Atlas->Position.x += Atlas->Velocity.x * 0.01666667f;
  Atlas->Position.y += Atlas->Velocity.y * 0.01666667f;
  Atlas->Position.z += Atlas->Velocity.z * 0.01666667f;

  // Rolling angle
  if (Atlas->Velocity.x != 0.0f || Atlas->Velocity.z != 0.0f) {
    float speed;
    speed = NuFsqrt(Atlas->Velocity.x * Atlas->Velocity.x +
                    Atlas->Velocity.z * Atlas->Velocity.z);
    Atlas->AngleY = NuAtan2D(Atlas->Velocity.x, Atlas->Velocity.z) / 182.04445f;
    Atlas->AngleX += speed * 0.01666667f / Atlas->Radius;
  }

  // Collision with walls
  if (Atlas->Position.y < Level_GliderFloor) {
    Atlas->Position.y = Level_GliderFloor;
    Atlas->Velocity.y = 0.0f;
  }

  // Shadow
  NewShadow(&Atlas->Position, Atlas->Radius * 2.0f);
}


// NGC MATCH
void CheckAtlasGround(struct ATLASSTRUCT *Atlas) {
  struct nuvec_s Rel;
  struct nuvec_s Normal = {0.0f, 1.0f, 0.0f};
  struct nuvec_s Pos;
  float Volume;
  s32 VolumeI;
  float Temp;

  Pos = Atlas->Position;
  NuVecScaleAccum(0.01f, &Pos, &Normal);
  NuVecScale(-0.02f, &Rel, &Normal);
  NewRayCastSetHandel(&Pos, &Rel, Atlas->Radius, 0.0f, 0.0f, Atlas->TerrHandle);
  Atlas->PlatformId = TerrainPlatId();
  Atlas->PlatformNormal = ShadNorm;
  Atlas->ShadowY = NewShadowMask(&Atlas->Position, Atlas->Radius, -1);
  if (((Atlas->ShadowY == 2000000.0f) || ((Atlas->OnGround & 1U) == 0)) ||
      (0.75f <= Atlas->Position.y - Atlas->ShadowY)) {
    Atlas->SurfaceType = 0;
  } else {
    Atlas->SurfaceType = ShadowInfo();
  }
  if (Atlas->SurfaceType == 3) {
    KillAtlasphere(Atlas);
  }
  if (Atlas->SurfaceType == 0xb) {
    Atlas->BoostTimer = 1.5f;
    Temp = NuFsqrt(Atlas->Velocity.x * Atlas->Velocity.x +
                   Atlas->Velocity.z * Atlas->Velocity.z);
    if (Temp != 0.0f) {
      Temp = (ATLASBOOSTSPEED / Temp);
      MyGameSfx(0xb7, &Atlas->Position, ATLASBOOSTVOL);
    }
    if (Temp > 1.0f) {
      Atlas->Velocity.x *= Temp;
      Atlas->Velocity.z *= Temp;
    }
    if (Atlas->BoostTimer < 0.5f) {
      Atlas->BoostTimer = 0.5f;
    }
  }
  if ((Atlas->OnGround & 3U) != 0) {
    Volume = NuVecDist(&Atlas->LastPosition, &Atlas->Position, NULL) *
             Atlas->Radius * 20.0f;
    VolumeI = GetVolumeI(Volume);
    ATLASLOOPVOL = Volume;
    MyGameSfxLoop(0x41, &Atlas->Position, VolumeI);
    NewRumble(
        &player->rumble,
        (int)(BALLRUMBLESLOPE * (1.0f - (1.0f - Volume) * (1.0f - Volume))));
  }
}

// NGC MATCH
void MoveAtlas(struct creature_s *Cre, struct nupad_s *Pad) {
  struct ATLASSTRUCT *Atlas;
  u16 old_yrot;
  float dx;
  float dz;

  Atlas = (struct ATLASSTRUCT *)Cre->Buggy;
  if (ProcessTimer(&AtlasWhackTimer) != 0) {
    AtlasWhackValue = 0.0f;
  }
  if ((LIFTPLAYER != 0) && ((Pad->paddata & 0x10) != 0)) {
    Atlas->Position.y = Atlas->Position.y + 0.1f;
    Atlas->Velocity.y = 0.0f;
  }
  AtlasFrame++;
  if (0x11d < AtlasFrame) {
    kj++;
  }
  ControlAtlas(Atlas, Pad, 0.01666667f);
  ProcessAtlas(Atlas);
  Cre->obj.pos.x = Atlas->Position.x;
  Cre->obj.pos.y = Atlas->Position.y - CData[0x53].radius;
  Cre->obj.pos.z = Atlas->Position.z;
  Cre->obj.mom = Atlas->Velocity;
  dx = Cre->obj.pos.x - Cre->obj.oldpos.x;
  dz = Cre->obj.pos.z - Cre->obj.oldpos.z;
  Cre->obj.xz_distance = NuFsqrt(dx * dx + dz * dz);
  old_yrot = Cre->obj.hdg;
  if ((Cre->obj.pad_speed > 0.0f) || (Cre->obj.xz_distance > 0.008333334f)) {
    Cre->obj.thdg = (short)NuAtan2D(dx, dz) + 0x8000;
  }
  Cre->obj.hdg = SeekRot(Cre->obj.hdg, Cre->obj.thdg, 4);
  Cre->obj.dyrot = (short)RotDiff(old_yrot, Cre->obj.hdg);
  PlayerCreatureCollisions(&Cre->obj);
  HitItems(&Cre->obj);
  if ((HitCrates(&Cre->obj, 1) != 0) &&
      ((temp_crate_type == 0x10 || (temp_crate_type == 9)))) {
    KillPlayer(&Cre->obj, 0xb);
  }
  WumpaCollisions(&Cre->obj);
  Cre->obj.attack = 0x430;
  ProcessAtlasTrail(Atlas);
  if (((Level == 0x15) && (Atlas->HitPoints > Atlas->DestHitPoints)) &&
      (GDeb[0x3d].i != -1)) {
    AddVariableShotDebrisEffect(GDeb[0x3d].i, &Atlas->Position, 1, 0, 0);
  }
  Atlas->InterestPoint = Atlas->Position;
}

// NGC MATCH
void ObjectToAtlas(struct obj_s *obj, struct creature_s *c) {
  struct ATLASSTRUCT *atlas;

  atlas = (struct ATLASSTRUCT *)c->Buggy;
  atlas->Position.x = obj->pos.x;
  atlas->Position.y = obj->pos.y + CData[0x53].radius;
  atlas->Position.z = obj->pos.z;
  if (temp_xzmomset != 0) {
    atlas->Velocity.x = obj->mom.x / 0.01666667f;
    atlas->Velocity.z = obj->mom.z / 0.01666667f;
  }
  if (obj->boing != 0) {
    obj->mom.y = 3.333333f;
  }
  atlas->Velocity.y = obj->mom.y;
}

// NGC MATCH
void DrawEarthBoss(void) {
  struct nuvec_s Scale;
  float Temp;

  if (EarthBoss.Dead == 0) {
    DrawJonny();
    if (EarthBoss.DrawCrunch != 0) {
      NuMtxSetRotationY(&mTEMP, (int)(EarthBoss.CrunchY * 182.0444f));
      NuMtxTranslate(&mTEMP, &EarthBoss.Position);
      mTEMP._31 = mTEMP._31 - EarthBoss.Radius;
      MyDrawModelNew(&EarthBoss.Crunch, &mTEMP, NULL);
    }
    if (EarthBoss.DrawShell != 0) {
      Temp = (EarthBoss.Radius / 0.65f) * 2.56f;
      Scale = SetNuVec(Temp, Temp, Temp);
      NuQuatToMtx(&EarthBoss.Quat, &mTEMP);
      NuMtxScale(&mTEMP, &Scale);
      NuMtxTranslate(&mTEMP, &EarthBoss.Position);
      MyDrawModelNew(&EarthBoss.Shell, &mTEMP, NULL);
    }
  }
  return;
}

// NGC MATCH
void UpdateRumbleCamTween(void) {
  if (RumbleCamTweenDest > RumbleCamTween) {
    RumbleCamTween += 0.01666667f;
    if (RumbleCamTweenDest < RumbleCamTween) {
      RumbleCamTween = RumbleCamTweenDest;
    }
  }
  if (RumbleCamTweenDest < RumbleCamTween) {
    RumbleCamTween -= 0.01666667f;
    if (RumbleCamTweenDest > RumbleCamTween) {
      RumbleCamTween = RumbleCamTweenDest;
    }
  }
  if ((EarthBoss.Action != 7) || (EarthBoss.HitPoints > 0)) {
    RumbleCamTweenInterest = (RumbleCamTween - 0.25f) * RUMZOOM;
  }
  if (RumbleCamTweenInterest < 0.0f) {
    RumbleCamTweenInterest = 0.0f;
  }
}

// NGC MATCH
void ProcessEarthBossActions(struct ATLASSTRUCT *EarthBoss) {
  struct nuvec_s delta;
  float dist;
  float ang;

  switch (EarthBoss->Action) {
  case 0: // Idle
    MyAnimateModelNew(&EarthBoss->Shell, 1.0f);
    dist = NuVecDistSqr(&PlayerGlider.Position, &EarthBoss->Position, &delta);
    if (dist < 2500.0f) {
      EarthBoss->Action = 1;
      EarthBoss->ActionTimer = 3.0f;
    }
    break;

  case 1: // Aggro
    MyAnimateModelNew(&EarthBoss->Shell, 1.0f);
    if (ProcessTimer(&EarthBoss->ActionTimer) != 0) {
      EarthBoss->Action = 2;
      EarthBoss->ActionTimer = 5.0f;
    }

    NuVecSub(&delta, &PlayerGlider.Position, &EarthBoss->Position);
    ang = NuAtan2D(delta.x, delta.z) / 182.04445f;
    SeekAngHalfLife360f(&EarthBoss->AngleY, ang, 0.3f, 0.01666667f);
    break;

  case 2: // Attack
    MyAnimateModelNew(&EarthBoss->Shell, 1.5f);
    if (ProcessTimer(&EarthBoss->ActionTimer) != 0) {
      EarthBoss->Action = 3;
      EarthBoss->ActionTimer = 2.0f;
    }

    NuVecSub(&delta, &PlayerGlider.Position, &EarthBoss->Position);
    ang = NuAtan2D(delta.x, delta.z) / 182.04445f;
    SeekAngHalfLife360f(&EarthBoss->AngleY, ang, 0.5f, 0.01666667f);

    // Fire projectile
    if (ProcessTimer(&EarthBoss->BoostTimer) != 0) {
      struct nuvec_s FireDir;
      struct numtx_s Mat;

      NuVecSub(&FireDir, &PlayerGlider.Position, &EarthBoss->Position);
      NuVecNorm(&FireDir, &FireDir);
      NuVecScale(-80.0f, &FireDir, &FireDir);

      NuMtxSetRotationY(&Mat, (s32)(EarthBoss->AngleY * 182.04445f));
      AddGliderBullet(&Mat, &EarthBoss->Position, &FireDir, 1);
      EarthBoss->BoostTimer = 0.5f;
      MyGameSfx(0x60, &EarthBoss->Position, 0x3fff);
    }
    break;

  case 3: // Vulnerable
    MyAnimateModelNew(&EarthBoss->Shell, 0.5f);
    if (ProcessTimer(&EarthBoss->ActionTimer) != 0) {
      EarthBoss->Action = 0;
    }

    CollideGliderBullets(&EarthBoss->Position, 5.0f, 1, 1.0f, 0, 0);
    if (CollideGliderBullets(&EarthBoss->Position, 5.0f, 0, 1.0f, 0, 0) != 0) {
      EarthBoss->HitPoints--;
      AddScreenWumpa(EarthBoss->Position.x, EarthBoss->Position.y,
                     EarthBoss->Position.z, 1);
      if (EarthBoss->HitPoints <= 0) {
        EarthBoss->Action = 4;
        AddGameDebris(0x47, &EarthBoss->Position);
        MyGameSfx(0xb4, &EarthBoss->Position, 0x7fff);
      } else {
        MyGameSfx(0xbb, &EarthBoss->Position, 0x3fff);
      }
      ClockOff();
    }
    break;

  case 4: // Dead
    EarthBoss->Whacko = 0;
    break;
  }

  // Collision with player
  if (EarthBoss->Whacko != 0) {
    dist = NuVecDistSqr(&PlayerGlider.Position, &EarthBoss->Position, &delta);
    if (dist < 100.0f) {
      NuVecSub(&delta, &PlayerGlider.Position, &EarthBoss->Position);
      NuVecNorm(&delta, &delta);
      NuVecScale(10.0f, &delta, &delta);
      NuVecAdd(&PlayerGlider.Velocity, &PlayerGlider.Velocity, &delta);

      if (InvincibilityCHEAT == 0 && VehicleLevelImmune == 0) {
        PlayerGlider.HitPoints -= 25;
        if (PlayerGlider.HitPoints < 0) {
          PlayerGlider.HitPoints = 0;
        }
      }
      NewBuzz(&player->rumble, 6);
      NewRumble(&player->rumble, 0xb4);
    }
  }
}


// NGC MATCH
void RumbleHeadUpDisplay(void) {
  s32 i;
  s32 Obj;
  struct nuvec_s Pos;
  struct nuvec_s Pos2;
  short CamAngY;
  float Scale;

  for (i = 0; i < NumRockPanel; i++) {
    Obj = RockPanelObj[i];
    DrawPanel3DObject(Obj, RockPanelData[i].x, RumbleDisplayY,
                      RockPanelData[i].z, RockPanelScale, RockPanelScale,
                      RockPanelScale, RockPanelRotX[i], RockPanelRotY[i],
                      RockPanelRotZ[i], ObjTab[Obj].obj.scene,
                      ObjTab[Obj].obj.special, 1);
  }
  Scale = RADARBASESCALE * RADARSCALE;
  Pos = v000;
  CamAngY = -GameCam[0].yrot;
  Obj = 0xa2;
  DrawPanel3DObject(Obj, Pos.x * RADARSCALE + RadarX,
                    Pos.z * RADARSCALE + RadarY, RadarZ, Scale, Scale, Scale, 0,
                    0, 0, ObjTab[Obj].obj.scene, ObjTab[Obj].obj.special, 1);
  Scale = (PlayerAtlas.Radius * RADARSCALE * DOTSCALE);
  NuVecRotateY(&Pos, &PlayerAtlas.Position, CamAngY);
  Obj = 0xa5;
  DrawPanel3DObject(Obj, Pos.x * RADARSCALEX + RadarX,
                    Pos.z * RADARSCALE + RadarY, RadarZ, Scale, Scale, Scale, 0,
                    0, 0, ObjTab[Obj].obj.scene, ObjTab[Obj].obj.special, 1);
  if (EarthBoss.Dead == 0) {
    Scale = (EarthBoss.Radius * RADARSCALE * DOTSCALE);
    NuVecRotateY(&Pos, &EarthBoss.Position, CamAngY);
    Obj = 0xa3;
    DrawPanel3DObject(Obj, Pos.x * RADARSCALEX + RadarX,
                      Pos.z * RADARSCALE + RadarY, RadarZ, Scale, Scale, Scale,
                      0, 0, 0, ObjTab[Obj].obj.scene, ObjTab[Obj].obj.special,
                      1);
  }
  Scale = (RADARSCALE * 0.5f * DOTSCALE);
  for (i = 0; i < 6; i++) {
    if (JeepRock[i].Active != 0) {
      NuVecRotateY(&Pos, &JeepRock[i].Pos, CamAngY);
      switch (JeepRock[i].Mode) {
      case 1:
      case 0x14:
        Obj = 0xa6;
        break;
      case -1:
        Obj = 0xa4;
        break;
      default:
        Obj = 0xa7;
        break;
      }
      DrawPanel3DObject(Obj, Pos.x * RADARSCALEX + RadarX,
                        Pos.z * RADARSCALE + RadarY, RadarZ, Scale, Scale,
                        Scale, 0, 0, 0, ObjTab[Obj].obj.scene,
                        ObjTab[Obj].obj.special, 1);
    }
  }
  if (FindNearestCreature(&(player->obj).pos, 0xa7, &Pos2) < 1000000.0f) {
    Scale = ((GameTimer.frame % 0x1e + 0x1e) * RADARSCALE * DOTSCALE) / 60.0f;
    NuVecRotateY(&Pos2, &Pos2, CamAngY);
    Obj = 0xa4;
    DrawPanel3DObject(Obj, Pos2.x * RADARSCALEX + RadarX,
                      Pos2.z * RADARSCALE + RadarY, RadarZ, Scale, Scale, Scale,
                      0, 0, 0, ObjTab[Obj].obj.scene, ObjTab[Obj].obj.special,
                      1);
  }
}

// NGC MATCH
void CheckAtlasVortex(struct ATLASSTRUCT *Atlas) {
  struct nuvec_s VortexPosition = {0.0f, -3.3f, 0.0f};
  struct nuvec_s Rel;
  float Dist;

  if (Atlas->Position.y < -3.0f) {
    VortexPosition.y = Atlas->Position.y - 0.3f;
  }
  NuVecSub(&Rel, &VortexPosition, &Atlas->Position);
  Dist = NuVecMag(&Rel);
  if (Dist < 1.0f) {
    NuVecScaleAccum(0.08333334f / Dist, &Atlas->Velocity, &Rel);
  }
}

// NGC MATCH
void ProcessAtlasAtlasCollisions(void) {
  if ((EarthBoss.HitPoints > 0) && (PlayerAtlas.HitPoints > 0)) {
    ProcessAtlasAtlasCollisions_a(&PlayerAtlas, &EarthBoss);
  }
}

// NGC MATCH
void InitTrail(void) {
  s32 loop;

  for (loop = 0; loop < 0x80; loop++) {
    trail[loop].pos1.x = -10000.0f;
  }
  trailpt = 0;
  trailair = 0;
}

// NGC MATCH
void FadeOutLastTrail(s32 pos, s32 count) {
  s32 i;
  s32 idx;
  float step;
  float accum;
  float val;

  step = 1.0f / (float)(count + 1);
  accum = step + 0.0f;
  i = 0;
  if (i >= count) return;

  do {
    idx = pos & 0x7f;
    if (trail[idx].pos1.x == -10000.0f) break;
    val = (float)trail[idx].intensity;
    accum += step;
    trail[idx].intensity = (s32)(val * accum);
    pos--;
    i++;
  } while (i < count);
}


// NGC MATCH
s32 PointsSame(struct nuvec_s *A, struct nuvec_s *B) {
  s32 ret = 0;

  if ((A->x == B->x) && (A->y == B->y)) {
    ret = (A->z == B->z) ? 1 : 0;
  }
  return ret;
}

// NGC MATCH
float PointLineSide(struct nuvec_s *A, struct nuvec_s *B, struct nuvec_s *C) {
  float ACx;
  float ACz;
  float BCx;
  float BCz;

  ACx = C->x - A->x;
  BCx = C->x - B->x;
  BCz = C->z - A->z;
  ACz = C->z - B->z;

  return ACx * ACz - BCz * BCx;
}

// NGC MATCH
s32 TrailPointInPoly(struct nuvec_s *Point, struct nuvec_s *A, struct nuvec_s *B,
                     struct nuvec_s *C, struct nuvec_s *D) {
  s32 i;
  struct nuvec_s *Src;
  struct nuvec_s *Pt;
  float side;

  Src = A;
  Pt = Point;
  for (i = 0; i <= 1; i++) {
    side = PointLineSide(Pt, A, B);
    if (side > 0.0f) {
      if (PointLineSide(Pt, B, C) > 0.0f) {
        if (PointLineSide(Pt, C, D) > 0.0f) {
          if (PointLineSide(Pt, D, A) > 0.0f) {
            goto found;
          }
        }
      }
    } else {
      if (PointLineSide(Pt, B, C) <= 0.0f) {
        if (PointLineSide(Pt, C, D) <= 0.0f) {
          if (PointLineSide(Pt, D, A) <= 0.0f) {
            goto found;
          }
        }
      }
    }
    Src = B;
    Pt = Point + 1;
    continue;
  found:
    *Pt = *Src;
    return 1;
  }
  return 0;
}


// NGC MATCH
s32 FindTrailAng(struct nuvec_s *A, struct nuvec_s *B) {
  struct nuvec_s Line;

  NuVecSub(&Line, B, A);
  return (NuAtan2D(Line.x, Line.z) - 0x2000) & 0xffff;
}

// NGC MATCH
void DrawVehicleTrail(void) { DrawJeepTrails(); }

// NGC MATCH
struct JEEPROCK *FindJeepRock(void) {
  s32 i;

  for (i = 0; i < 6; i++) {
    if (JeepRock[i].Active == 0) {
      return &JeepRock[i];
    }
  }
  return NULL;
}

// NGC MATCH
void InitJeepRocks(void) {
  s32 i;

  for (i = 0; i < 6; i++) {
    JeepRock[i].Active = 0;
  }
}

// NGC MATCH
void KeepHoldOnRock(struct JEEPROCK *Rock, struct nuvec_s *Pos,
                    struct nuvec_s *Vel) {
  Rock->Pos = *Pos;
  Rock->Atlas.Position = *Pos;
  Rock->Atlas.OldPosition = Rock->Atlas.Position;
  Rock->Vel = *Vel;
  Rock->Atlas.Velocity = *Vel;
  Rock->Grabbed = 1;
}

// NGC MATCH
struct JEEPROCK *AddRockVel(struct nuvec_s *Pos, struct nuvec_s *Vel,
                            float Radius, s32 Type) {
    struct JEEPROCK *Rock;
    struct nuquat_s quat;
    s32 index;

    Rock = FindJeepRock();
    if (Rock == NULL) {
        return NULL;
    }

    memset(Rock, 0, sizeof(struct JEEPROCK));

    NuMtxSetRotationY(&mTEMP,
                       (int)(frand() * 180.0f * (1.0f / 60.0f) * 182.04445f));
    NuMtxRotateX(&mTEMP,
                 (int)(frand() * 720.0f * (1.0f / 60.0f) * 182.04445f));
    NuMtxToQuat(&mTEMP, &quat);
    NuQuatNormalise(&quat, &quat);

    Rock->Pos = *Pos;

    switch (Type) {
    case 1:
        Rock->FireBlob = Type;
        break;
    case 2:
        Rock->SmashMe = 1;
        break;
    default:
        break;
    }

    Rock->Scale = SetNuVec(Radius + Radius, Radius + Radius, Radius + Radius);

    Rock->Active = 1;
    Rock->Vel = *Vel;
    Rock->Life = 5.5f;

    index = Rock - JeepRock;
    InitAtlas(&Rock->Atlas, &Rock->Pos, Radius, index);

    Rock->Atlas.Position = *Vel;
    Rock->Atlas.Quat = *(struct Quat *)&quat;
    Rock->Atlas.FrameQuat[0] = *(struct Quat *)&quat;
    Rock->Atlas.FrameQuat[1] = *(struct Quat *)&quat;
    Rock->Atlas.FrameQuat[2] = *(struct Quat *)&quat;
    Rock->Atlas.LastQuat = *(struct Quat *)&quat;

    Rock->Atlas.PlatformId = 1;
    Rock->Atlas.Type = index - 3;

    return Rock;
}


// NGC MATCH
struct JEEPROCK *AddRock(struct nuvec_s *Pos, float Radius, s32 Type) {
  return AddRockVel(Pos, &v000, Radius, Type);
}

// NGC MATCH
void DrawJeepRock(struct JEEPROCK *Rock) {
  s32 i;

  NuQuatToMtx(&(Rock->Atlas).Quat, &mTEMP);
  NuMtxPreScale(&mTEMP, &Rock->Scale);
  NuMtxTranslate(&mTEMP, &(Rock->Atlas).Position);
  if (Level == 0x16) {
    i = 0x58;
  } else {
    if ((Rock->Mode == 0x14) || (Rock->Mode == 1)) {
      i = 0xa9;
    } else {
      i = 0xaa;
      if (Rock->Mode == -1) {
        i = 0xab;
      }
    }
  }
  Rock->Seen = 0;
  if (ObjTab[i].obj.special != NULL) {
    Rock->Seen = NuRndrGScnObj(
        (ObjTab[i].obj.scene)->gobjs[ObjTab[i].obj.special->instance->objid],
        &mTEMP);
  }
}

// NGC MATCH
void ShootRoksSkyward(void) {
  s32 i;

  RumbleDisplayMode = -1;
  MyGameSfx(0xb9, &EarthBoss.Position, 0x7fff);
  ShootRockSound = 1;
  for (i = 0; i < 6; i++) {
    if (JeepRock[i].Active != 0) {
      JeepRock[i].Atlas.Velocity = SetNuVec(0.0f, 13.0f, 0.0f);
      JeepRock[i].Mode = 0x14;
      JeepRock[i].Atlas.BeenHit = 0;
      JeepRock[i].FlameTimer = 10.0f;
    }
  }
}

// NGC MATCH
s32 AllRoksSkyward(void) {
  s32 i;
  s32 j;

  j = 1;
  for (i = 0; i < 6; i++) {
    if (JeepRock[i].Active != 0) {
      if (JeepRock[i].Pos.y < 10.0f) {
        if ((JeepRock[i].Mode != 0x14) ||
            (JeepRock[i].Atlas.Velocity.y < 5.0f)) {
          JeepRock[i].Active = 0;
        }
        j = 0;
      } else {
        JeepRock[i].Active = 0;
      }
    }
  }
  if (j != 0) {
    RumbleDisplayMode = 0;
    ShootRockSound = 0;
  }
  return j;
}

// NGC MATCH
s32 GetRumbleTotalRoks(void) {
  s32 i;
  s32 j;

  if (RumbleDisplayMode == -1) {
    return RumbleStoreTotalRoks;
  }
  for (i = 0, j = 0; i < 6; i++) {
    if (JeepRock[i].Active != 0) {
      j++;
    }
  }
  RumbleStoreTotalRoks = j;
  return RumbleStoreTotalRoks;
}

// NGC MATCH
s32 GetRumbleCrunchRoks(void) {
  s32 i;
  s32 j;

  if (RumbleDisplayMode == -1) {
    return RumbleStoreCrunchRoks;
  }
  for (i = 0, j = 0; i < 6; i++) {
    if (((JeepRock[i].Active != 0) && (0.0f < JeepRock[i].FlameTimer)) &&
        (JeepRock[i].Mode == -1)) {
      j++;
    }
  }
  RumbleStoreCrunchRoks = j;
  return RumbleStoreCrunchRoks;
}

// NGC MATCH
s32 GetRumblePlayerRoks(void) {
  s32 i;
  s32 j;

  if (RumbleDisplayMode == -1) {
    return RumbleStoreCrashRoks;
  }
  for (i = 0, j = 0; i < 6; i++) {
    if (((JeepRock[i].Active != 0) && (JeepRock[i].FlameTimer > 0.0f)) &&
        (JeepRock[i].Mode == 1)) {
      j++;
    }
  }
  RumbleStoreCrashRoks = j;
  return RumbleStoreCrashRoks;
}

// NGC MATCH
void SmashRockIntoTwo(struct JEEPROCK *Rock) {
  struct nuvec_s vel;
  struct nuvec_s pos;
  float NewRad;

  NewRad = Rock->Atlas.Radius / 1.2599f;
  vel = Rock->Vel;
  pos = Rock->Pos;
  pos.x -= (NewRad * 0.5f);
  vel.x -= 2.0f;
  AddRockVel(&pos, &vel, NewRad, 0);
  pos.x = (NewRad * 1.01f + pos.x);
  vel.x += 4.0f;
  AddRockVel(&pos, &vel, NewRad, 0);
  Rock->Explode = 1;
}

// NGC MATCH
void DrawJeepRocks(void) {
  s32 i;

  for (i = 0; i < 6; i++) {
    if (JeepRock[i].Active != 0) {
      DrawJeepRock(&JeepRock[i]);
    }
  }
}

// NGC MATCH
void ProcessJeepRock(struct JEEPROCK *Rock) {
    struct JEEPROCK *OtherRock;

    if (Level == 21) {
        if (Rock->Mode != 1 && Rock->Mode != 20) {
            if (ProcessTimer(&Rock->FlameTimer) != 0) {
                Rock->Mode = 0;
            }
        }
    }

    if (Rock->Grabbed != 0) {
        Rock->Grabbed = 0;
        return;
    }

    if (Rock->Stuck == 0) {
        if (Rock->Life < 4.0f) {
            Rock->Active = 0;
        }
    }

    if (Rock->SmallDamage > 2) {
        if (Rock->Mode != 20) {
            OtherRock = &JeepRock[Rock->SmallDamage];
            if (OtherRock->Mode != 0) {
                s32 combined = Rock->Mode + OtherRock->Mode;
                if (combined == 0) {
                    OtherRock->Mode = 0;
                    Rock->Mode = 0;
                    OtherRock->FlameTimer = 0.0f;
                    Rock->FlameTimer = 0.0f;
                } else if (OtherRock->Mode == -1) {
                    Rock->Mode = OtherRock->Mode;
                    OtherRock->FlameTimer = 7.0f;
                    Rock->FlameTimer = 7.0f;
                }
                OtherRock->SmallDamage = 0;
            }
            Rock->SmallDamage = 0;
        }
    }

    if (Level == 21) {
        Rock->Life = 10.0f;
    }

    if (ProcessTimer(&Rock->Life) != 0) {
        Rock->Explode = 1;
    } else if (Rock->FireBlob == 2) {
        /* no-op */
    } else if (Rock->Mode != 20) {
        ProcessAtlas(&Rock->Atlas);
        Rock->Pos = Rock->Atlas.Position;

        if (Rock->Atlas.PlatformId != 0) {
            if (Level != 21) {
                Rock->Explode = 1;
            }
        }

        if (Rock->SmashMe != 0) {
            if (Rock->Atlas.SurfaceType & 3) {
                SmashRockIntoTwo(Rock);
            }
        }

        if (Rock->FireBlob == 1) {
            if (Rock->Atlas.SurfaceType & 3) {
                Rock->FireBlob = 2;
            }
        }
    } else {
        NuVecScaleAccum(1.0f / 60.0f, &Rock->Atlas.Position,
                        &Rock->Atlas.Velocity);
        Rock->Pos = Rock->Atlas.Position;
    }

    {
        s32 debrisEffect;

        debrisEffect = GDeb[0x3D].i;

        if (Level == 21) {
            switch (Rock->Mode) {
            case 1:
                debrisEffect = GDeb[0x3E].i;
                break;
            case 20:
                debrisEffect = GDeb[0x40].i;
                break;
            case -1:
                break;
            default:
                debrisEffect = GDeb[0x3F].i;
                break;
            }
        }

        if (debrisEffect != -1) {
            AddVariableShotDebrisEffect(debrisEffect, &Rock->Pos, 1, 0, 0);
        }
    }

    if (Rock->Explode != 0) {
        MyGameSfx(0x56, &Rock->Pos,
                   (s32)(Rock->Atlas.Radius * (float)ROCKSMASHVOL));
        AddGameDebris(0x42, &Rock->Pos);
        Rock->Active = 0;
    }
}


// NGC MATCH
void ProcessJeepRocks(void) {
  s32 i;
  float Powerf;
  float Dist;

  Powerf = 0.0f;
  for (i = 0; i < 6; i++) {
    if (JeepRock[i].Active != 0) {
      ProcessJeepRock(&JeepRock[i]);
    }
  }
  if (Level == 0x16) {
    for (i = 0; i < 6; i++) {
      if (JeepRock[i].Active != 0) {
        if ((JeepRock[i].Atlas.Radius >= 0.3f) &&
            ((JeepRock[i].Atlas.OnGround & 3U) != 0)) {
          Dist =
              NuVecDist(&JeepRock[i].Atlas.Position, &(player->obj).pos, NULL) *
              JeepRock[i].Atlas.Radius / 0.4f;
          if (Dist < 1.0f) {
            Powerf = 255.0f;
          } else {
            Powerf += (255.0f / Dist);
          }
        }
      }
    }
    if (Powerf < 255.0f) {
      Powerf = 255.0f;
    }
    NewRumble(&player->rumble, (s32)Powerf);
  }
}

// NGC MATCH
s32 CheckAgainstRocks(struct nuvec_s *Position, struct nuvec_s *Move) {
  struct nuvec_s Rel;
  struct nuvec_s Pos;
  s32 i;
  s32 Ret;

  Ret = 0;
  if (Level != 0x16) {
    return 0;
  }
  NuVecAdd(&Pos, Position, Move);
  for (i = 0; i < 6; i++) {
    if (JeepRock[i].Active != 0) {
      NuVecSub(&Rel, &Pos, &JeepRock[i].Pos);
      if (NuVecMagSqr(&Rel) < 0.5625f) {
        JeepRock[i].Explode = 1;
        Ret = 1;
      }
    }
  }
  return Ret;
}

// NGC MATCH
void ProcessRockRockCollisions(void) {
  s32 i;
  s32 j;

  for (i = 1; i < 6; i++) {
    if (((JeepRock[i].Active != 0) && (JeepRock[i].Mode != 0x14))) {
      for (j = 0; j < i; j++) {
        if (JeepRock[j].Active != 0) {
          if (JeepRock[j].Mode != 0x14) {
            ProcessAtlasAtlasCollisions_a(&JeepRock[i].Atlas,
                                          &JeepRock[j].Atlas);
          }
        }
      }
    }
  }
  if (Level == 0x15) {
    for (i = 0; i < 6; i++) {
      if (JeepRock[i].Active != 0) {
        if (JeepRock[i].Mode != 0x14) {
          ProcessAtlasAtlasCollisions_a(&JeepRock[i].Atlas, &PlayerAtlas);
          if (EarthBoss.HitPoints > 0) {
            ProcessAtlasAtlasCollisions_a(&JeepRock[i].Atlas, &EarthBoss);
          }
        }
      }
    }
  }
}

// NGC MATCH
void RumbleCam(struct cammtx_s *CamMtx) {
  float IdealY;
  float DeltaAng;
  float fVar6;
  float fVar7;
  float fVar8;
  struct nuvec_s DeltaVec;
  struct nuvec_s CamPos;
  struct nuvec_s Interest;

  NuVecScale(RumbleCamTween, &Interest, &EarthBoss.InterestPoint);
  NuVecScaleAccum(1.0f - RumbleCamTween, &Interest, &PlayerAtlas.InterestPoint);
  Interest.y +=
      (1.0f - RumbleCamTweenInterest) + (1.0f - RumbleCamTweenInterest);
  fVar6 = ((float)NuAtani((-Interest.x * 256.0f), (-Interest.z * 256.0f))) /
          182.04445f;
  if (ResetAtlasCamera != 0) {
    ResetAtlasCamera = 0;
    AngleY_836 = fVar6;
  }
  DeltaAng = Rationalise360f(fVar6 - AngleY_836);
  fVar8 = NuFsqrt(Interest.x * Interest.x + Interest.z * Interest.z);
  IdealY = 3.0000002f;
  if (fVar8 < 5.0f) {
    DeltaAng = (DeltaAng * ((fVar8 / 5.0f) * (fVar8 / 5.0f)));
  }
  if (DeltaAng > 3.0000002f) {
    DeltaAng = IdealY;
  } else {
    if (DeltaAng < -IdealY) {
      DeltaAng = -3.0000002f;
    }
  }
  AngleY_836 = Rationalise360f(AngleY_836 + DeltaAng);
  NuVecRotateY(&CamPos, SetNuVecPntr(0.0f, 5.0f, 9.9f),
               (int)(AngleY_836 * 182.04445f));
  Interest.y += RumbleCamVal;
  NuVecScale(1.0f - RumbleCamTweenInterest, &CamPos, &CamPos);
  NuVecScaleAccum(RumbleCamTweenInterest, &CamPos, &Interest);
  DeltaVec.x = Interest.x - CamPos.x;
  DeltaVec.y = Interest.y - CamPos.y;
  DeltaVec.z = Interest.z - CamPos.z;
  fVar7 = NuFsqrt(DeltaVec.x * DeltaVec.x + DeltaVec.z * DeltaVec.z);
  CamMtx->xrot = NuAtani((int)(-DeltaVec.y * 256.0f), (int)(fVar7 * 256.0f));
  CamMtx->yrot =
      NuAtani((int)(DeltaVec.x * 256.0f), (int)(DeltaVec.z * 256.0f));
  NuMtxSetRotationX(&CamMtx->m, CamMtx->xrot);
  NuMtxRotateY(&CamMtx->m, CamMtx->yrot);
  NuMtxTranslate(&CamMtx->m, &CamPos);
  CamMtx->pos = CamPos;
  RumbleCamPos = CamPos;
  RumbleCamY = CamMtx->yrot;
}

// NGC MATCH
void DrawVehMasks(void) {
  struct VEHMASK *Mask;
  s32 d;
  s32 i;
  s32 iVar2;
  struct nuvec_s vec;

  Mask = VehicleMask;

  for (i = 0; i < 2; i++, Mask++) {
    if (Mask->Active != 0) {
      if (Level == 0x17) {
        NuMtxSetRotationY(&mTEMP,
                          NuAtan2D(GameCam[0].pos.x - Mask->Position.x,
                                   GameCam[0].pos.z - Mask->Position.z) +
                              0x8000);
      } else {
        NuMtxSetRotationY(&mTEMP, (s32)(Mask->DrawAngY * 182.04445f));
      }
      NuMtxScale(&mTEMP, SetNuVecPntr(Mask->DrawScale, Mask->DrawScale,
                                      Mask->DrawScale));
      NuMtxTranslate(&mTEMP, &Mask->Position);
      d = MyDrawModelNew(&Mask->MainDraw, &mTEMP, NULL);

      Mask->Seen = d;
      vec = Mask->Position;
      vec.y += 0.2f;

      iVar2 = 0x7e;
      switch (Mask->Id) {
      case 0x57:
        iVar2 = 0x7f;
        break;
      case 0x56:
        iVar2 = 0x80;
        break;
      case 0x58:
        iVar2 = 0x81;
        break;
      case 3:
        iVar2 = 0xa3;
        break;
      }

      if (Paused == 0) {
        AddVariableShotDebrisEffect(GDeb[iVar2].i, &vec, 1, 0, 0);
      }
    }
  }
}
