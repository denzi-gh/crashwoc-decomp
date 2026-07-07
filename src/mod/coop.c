#include "coop.h"

typedef signed char s8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

struct nuvec_s {
    f32 x;
    f32 y;
    f32 z;
};

struct anim_s {
    f32 anim_time;
    f32 blend_src_time;
    f32 blend_dst_time;
    short action;
    short oldaction;
    short newaction;
    short blend_src_action;
    short blend_dst_action;
    short blend_frame;
    short blend_frames;
    char blend;
    u8 flags;
};

struct RPos_s {
    char iRAIL;
    char vertical;
    short iALONG;
    short i1;
    short i2;
    f32 fALONG;
    f32 fACROSS;
    u16 angle;
    u16 cam_angle;
    u8 mode;
    char pad1;
    char pad2;
    char pad3;
    struct nuvec_s pos;
};

struct CharacterModel {
    void *hobj;
    void *anmdata[118];
    void *animlist[118];
    void *fanmdata[118];
    void *fanimlist[118];
    int shadhdr;
    short character;
    char pad1;
    char pad2;
    void *pLOCATOR[16];
};

union type_s_name {
    u8 chrs;
    u16 all;
};

struct obj_s {
    struct obj_s *parent;
    struct CharacterModel *model;
    void *mask;
    void *contact;
    void *pLOCATOR;
    struct anim_s anim;
    short character;
    short vehicle;
    u32 flags;
    u32 frame;
    u32 draw_frame;
    u32 vehicle_frame;
    struct RPos_s RPos;
    struct nuvec_s pos;
    struct nuvec_s mom;
    struct nuvec_s oldpos;
    struct nuvec_s startpos;
    struct nuvec_s vSN;
    struct nuvec_s vLN;
    struct nuvec_s vRN;
    f32 shadow;
    f32 layer_shadow;
    f32 roof_y;
    f32 clearance;
    f32 forward;
    f32 abs_forward;
    f32 side;
    f32 abs_side;
    f32 xyz_distance;
    f32 xz_distance;
    f32 radius;
    struct nuvec_s min;
    struct nuvec_s max;
    f32 scale;
    f32 SCALE;
    f32 RADIUS;
    f32 old_SCALE;
    f32 objbot;
    f32 objtop;
    f32 bot;
    f32 top;
    f32 oldobjbot;
    f32 oldobjtop;
    f32 die_time;
    f32 die_duration;
    f32 reflect_y;
    f32 idle_gametime;
    f32 pad_speed;
    f32 pad_dx;
    f32 pad_dz;
    char i;
    char dead;
    u16 pad_angle;
    u16 attack;
    u16 vulnerable;
    short die_action;
    char old_ground;
    char finished;
    u16 xrot;
    u16 yrot;
    u16 zrot;
    u16 surface_xrot;
    u16 surface_zrot;
    u16 layer_xrot;
    u16 layer_zrot;
    u16 roof_xrot;
    u16 roof_zrot;
    short target_xrot;
    short target_yrot;
    short dyrot;
    union type_s_name gndflags;
    u16 hdg;
    u16 thdg;
    char ground;
    char surface_type;
    char layer_type;
    char roof_type;
    char invisible;
    u8 submerged;
    char transporting;
    char got_shadow;
    u8 boing;
    u8 contact_type;
    char die_model[2];
    u8 invincible;
    char pos_adjusted;
    char wade;
    char dangle;
    char ddsand;
    char ddsnow;
    char ddwater;
    char ddr;
    char ddg;
    char ddb;
    char last_ground;
    char direction;
    char kill_contact;
    u8 touch;
};

struct rumble_s {
    u8 buzz;
    u8 power;
    u8 frame;
    u8 frames;
};

/*
 * Mirrors struct creature_s (src/gamecode/creature.h) field-for-field from
 * `ai` onward. Fields the co-op mod does not touch are kept as opaque byte
 * blobs sized to match the real nested types (AI_s, numtx_s, Nearest_Light_s)
 * so the overall layout matches the retail binary without pulling in their
 * full type graphs. Offsets were confirmed by compiling an offsetof() probe
 * against the real creature.h with the project's ProDG 3.5 toolchain.
 */
struct creature_s {
    char used;
    char on;
    char off_wait;
    char i_aitab;
    struct obj_s obj;
    char ai_opaque[0x98];
    void *Buggy;
    void *cmdtable;
    void *cmdcurr;
    void *OnFootMoveInfo;
    char m_opaque[0x40];
    char mtxLOCATOR_opaque[0x800];
    struct nuvec_s momLOCATOR[16][2];
    char lights_opaque[0xB0];
    struct rumble_s rumble;
    f32 idle_time;
    f32 idle_wait;
    short idle_action;
    short old_idle_action;
    char idle_mode;
    char idle_repeat;
    char jump;
    char jump_type;
    char jump_subtype;
    char ok_slam;
    char slam;
    char spin;
    char crawl;
    char crawl_lock;
    char tiptoe;
    char sprint;
    u8 somersault;
    u8 land;
    char pad_type;
    char jump_hack;
    u8 jump_hold;
    u8 allow_jump;
    short jump_frames;
    short jump_frame;
    short slam_wait;
    short spin_frames;
    short spin_frame;
    short spin_wait;
    short slide;
    short crouch_pos;
    u16 slam_frame;
    short fire_action;
    u8 fire;
    u8 tap;
    char target;
    char target_wait;
    char fire_lock;
    char idle_sigh;
    u8 hit_type;
    u8 freeze;
    char anim_processed;
    char pad1;
};
COOP_STATIC_ASSERT(sizeof(struct creature_s) == 0xCE4u);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, rumble) == 0xCA4u);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, ok_slam) == 0xCB9u);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, slam) == 0xCBAu);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, spin) == 0xCBBu);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, jump_hold) == 0xCC4u);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, slam_wait) == 0xCCAu);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, spin_frames) == 0xCCCu);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, spin_frame) == 0xCCEu);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, spin_wait) == 0xCD0u);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, slam_frame) == 0xCD6u);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, fire_action) == 0xCD8u);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, fire) == 0xCDAu);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, target) == 0xCDCu);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, target_wait) == 0xCDDu);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, hit_type) == 0xCE0u);
COOP_STATIC_ASSERT(COOP_OFFSETOF(struct creature_s, freeze) == 0xCE1u);

struct hub_s {
    u8 flags;
    u8 crystals;
    char pad1;
    char pad2;
};

struct time_s {
    char name[4];
    u32 itime;
};

struct level_s {
    u16 flags;
    char pad1;
    char pad2;
    struct time_s time[3];
};

struct game_s {
    char name[9];
    u8 vibration;
    u8 surround;
    u8 sfx_volume;
    u8 music_volume;
    char screen_x;
    char screen_y;
    u8 language;
    struct hub_s hub[6];
    struct level_s level[35];
    u8 lives;
    u8 wumpas;
    u8 mask;
    u8 percent;
    u8 crystals;
    u8 relics;
    u8 crate_gems;
    u8 bonus_gems;
    u8 gems;
    u8 gembits;
    u8 powerbits;
    u8 empty;
    u32 cutbits;
    u8 year;
    u8 month;
    u8 day;
    u8 hours;
    u8 mins;
    u8 pad_[3];
};

/*
 * Minimal view of nupad_s (full struct is 0xF4 bytes, see
 * dump_alphaNGCport_DWARF.txt). We only need the digital button word, so the
 * preceding fields are padded out to keep paddata at its exact offset (0xCC).
 */
struct nupad_s {
    u8 pad_a[0xCC];
    u32 paddata; /* 0xCC: digital button bitfield */
};

extern struct nupad_s *Pad[2];

void Text3D(char *txt, f32 x, f32 y, f32 z, f32 scalex, f32 scaley, f32 scalez,
            s32 align, s32 colour);
void NuShaderSetBypassShaders(s32 bypass);

extern s32 Level;
extern s32 GameMode;
extern s32 Hub;
extern s32 Bonus;
extern s32 Death;
extern s32 GemPath;
extern s32 level_part_2;
extern s32 VEHICLECONTROL;
extern s32 cut_on;
extern struct game_s Game;
extern struct creature_s *player;
extern struct CharacterModel CModel[49];
extern signed char CRemap[191];

void UpdatePlayerStats(struct creature_s *plr);
void DrawCreatures(struct creature_s *c, s32 count, s32 render, s32 shadow);
void CalculateGamePercentage(struct game_s *game);
s32 Draw3DCharacter(struct nuvec_s *pos, u16 xrot, u16 yrot, u16 zrot,
                    struct CharacterModel *model, s32 action, f32 scale,
                    f32 anim_time, s32 rot);

struct CoopMailbox COOP_DATA gCoopMailbox = {
    COOP_PROTOCOL_MAGIC,
    COOP_PROTOCOL_ABI_VERSION,
    COOPMAILBOX_SIZE,
    COOP_GCBE7D_BUILD_ID,
    0,
    0,
    0,
    0,
};

static struct CoopProgress COOP_DATA sLastPublishedProgress;
static struct creature_s COOP_DATA sRemoteCreature;
static u32 COOP_DATA sLocalProgressRevision = 1;
static f32 COOP_DATA sCoopZero = 0.0f;
static f32 COOP_DATA sCoopOne = 1.0f;
static f32 COOP_DATA sCoopFloatLimit = 1000000.0f;

/*
 * Kept in .coop_data (not the default .rodata) so the string bytes and the
 * scale constant do not grow the base .rodata past its fixed VMA ceiling. Inline
 * literals here would emit into .rodata/.data of the main image and shift every
 * following section, which is exactly the boot crash this feature originally hit.
 */
static f32 COOP_DATA sCoopPanelTextScale = 0.8f;
static char COOP_DATA sCoopPanelText[] = "#wLECK EIER MARCEL";

#define COOP_DEBUG_MAGIC 0x43444247u
#define COOP_DEBUG_DRAWN 12u
#define COOP_DEBUG_STALE 1u
#define COOP_DEBUG_NO_INBOUND 2u
#define COOP_DEBUG_INACTIVE 3u
#define COOP_DEBUG_UNUSED 4u
#define COOP_DEBUG_DEAD 5u
#define COOP_DEBUG_INVISIBLE 6u
#define COOP_DEBUG_LOCAL_STATE 7u
#define COOP_DEBUG_LOCATION 8u
#define COOP_DEBUG_BAD_FLOAT 9u
#define COOP_DEBUG_NO_MODEL 10u
#define COOP_DEBUG_BAD_ACTION 11u

/*
 * Generous upper bound on spin_frames: real Crash/Coco spin animations are
 * derived from ModelAnimDuration() * 60fps (see creature.c) and run well
 * under a second. 600 frames (10s at 60Hz) rejects corrupted/garbage remote
 * values while comfortably covering any real spin animation length.
 */
#define COOP_SPIN_FRAMES_MAX 600u

static void COOP_TEXT CoopCopyProgress(struct CoopProgress *dst)
{
    s32 i;

    dst->revision = sLocalProgressRevision;
    for (i = 0; i < 35; i++) {
        dst->level_flags[i] = Game.level[i].flags;
    }
    for (i = 0; i < 6; i++) {
        dst->hub_flags[i] = Game.hub[i].flags;
        dst->hub_crystals[i] = Game.hub[i].crystals;
    }
    dst->powerbits = Game.powerbits;
    dst->gembits = Game.gembits;
    dst->reserved8[0] = 0;
    dst->reserved8[1] = 0;
    if ((gCoopMailbox.capabilities & COOP_CAP_CUTBITS) != 0) {
        dst->cutbits = Game.cutbits;
    } else {
        dst->cutbits = 0;
    }
}

static s32 COOP_TEXT CoopProgressDiffers(struct CoopProgress *a, struct CoopProgress *b)
{
    s32 i;

    for (i = 0; i < 35; i++) {
        if (a->level_flags[i] != b->level_flags[i]) {
            return 1;
        }
    }
    for (i = 0; i < 6; i++) {
        if ((a->hub_flags[i] != b->hub_flags[i]) ||
            (a->hub_crystals[i] != b->hub_crystals[i])) {
            return 1;
        }
    }
    return (a->powerbits != b->powerbits) ||
           (a->gembits != b->gembits) ||
           (a->cutbits != b->cutbits);
}

static void COOP_TEXT CoopPublishLocation(struct CoopLocation *location, struct creature_s *plr)
{
    location->level = Level;
    location->game_mode = GameMode;
    location->hub = Hub;
    location->level_part_2 = level_part_2;
    location->bonus = Bonus;
    location->death = Death;
    location->gem_path = GemPath;
    location->vehicle_class = (plr != 0) ? plr->obj.vehicle : -1;
}

static void COOP_TEXT CoopPublishAvatar(struct CoopAvatar *avatar, struct creature_s *plr)
{
    u32 flags;
    u32 move_flags;

    flags = 0;
    move_flags = 0;
    if ((plr != 0) && (plr->used != 0)) {
        flags |= COOP_AVATAR_USED;
        if (plr->obj.dead != 0) {
            flags |= COOP_AVATAR_DEAD;
        }
        if (plr->obj.invisible != 0) {
            flags |= COOP_AVATAR_INVISIBLE;
        }
        avatar->frame = plr->obj.frame;
        avatar->pos_x = plr->obj.pos.x;
        avatar->pos_y = plr->obj.pos.y;
        avatar->pos_z = plr->obj.pos.z;
        avatar->xrot = plr->obj.xrot;
        avatar->yrot = plr->obj.yrot;
        avatar->zrot = plr->obj.zrot;
        avatar->heading = plr->obj.hdg;
        avatar->character = plr->obj.character;
        avatar->action = plr->obj.anim.action;
        avatar->anim_time = plr->obj.anim.anim_time;
        avatar->vehicle = plr->obj.vehicle;
        /* Spin rendering uses the dedicated spin fields, not anim.action. */
        if (plr->spin != 0) {
            move_flags |= COOP_MOVE_SPIN;
        }
        avatar->spin_frame = (u16)plr->spin_frame;
        avatar->spin_frames = (u16)plr->spin_frames;
    } else {
        avatar->frame = 0;
        avatar->pos_x = sCoopZero;
        avatar->pos_y = sCoopZero;
        avatar->pos_z = sCoopZero;
        avatar->xrot = 0;
        avatar->yrot = 0;
        avatar->zrot = 0;
        avatar->heading = 0;
        avatar->character = 0;
        avatar->action = -1;
        avatar->anim_time = sCoopOne;
        avatar->vehicle = -1;
        avatar->spin_frame = 0;
        avatar->spin_frames = 0;
    }
    avatar->flags = flags;
    avatar->move_flags = move_flags;
}

static void COOP_TEXT CoopWriteLocalSnapshot(struct creature_s *plr)
{
    struct CoopSnapshot *snapshot;
    struct CoopProgress progress;
    u32 seq;

    snapshot = &gCoopMailbox.local_snapshot;
    CoopCopyProgress(&progress);
    if (CoopProgressDiffers(&progress, &sLastPublishedProgress) != 0) {
        sLocalProgressRevision++;
        progress.revision = sLocalProgressRevision;
        sLastPublishedProgress = progress;
    }

    seq = gCoopMailbox.local_seq;
    if ((seq & 1u) != 0) {
        seq++;
    }
    gCoopMailbox.local_seq = seq + 1u;
    snapshot->status_flags = COOP_STATUS_CONNECTED | COOP_STATUS_ACTIVE;
    CoopPublishLocation(&snapshot->location, plr);
    CoopPublishAvatar(&snapshot->avatar, plr);
    snapshot->progress = progress;
    gCoopMailbox.local_seq = seq + 2u;
}

static s32 COOP_TEXT CoopReadInboundSnapshot(struct CoopSnapshot *out)
{
    u32 seq_a;
    u32 seq_b;

    seq_a = gCoopMailbox.inbound_seq;
    if ((seq_a == 0) || ((seq_a & 1u) != 0)) {
        return 0;
    }
    *out = gCoopMailbox.inbound_snapshot;
    seq_b = gCoopMailbox.inbound_seq;
    if ((seq_a != seq_b) || ((seq_b & 1u) != 0)) {
        return 0;
    }
    return 1;
}

static void COOP_TEXT CoopApplyInboundProgress(struct CoopProgress *progress)
{
    s32 changed;
    s32 i;
    u16 level_flags;
    u8 hub_flags;
    u8 crystals;

    /*
     * The merge below always runs, even when this revision was already seen.
     * Loading a lower-progress save slot leaves the mailbox's last-applied
     * revision unchanged while the local Game state regresses, so gating the
     * merge on revision > last_applied would skip restoring session progress
     * after such a load. The OR/max merge is naturally idempotent, so it is
     * safe to repeat every frame; only forward revision movement is latched.
     */
    if (progress->revision > gCoopMailbox.last_applied_progress_revision) {
        gCoopMailbox.last_applied_progress_revision = progress->revision;
    }

    changed = 0;
    for (i = 0; i < 35; i++) {
        level_flags = Game.level[i].flags | progress->level_flags[i];
        if (level_flags != Game.level[i].flags) {
            Game.level[i].flags = level_flags;
            changed = 1;
        }
    }
    for (i = 0; i < 6; i++) {
        hub_flags = Game.hub[i].flags | progress->hub_flags[i];
        if (hub_flags != Game.hub[i].flags) {
            Game.hub[i].flags = hub_flags;
            changed = 1;
        }
        crystals = Game.hub[i].crystals;
        if (progress->hub_crystals[i] > crystals) {
            Game.hub[i].crystals = progress->hub_crystals[i];
            changed = 1;
        }
    }
    if ((Game.powerbits | progress->powerbits) != Game.powerbits) {
        Game.powerbits |= progress->powerbits;
        changed = 1;
    }
    if ((Game.gembits | progress->gembits) != Game.gembits) {
        Game.gembits |= progress->gembits;
        changed = 1;
    }
    if ((gCoopMailbox.capabilities & COOP_CAP_CUTBITS) != 0) {
        if ((Game.cutbits | progress->cutbits) != Game.cutbits) {
            Game.cutbits |= progress->cutbits;
            changed = 1;
        }
    }
    if (changed != 0) {
        CalculateGamePercentage(&Game);
    }
}

void COOP_TEXT CoopFrameUpdate(struct creature_s *plr)
{
    struct CoopSnapshot inbound;

    gCoopMailbox.reserved[0] = COOP_DEBUG_MAGIC;
    if (gCoopMailbox.magic != COOP_PROTOCOL_MAGIC) {
        gCoopMailbox.magic = COOP_PROTOCOL_MAGIC;
        gCoopMailbox.abi_version = COOP_PROTOCOL_ABI_VERSION;
        gCoopMailbox.struct_size = COOPMAILBOX_SIZE;
        gCoopMailbox.build_id = COOP_GCBE7D_BUILD_ID;
    }
    gCoopMailbox.game_heartbeat++;
    CoopWriteLocalSnapshot(plr);
    if (CoopReadInboundSnapshot(&inbound) != 0) {
        if ((inbound.status_flags & COOP_STATUS_CONNECTED) != 0) {
            CoopApplyInboundProgress(&inbound.progress);
        }
    }
}

static s32 COOP_TEXT CoopFloatValid(f32 value)
{
    if (value != value) {
        return 0;
    }
    return (value > -sCoopFloatLimit) && (value < sCoopFloatLimit);
}

static s32 COOP_TEXT CoopSameLocation(struct CoopLocation *remote, struct creature_s *plr)
{
    s32 local_vehicle;

    local_vehicle = (plr != 0) ? plr->obj.vehicle : -1;
    return (remote->level == Level) &&
           (remote->game_mode == GameMode) &&
           (remote->hub == Hub) &&
           (remote->level_part_2 == level_part_2) &&
           (remote->bonus == Bonus) &&
           (remote->death == Death) &&
           (remote->gem_path == GemPath) &&
           (remote->vehicle_class == local_vehicle);
}

static struct CharacterModel * COOP_TEXT CoopResolveModel(struct CoopAvatar *avatar)
{
    s32 character;
    s32 remap;

    character = avatar->character;
    if ((character >= 0) && (character < 191)) {
        remap = CRemap[character];
        if ((remap >= 0) && (remap < 49)) {
            return &CModel[remap];
        }
    }
    if ((player != 0) && (player->obj.model != 0)) {
        return player->obj.model;
    }
    return 0;
}

static void COOP_TEXT CoopDebugRemote(u32 reason, struct CoopSnapshot *inbound, u32 same_location)
{
    gCoopMailbox.reserved[0] = COOP_DEBUG_MAGIC;
    gCoopMailbox.reserved[1]++;
    gCoopMailbox.reserved[2] = reason;
    if (reason == COOP_DEBUG_DRAWN) {
        gCoopMailbox.reserved[3]++;
    } else {
        gCoopMailbox.reserved[4]++;
    }
    gCoopMailbox.reserved[5] = (u32)Level;
    gCoopMailbox.reserved[6] = (inbound != 0) ? (u32)inbound->location.level : 0xFFFFFFFFu;
    gCoopMailbox.reserved[7] = same_location;
}

/*
 * Neutralizes local-only transient gameplay/animation state on a remote
 * creature that was just bulk-copied from the local player. Render context
 * (model, matrices, OnFootMoveInfo, character) is intentionally left as
 * copied so DrawCreatures() has a valid creature to render; only fields that
 * drive local gameplay behavior or one-shot local effects are cleared here.
 * The caller applies remote-specific state (position, rotation, spin, action)
 * afterward.
 */
static void COOP_TEXT CoopSanitizeRemoteCreature(struct creature_s *remote, struct creature_s *local)
{
    (void)local;
    remote->spin = 0;
    remote->spin_frame = 0;
    remote->spin_frames = 0;
    remote->spin_wait = 0;
    remote->slam = 0;
    remote->slam_wait = 0;
    remote->slam_frame = 0;
    remote->obj.dangle = 0;
    remote->freeze = 0;
    remote->target = 0;
    remote->target_wait = 0;
    remote->fire = 0;
    remote->fire_action = 0;
    remote->rumble.buzz = 0;
    remote->rumble.power = 0;
    remote->rumble.frame = 0;
    remote->rumble.frames = 0;
    remote->hit_type = 0;
    remote->jump_hold = 0;
}

static void COOP_TEXT CoopHideRemotePlayer(void)
{
    if ((player == 0) || (player->used == 0) || (player->obj.model == 0)) {
        sRemoteCreature.used = 0;
        sRemoteCreature.on = 0;
        sRemoteCreature.obj.flags = 0;
        sRemoteCreature.obj.model = 0;
        return;
    }

    sRemoteCreature = *player;
    CoopSanitizeRemoteCreature(&sRemoteCreature, player);
    sRemoteCreature.obj.model = player->obj.model;
    sRemoteCreature.obj.invisible = 1;
    sRemoteCreature.obj.SCALE = sCoopZero;
    sRemoteCreature.obj.scale = sCoopZero;
    sRemoteCreature.obj.RADIUS = sCoopZero;
    sRemoteCreature.obj.radius = sCoopZero;
    sRemoteCreature.obj.anim.action = -1;
    sRemoteCreature.obj.anim.oldaction = -1;
    sRemoteCreature.obj.anim.newaction = -1;
    sRemoteCreature.obj.anim.blend = 0;
    sRemoteCreature.obj.anim.anim_time = sCoopOne;
    DrawCreatures(&sRemoteCreature, 1, 1, 0);
}

void COOP_TEXT CoopDrawRemotePlayer(void)
{
    struct CoopSnapshot inbound;
    struct CharacterModel *model;
    s32 action;
    s32 same_location;

    if ((gCoopMailbox.bridge_heartbeat == 0) ||
        ((gCoopMailbox.game_heartbeat - gCoopMailbox.bridge_heartbeat) > 180u)) {
        CoopDebugRemote(COOP_DEBUG_STALE, 0, 0);
        CoopHideRemotePlayer();
        return;
    }
    if (CoopReadInboundSnapshot(&inbound) == 0) {
        CoopDebugRemote(COOP_DEBUG_NO_INBOUND, 0, 0);
        CoopHideRemotePlayer();
        return;
    }
    if ((inbound.status_flags & (COOP_STATUS_CONNECTED | COOP_STATUS_ACTIVE)) !=
        (COOP_STATUS_CONNECTED | COOP_STATUS_ACTIVE)) {
        CoopDebugRemote(COOP_DEBUG_INACTIVE, &inbound, 0);
        CoopHideRemotePlayer();
        return;
    }
    if ((inbound.avatar.flags & COOP_AVATAR_USED) == 0) {
        CoopDebugRemote(COOP_DEBUG_UNUSED, &inbound, 0);
        CoopHideRemotePlayer();
        return;
    }
    if ((inbound.avatar.flags & COOP_AVATAR_DEAD) != 0) {
        CoopDebugRemote(COOP_DEBUG_DEAD, &inbound, 0);
        CoopHideRemotePlayer();
        return;
    }
    if ((inbound.avatar.flags & COOP_AVATAR_INVISIBLE) != 0) {
        CoopDebugRemote(COOP_DEBUG_INVISIBLE, &inbound, 0);
        CoopHideRemotePlayer();
        return;
    }
    if ((player == 0) || (player->used == 0) || (cut_on != 0) || (VEHICLECONTROL != 0)) {
        CoopDebugRemote(COOP_DEBUG_LOCAL_STATE, &inbound, 0);
        CoopHideRemotePlayer();
        return;
    }
    same_location = CoopSameLocation(&inbound.location, player);
    if ((inbound.avatar.vehicle != -1) || (same_location == 0)) {
        CoopDebugRemote(COOP_DEBUG_LOCATION, &inbound, (u32)same_location);
        CoopHideRemotePlayer();
        return;
    }
    if ((CoopFloatValid(inbound.avatar.pos_x) == 0) ||
        (CoopFloatValid(inbound.avatar.pos_y) == 0) ||
        (CoopFloatValid(inbound.avatar.pos_z) == 0) ||
        (CoopFloatValid(inbound.avatar.anim_time) == 0)) {
        CoopDebugRemote(COOP_DEBUG_BAD_FLOAT, &inbound, 1);
        CoopHideRemotePlayer();
        return;
    }

    model = CoopResolveModel(&inbound.avatar);
    if (model == 0) {
        CoopDebugRemote(COOP_DEBUG_NO_MODEL, &inbound, 1);
        CoopHideRemotePlayer();
        return;
    }
    action = inbound.avatar.action;
    if ((action < -1) || (action >= 118)) {
        CoopDebugRemote(COOP_DEBUG_BAD_ACTION, &inbound, 1);
        CoopHideRemotePlayer();
        return;
    }
    if ((action >= 0) && (model->anmdata[action] == 0)) {
        action = -1;
    }

    sRemoteCreature = *player;
    CoopSanitizeRemoteCreature(&sRemoteCreature, player);
    sRemoteCreature.obj.model = model;
    sRemoteCreature.obj.pos.x = inbound.avatar.pos_x;
    sRemoteCreature.obj.pos.y = inbound.avatar.pos_y;
    sRemoteCreature.obj.pos.z = inbound.avatar.pos_z;
    sRemoteCreature.obj.oldpos = sRemoteCreature.obj.pos;
    sRemoteCreature.obj.RPos.pos = sRemoteCreature.obj.pos;
    sRemoteCreature.obj.xrot = inbound.avatar.xrot;
    sRemoteCreature.obj.yrot = inbound.avatar.yrot;
    sRemoteCreature.obj.zrot = inbound.avatar.zrot;
    sRemoteCreature.obj.hdg = inbound.avatar.heading;
    sRemoteCreature.obj.thdg = inbound.avatar.heading;
    sRemoteCreature.obj.character = inbound.avatar.character;
    sRemoteCreature.obj.vehicle = -1;
    sRemoteCreature.obj.dead = 0;
    sRemoteCreature.obj.invisible = 0;
    sRemoteCreature.obj.invincible = 0;
    sRemoteCreature.obj.finished = 0;
    /*
     * DrawCreatures() takes a dedicated spin-rendering branch driven by
     * c->spin/spin_frame/spin_frames rather than anim.action, so the normal
     * action fields are set unconditionally here (also covering the frame
     * right after a spin ends, when the remote snapshot's action already
     * reflects run/idle/jump again) and spin is applied on top only when the
     * remote snapshot reports it and the values pass validation.
     */
    sRemoteCreature.obj.anim.action = action;
    sRemoteCreature.obj.anim.oldaction = action;
    sRemoteCreature.obj.anim.newaction = action;
    sRemoteCreature.obj.anim.blend = 0;
    sRemoteCreature.obj.anim.anim_time = inbound.avatar.anim_time;
    if ((inbound.avatar.move_flags & COOP_MOVE_SPIN) != 0) {
        u32 spin_frames = inbound.avatar.spin_frames;
        u32 spin_frame = inbound.avatar.spin_frame;
        if ((spin_frames > 0) && (spin_frames <= COOP_SPIN_FRAMES_MAX) &&
            (spin_frame <= spin_frames)) {
            sRemoteCreature.spin = 1;
            sRemoteCreature.spin_frame = (short)spin_frame;
            sRemoteCreature.spin_frames = (short)spin_frames;
        }
    }
    CoopDebugRemote(COOP_DEBUG_DRAWN, &inbound, 1);
    DrawCreatures(&sRemoteCreature, 1, 1, 0);
}

void COOP_TEXT CoopUpdatePlayerStatsWrapper(struct creature_s *plr)
{
    UpdatePlayerStats(plr);
    CoopFrameUpdate(plr);
}

void COOP_TEXT CoopDrawCreaturesWrapper(struct creature_s *c, int count, int render, int shadow)
{
    DrawCreatures(c, count, render, shadow);
    CoopDrawRemotePlayer();
}

/*
 * Hooked over the `bl NuShaderSetBypassShaders` at DrawPanel's exit
 * (0x800601D0), the point where every DrawPanel path converges. Draws a piece
 * of custom text while A + L are held on pad 0.
 *
 * Button bits come from paddata (see dummyfunc.c pad decode): 0x40 = A,
 * 0x04 = L trigger. Both are plain digital bits used by the shipped game
 * (e.g. vehicle.c reads 0x04), so they are reliable across the GC port -
 * unlike the analogue l2_alg field, which the pad decoder leaves at 0 except
 * as a stick-derived side effect.
 *
 * We run the original NuShaderSetBypassShaders(bypass) FIRST: DrawPanel enables
 * bypass shaders while drawing (panel.c NuShaderSetBypassShaders(1)) and only
 * clears them here. Drawing before this reset renders the text with bypass
 * still on, so it never appears - draw AFTER the reset instead.
 *
 * The wrapper lives in .coop_text so it does NOT grow .text past its fixed
 * .rodata ceiling at 0x80104920 (which is what crashed the earlier in-panel.c
 * version on boot).
 */
#define COOP_TEXT_PAD_A 0x40u  /* paddata: A button */
#define COOP_TEXT_PAD_L 0x04u  /* paddata: L trigger (digital) */
#define COOP_TEXT_COMBO (COOP_TEXT_PAD_A | COOP_TEXT_PAD_L)

void COOP_TEXT CoopDrawPanelTextWrapper(s32 bypass)
{
    struct nupad_s *pad = Pad[0];

    NuShaderSetBypassShaders(bypass);

    if (pad != 0 && (pad->paddata & COOP_TEXT_COMBO) == COOP_TEXT_COMBO) {
        Text3D(sCoopPanelText, sCoopZero, sCoopZero, sCoopOne,
               sCoopPanelTextScale, sCoopPanelTextScale, sCoopPanelTextScale, 1, 0);
    }
}
