#ifndef EDOBJ_H
#define EDOBJ_H

#include "../nu.h"
#include "edfile.h"

struct object_path_s {
    s32 objid;
    f32 speed;
    s32 oscillate;
    s32 repeat;
    f32 pause;
    s32 usedway;
    s32 usedtime;
    s32 usedpart;
    s32 usedsound;
    s32 start_offset;
    s32 terrplatid;
    struct nuvec_s waypoint[8];
    f32 waypoint_speed[8];
    struct nuvec_s waypoint_rot[8];
    s32 waypoint_time[8];
    s32 trigger_type;
    s32 trigger_id;
    f32 trigger_var;
    f32 trigger_wait;
    char particle_name[16][8];
    s32 particle_type[8];
    s32 particle_rate[8];
    s32 particle_switch[8];
    struct nuvec_s particle_offset[8];
    s16 particle_emitrotz[8];
    s16 particle_emitroty[8];
    f32 sound_last_time;
    char sound_name[16][8];
    s32 sound_id[8];
    s32 sound_type[8];
    f32 sound_time[8];
    struct nuvec_s sound_offset[8];
    f32 playergrav;
    f32 tension;
    f32 damping;
};

extern struct nuinstance_s ObjectInstance[64];
extern struct nuinstanim_s ObjectInstAnim[64];
extern struct object_path_s ObjectPath[64];
extern struct nugscn_s *edobj_base_scene;
extern struct nuvec_s edobj_cam_pos;
extern s32 edobj_next_instance;
extern s32 edobj_nearest;
extern s32 edobj_waypoint_mode;
extern s32 edobj_copy_mode;
extern s32 edobj_particle_mode;
extern s32 edobj_sound_mode;

struct nuvec_s *edmainQueryLocVec(void);
void edobjConvertPathToAnim(s32 sel);
s32 edobjLookupInstance(char *name);

#endif // !EDOBJ_H
