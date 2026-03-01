#ifndef NUGCUT_H
#define NUGCUT_H

#include "../types.h"
#include "../nu.h"

typedef struct NUGCUTSCENE_s NUGCUTSCENE_s;
typedef struct instNUGCUTSCENE_s instNUGCUTSCENE_s;
typedef struct NUGCUTCAMSYS_s NUGCUTCAMSYS_s;
typedef struct instNUGCUTCAMSYS_s instNUGCUTCAMSYS_s;
typedef struct NUGCUTRIGIDSYS_s NUGCUTRIGIDSYS_s;
typedef struct instNUGCUTRIGIDSYS_s instNUGCUTRIGIDSYS_s;
typedef struct NUGCUTCHARSYS_s NUGCUTCHARSYS_s;
typedef struct instNUGCUTCHARSYS_s instNUGCUTCHARSYS_s;
typedef struct NUGCUTLOCATORSYS_s NUGCUTLOCATORSYS_s;
typedef struct instNUGCUTLOCATORSYS_s instNUGCUTLOCATORSYS_s;
typedef struct NUGCUTTRIGGERSYS_s NUGCUTTRIGGERSYS_s;
typedef struct instNUGCUTTRIGGERSYS_s instNUGCUTTRIGGERSYS_s;
typedef struct NUGCUTCHAR_s NUGCUTCHAR_s;
typedef struct instNUGCUTCHAR_s instNUGCUTCHAR_s;
typedef struct NUGCUTRIGID_s NUGCUTRIGID_s;
typedef struct instNUGCUTRIGID_s instNUGCUTRIGID_s;
typedef struct NUGCUTLOCATOR_s NUGCUTLOCATOR_s;
typedef struct instNUGCUTLOCATOR_s instNUGCUTLOCATOR_s;
typedef struct NUGCUTTRIGGER_s NUGCUTTRIGGER_s;
typedef struct instNUGCUTTRIGGER_s instNUGCUTTRIGGER_s;
typedef struct NUGCUTLOCFNDATA_s NUGCUTLOCFNDATA_s;
typedef struct NUTRIGGERSYS_s NUTRIGGERSYS_s;
typedef struct instNUTRIGGERSYS_s instNUTRIGGERSYS_s;
typedef struct instNUTRIGGER_s instNUTRIGGER_s;

typedef enum locdir {
    LOCATOR_NO_DIRECTION = 0,
    LOCATOR_UP = 1,
    LOCATOR_DOWN = 2,
    LOCATOR_X = 3,
    LOCATOR_NEGX = 4,
    LOCATOR_Y = 5,
    LOCATOR_NEGY = 6,
    LOCATOR_Z = 7,
    LOCATOR_NEGZ = 8
} locdir;

struct NuVec {
    float x;
    float y;
    float z;
};

struct nuquat_s {
    float x;
    float y;
    float z;
    float w;
};

struct nuhspecial_s {
    struct nugscn_s *scene;
    struct nuspecial_s *special;
};

struct NUSTATEANIM_s {
    s32 nchanges;
    float *frames;
    u8 *states;
};

struct NUGCUTTRIGGER_s {
    s16 ix;
    s16 pad;
    char *triggername;
    struct NUSTATEANIM_s *enableflag1anim;
};

struct instNUGCUTTRIGGER_s {
    char next_ix;
    char pad[3];
};

struct instNUTRIGGER_s {
    s16 hitpoints;
    u8 enableflags;
    char flags;
};

struct NUTRIGGERSYS_s {
    s32 ntriggers;
    struct NUGCUTTRIGGER_s *triggers;
};

struct instNUTRIGGERSYS_s {
    struct instNUTRIGGERSYS_s *next;
    struct instNUTRIGGERSYS_s *prev;
    struct NUTRIGGERSYS_s *triggersys;
    struct instNUTRIGGER_s *itriggers;
    void *scenemanager;
    int is_disabled : 1;
};

struct instNUGCUTTRIGGERSYS_s {
    struct instNUTRIGGERSYS_s *itriggersys;
    struct instNUGCUTTRIGGER_s *itriggers;
};

struct NUGCUTBBOX_s {
    struct nuvec_s min;
    struct nuvec_s max;
};

struct NUGCUTTRIGGERSYS_s {
    s32 ntriggers;
    struct NUGCUTTRIGGER_s *triggers;
};

struct NUGCUTCAM_s {
    struct numtx_s mtx;
    u8 flags;
    u8 id;
    u8 anim_ix;
    char pad[13];
};

struct instNUGCUTCAM_s {
    u8 flags;
    u8 tgt_ix;
    char pad[2];
};

struct NUGCUTCAMTGT_s {
    struct nuvec_s *tgt;
    float frame;
    float nframes;
    char camid;
    char pad[3];
};

struct NUGCUTCAMSYS_s {
    u32 ncutcams;
    struct NUGCUTCAM_s *cutcams;
    struct nuanimdata2_s *camanim;
    struct NUSTATEANIM_s *camchanges;
    char initial_camid;
    char remap_table[10];
    u8 pad;
};

struct instNUGCUTCAMSYS_s {
    struct NUGCUTCAMTGT_s *tgts;
    struct instNUGCUTCAM_s *icutcams;
    u8 next_switch;
    char current_camera;
    u8 next_tgt_ix;
    u8 tgtarraysize;
    u8 ntgts;
    char pad[3];
};

struct NUGCUTLOCATORTYPE_s {
    char *name;
    u8 flags;
    char pad0;
    u16 ix;
    char pad[4];
};

struct NUGCUTLOCATOR_s {
    struct numtx_s mtx;
    struct nuvec_s pivot;
    float rate;
    struct nuanimdata2_s *anim;
    enum locdir direction;
    u8 flags;
    u8 locator_type;
    u8 joint_ix;
    char pad;
    float param1;
    float param2;
};

struct instNUGCUTLOCATOR_s {
    float timer;
    void *data;
};

struct NUGCUTLOCATORSYS_s {
    struct NUGCUTLOCATOR_s *locators;
    struct NUGCUTLOCATORTYPE_s *locator_types;
    u8 nlocators;
    u8 ntypes;
    char pad[2];
};

struct instNUGCUTLOCATORSYS_s {
    struct instNUGCUTLOCATOR_s *ilocators;
};

struct NUGCUTRIGID_s {
    struct numtx_s mtx;
    char *name;
    struct nuhspecial_s special;
    struct nuanimdata2_s *anim;
    struct NUSTATEANIM_s *visibiltyanim;
    struct NUGCUTLOCATOR_s *locators;
    u8 flags;
    u8 nlocators;
    u8 first_locator;
    char pad;
};

struct instNUGCUTRIGID_s {
    struct nuhspecial_s special;
    char visibleframeix;
    char pad[3];
};

struct NUGCUTRIGIDSYS_s {
    struct NUGCUTRIGID_s *rigids;
    u16 nrigids;
    char pad[2];
};

struct instNUGCUTRIGIDSYS_s {
    struct instNUGCUTRIGID_s *irigids;
};

struct NUGCUTCHAR_s {
    struct numtx_s mtx;
    char *name;
    struct nuanimdata2_s *char_anim;
    struct nuanimdata2_s *joint_anim;
    struct nuanimdata2_s *face_anim;
    void *character;
    struct NUGCUTLOCATOR_s *locators;
    float animrate;
    u8 flags;
    u8 animix;
    u8 nlocators;
    u8 type;
    u8 first_locator;
    u8 blendframes;
    char pad[2];
};

struct instNUGCUTCHAR_s {
    void *character;
    float blend;
    float mtxblend;
    float frame1;
    float frame2;
    char flags;
    u8 prev_animix;
    u8 current_animix;
    u8 blendto_animix;
};

struct NUGCUTCHARSYS_s {
    struct NUGCUTCHAR_s *chars;
    u16 nchars;
    char pad[2];
};

struct instNUGCUTCHARSYS_s {
    struct instNUGCUTCHAR_s *ichars;
};

struct NUGCUTSCENE_s {
    s32 version;
    s32 address_offset;
    float nframes;
    char *name_table;
    struct NUGCUTCAMSYS_s *cameras;
    struct NUGCUTRIGIDSYS_s *rigids;
    struct NUGCUTCHARSYS_s *chars;
    struct NUGCUTLOCATORSYS_s *locators;
    struct NUGCUTBBOX_s *bbox;
    struct NUGCUTTRIGGERSYS_s *triggersys;
};

struct instNUGCUTSCENE_s {
    struct instNUGCUTSCENE_s *next;
    struct instNUGCUTSCENE_s *prev;
    char name[16];
    struct numtx_s mtx;
    struct NUGCUTSCENE_s *cutscene;
    struct nuvec_s centre;
    float maxdistsqr;
    int played_first_frame : 1;
    unsigned int is_playing : 1;
    int looping : 1;
    int autostart : 1;
    int has_mtx : 1;
    int checkbboxclip : 1;
    int checkmaxdist : 1;
    int is_visible : 1;
    int is_disabled : 1;
    int been_updated_this_frame : 1;
    float cframe;
    float rate;
    struct instNUGCUTCAMSYS_s *icameras;
    struct instNUGCUTRIGIDSYS_s *irigids;
    struct instNUGCUTCHARSYS_s *ichars;
    struct instNUGCUTLOCATORSYS_s *ilocators;
    struct instNUGCUTTRIGGERSYS_s *itriggersys;
    struct instNUGCUTSCENE_s *next_to_play;
    void (*endfn)(void *);
};

struct NUGCUTLOCFNDATA_s {
    char *name;
    void (*fn)(struct instNUGCUTSCENE_s *, struct NUGCUTLOCATORSYS_s *,
               struct instNUGCUTLOCATOR_s *, struct NUGCUTLOCATOR_s *, float,
               struct numtx_s *);
};

#endif
