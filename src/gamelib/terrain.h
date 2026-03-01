#ifndef TERRAIN_H
#define TERRAIN_H

#include "../types.h"
#include "../nu.h"

typedef enum terr_type {
    TERR_TYPE_NORMAL = 0,
    TERR_TYPE_PLATFORM = 1,
    TERR_TYPE_WALLSPL = 2,
    TERR_TYPE_CRASHDATA = 3,
    TERR_TYPE_EMPTY = 255
} TERR_TYPE;

typedef struct TERRAINFLAGS_s {
    u32 rot;
} TERRAINFLAGS;

typedef struct platattrib {
    unsigned int hit : 1;
    unsigned int rotate : 1;
} PLATATTRIB;

typedef struct {
    struct nuvec_s offset;
    float ang;
    float size;
} TerrShapeType;

typedef struct {
    int offset;
    struct nuvec_s translation;
    short type;
    short info;
    short rx;
    short ry;
    short rz;
    short pad;
    struct nuvec_s rotation;
    TERRAINFLAGS flags;
    short prim;
    short id;
    int datapos;
} OffType;

typedef struct {
    int count;
    OffType offlist[512];
} OffFileType;

typedef struct {
    float minx;
    float maxx;
    float miny;
    float maxy;
    float minz;
    float maxz;
    struct nuvec_s pnts[4];
    struct nuvec_s norm[2];
    unsigned char info[4];
} tertype;

typedef struct {
    struct nuvec_s Location;
    short *model;
    struct nuvec_s min;
    struct nuvec_s max;
    TERRAINFLAGS flags;
    TERR_TYPE type;
    short info;
    short id;
    float radius;
} terrsitu;

typedef struct {
    unsigned short count;
    unsigned short val;
    struct nuvec_s spl[4096];
} WallSpline;

typedef struct {
    short tabindex;
    short count;
    float minx;
    float minz;
    float maxx;
    float maxz;
} terrgrouptype;

typedef struct {
    struct nuvec_s origpos;
    struct nuvec_s origvel;
    struct nuvec_s curpos;
    struct nuvec_s curvel;
    short id;
    short scanmode;
    float stopflag;
    float vellen;
    unsigned char *flags;
    float ax;
    float ay;
    float len;
    float size;
    float sizesq;
    float sizediv;
    float yscale;
    float yscalesq;
    float inyscale;
    float inyscalesq;
    short hitcnt;
    short hitterrno;
    float csx;
    float csy;
    float csz;
    float cex;
    float cey;
    float cez;
    short hittype;
    short plathit;
    short *PlatScanStart;
    tertype *hitter;
    float hittime;
    float timeadj;
    float impactadj;
    struct nuvec_s hitnorm;
    struct nuvec_s uhitnorm;
    struct nuvec_s tempvec[2];
    tertype rotter;
    tertype *hitdata[512];
} TerTempInfoType;

typedef struct {
    float x;
    float y;
    float z;
    float x2;
    float y2;
    float z2;
    short timer;
    short pad;
    tertype *hitdata[512];
} pollisttype;

typedef struct {
    struct nuvec_s pos;
    float radius;
} Shperetype;

typedef struct {
    struct numtx_s oldmtx;
    struct numtx_s *curmtx;
    short terrno;
    short instance;
    PLATATTRIB status;
    short hitcnt;
    short pad;
    float plrgrav;
    float ypos;
    float yvel;
    float tension;
    float damp;
} TERRPLATTYPE;

typedef struct {
    void *ptrid;
    short platid;
    short platinf;
    short timer;
    short pad;
} TERRINFO;

typedef struct {
    terrsitu *terr;
    TERRPLATTYPE platdata[128];
    void *wallinfo;
    TERRINFO TrackInfo[4];
    terrgrouptype terrgroup[257];
    int terrainlow;
    int terrgcnt;
    short terrlist[4096];
    pollisttype pollist[9];
} TERRSET;

typedef struct {
    terrsitu *terr;
    terrsitu *curlist;
    short idlookup[128];
    TERRPLATTYPE platdata[128];
    void *wallinfo;
} TERRPICKUPSET;

extern struct nuvec_s ShadNorm;
extern struct nuvec_s EShadNorm;
extern float EShadY;
extern int terrhitflags;
extern struct nuvec_s ShadRoofNorm;
extern struct nuvec_s EShadRoofNorm;
extern float ShadRoofY;
extern float EShadRoofY;
extern TERRSET *CurTerr;
extern TERRPICKUPSET *PickupTerr;
extern Shperetype SphereData[16];
extern int curSphereter;
extern int curPickInst;
extern struct numtx_s tertempmat;
extern struct nuvec4_s tertempvec4;
extern TERRINFO *CurTrackInfo;
extern TerTempInfoType *TerI;
extern tertype *TerrPoly;
extern tertype TerrPolyInfo;
extern int TerrPolyObj;
extern tertype *ShadPoly;
extern tertype *EShadPoly;
extern tertype *ShadRoofPoly;
extern tertype *EShadRoofPoly;
extern tertype ScaleTerrainT1[512];
extern tertype ScaleTerrainT2[512];
extern struct nuvec_s WallSplList[128];
extern int WallSplCount;
extern tertype *ScaleTerrain;
extern TerrShapeType *TerrShape;
extern int TerrShapeAdjCnt;
extern int PlatImpactId;
extern struct nuvec_s PlatImpactNorm;
extern tertype PlatImpactTer;
extern int PlatCrush;
extern int plathitid;
extern short castnum;
extern short ecastnum;
extern short castroofnum;
extern short ecastroofnum;
extern short shadhit;
extern short eshadhit;
extern short shadroofhit;
extern short eshadroofhit;
extern int terraincnt;
extern int situtotal;
extern int platinrange;
extern void *crashdata;
extern int terlistptr;
extern short TempScanStack[4096];
extern short *TempStackPtr;
extern int TerrPlatDis;
extern int testlock;

#endif
