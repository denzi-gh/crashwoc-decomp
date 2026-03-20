#include "main.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct gdeb_s {
    s32 i;
    char* name;
    u64 levbits;
};

struct tersurface_s {
    f32 friction;
    s16 gdeb;
    u16 flags;
};

struct plritem_s {
    s16 count;
    s16 draw;
    s8 frame;
    s8 wait;
    u8 delay;
    u8 item;
};

struct plrbonus_s {
    s16 count;
    s16 draw;
    s8 frame;
    s8 wait;
    u8 delay;
    u8 item;
};

struct projectile_s {
    struct obj_s obj;
    struct nuvec_s srcpos;
    struct nuvec_s dstpos;
    f32 time;
    f32 duration;
    s8 type;
    s8 active;
    s16 i_objtab;
    s8 kill;
    u8 owner_safety;
    char pad1;
    char pad2;
};

struct objtab_s {
    struct nuhspecial_s obj;
    char pad[24];
};

extern struct obj_s *pObj[64];
extern s32 GAMEOBJECTCOUNT;
extern s32 temp_cuboid_side;
extern f32 temp_cuboid_bounce_angle;
extern u16 temp_yrot;
extern s32 VEHICLECONTROL;
extern s32 TimeTrial;
extern s32 level_part_2;
extern struct wumpa_s Wumpa[320];
extern struct winfo_s WInfo[8];
extern struct gdeb_s GDeb[170];
extern f32 in_s_friction;
extern f32 in_speed;
extern f32 in_f_friction;
extern u16 temp_xrot;
extern u16 temp_zrot;
extern struct tersurface_s TerSurface[];
extern f32 SAFEY;
extern struct nuvec_s v010;
extern struct nuvec_s ShadNorm;
extern u16 plr_conveyor_yrot;
extern struct nuvec_s plr_conveyor_mom;
extern f32 EShadY;
extern struct tersurface_s TerLayer[];
extern struct nuvec_s EShadNorm;
extern f32 ShadRoofY;
extern struct nuvec_s ShadRoofNorm;
extern u16 temp_roof_xrot;
extern u16 temp_roof_zrot;
extern struct plritem_s plr_crystal;
extern struct plritem_s plr_crategem;
extern struct plrbonus_s plr_bonusgem;
extern u16 plr_items;
extern s32 new_power;
extern s32 new_mode;
extern s32 new_level;
extern s32 boss_dead;
extern f32 temp_ratio;
extern s32 temp_face;
extern f32 vtog_duration;
extern f32 vtog_time;
extern s32 vtog_blend;
extern struct projectile_s Projectile[16];
extern f32 YTOL;

void AddPanelDebris(float x, float y, int type, float scale, int count);
void LoseMask(struct obj_s *obj);
s32 FurtherALONG(s32 iRAIL0, s32 iALONG0, float fALONG0, s32 iRAIL1, s32 iALONG1, float fALONG1);
void ShadowDir(struct nuvec_s *dir);
s32 RayIntersectCuboid(struct nuvec_s *p0, struct nuvec_s *p1, struct nuvec_s *min, struct nuvec_s *max);
void AddScreenWumpa(float x, float y, float z, s32 count);
extern struct cammtx_s *pCam;
extern f32 NuTrigTable[];
extern struct objtab_s ObjTab[];
s32 NuRndrGScnObj(struct nugobj_s *gobj, struct numtx_s *wm);
extern s32 GameMode;
extern s32 temp_xzmomset;
extern s32 gamesfx_effect_volume;
extern s32 InvincibilityCHEAT;
extern s32 Level;
extern s32 Hub;
extern u64 LBIT;
extern f32 plr_invisibility_time;
extern s32 bonus_crates_destroyed;
extern s32 old_bonus_crates;
float GameObjectRadius(struct obj_s *obj);
void ObjectToAtlas(struct obj_s *obj, struct creature_s *c);
s32 CylinderCuboidOverlapXZ(struct nuvec_s *pos, f32 radius, struct obj_s *cub, struct nuvec_s *opp_pos);
void BlendGameCamera(struct cammtx_s *cam, f32 time);
void AddMechanicalDebris(struct nuvec_s *pos, s32 vehicle);
void AddExtraLife(struct nuvec_s *pos, s32 pdeb);

/* TODO
	CrateCollisions
	KillGameObject
	AddProjectile
	UpdateProjectiles
	DrawProjectiles
*/

void PlayerCreatureCollisions(struct obj_s *obj) {
    struct nuvec_s opp_pos;
    struct nuvec_s pos;
    s32 i;
    s32 lose_mask;
    s32 side;
    s32 side2;
    s32 cuboid;
    s32 overlap;
    struct obj_s *opp;
    s16 mask_active;
    u16 attack;
    f32 dx, dz, d;
    s32 yrot;
    s32 set;
    s32 bounce;
    s32 miss;

    if (level_part_2 != 0) return;
    temp_xzmomset = 0;
    if (obj->dead != 0) return;

    mask_active = 0;
    if (obj->mask != NULL) {
        s16 ma = obj->mask->active;
        if (ma != 0 && (LDATA->flags & 0xe00) == 0) {
            mask_active = ma;
        }
    }

    lose_mask = 0;
    for (i = 0; i < GAMEOBJECTCOUNT; i++) {
        opp = pObj[i];
        if (opp == NULL) continue;

        opp->touch = 0;
        opp->kill_contact = 0;
        opp->contact = NULL;
        if (opp == obj) continue;
        if (opp->dead != 0) continue;
        if (opp->invisible != 0) continue;
        if (opp->draw_frame == 0) continue;
        if ((opp->flags & 4) == 0) continue;
        if ((opp->flags & 0x1000) != 0) continue;

        NewTopBot(obj);

        if ((opp->flags & 0x2000) != 0) {
            if (opp->draw_frame == 0) continue;
            if (opp->pLOCATOR == NULL || *(s32 *)((char *)opp->model + 0x76c) == 0) {
                opp_pos = opp->pos;
            } else {
                opp_pos.x = opp->pLOCATOR->_30;
                opp_pos.y = opp->pLOCATOR->_31;
                opp_pos.z = opp->pLOCATOR->_32;
            }
        } else {
            opp_pos = opp->pos;
        }

        if ((opp->flags & 0x8000) != 0) {
            overlap = CylinderCuboidOverlapXZ(&obj->pos, obj->RADIUS, opp, &opp_pos);
            cuboid = 1;
            side = temp_cuboid_side;
        } else {
            dz = obj->pos.z - opp_pos.z;
            dx = obj->pos.x - opp_pos.x;
            d = dx * dx + dz * dz;
            overlap = 1;
            if (d > (obj->RADIUS + opp->RADIUS) * (obj->RADIUS + opp->RADIUS)) {
                overlap = 0;
            }
            cuboid = 0;
            side = 8;
        }

        opp->objbot = opp->bot * opp->SCALE + opp_pos.y;
        opp->objtop = opp->top * opp->SCALE + opp_pos.y;

        if (overlap == 0) {
            /* No XZ overlap - check attack-based kills */
            if ((opp->attack & 2) == 0) {
                attack = obj->attack;
                if ((attack & 2) != 0 && (opp->vulnerable & 2) != 0) {
                    goto kill_nonov;
                }
                if (obj->character == 1 && (attack & 0x10) != 0 && (opp->vulnerable & 0x10) != 0) {
                    goto kill_nonov;
                }
                if (obj->mom.y < 0.0f && (attack & 0xc) != 0 && (opp->vulnerable & 0xc) != 0) {
                    goto kill_nonov;
                }
            }
            continue;
        kill_nonov:
            if (obj->objtop <= opp->objbot) continue;
            if (obj->objbot > opp->objtop) continue;

            d = GameObjectRadius(obj);
            if (cuboid != 0) {
                overlap = CylinderCuboidOverlapXZ(&obj->pos, d, opp, &opp->pos);
                if (overlap == 0) continue;
                attack = obj->attack;
                {
                    s32 dead_val = 4;
                    if ((attack & 2) != 0) dead_val = 1;
                    opp->dead = dead_val;
                }
                side = temp_cuboid_side;
            } else {
                if (d > (d + opp->RADIUS) * (d + opp->RADIUS)) continue;
                attack = obj->attack;
                {
                    s32 dead_val = 4;
                    if ((attack & 2) != 0) dead_val = 1;
                    opp->dead = dead_val;
                }
                side = 8;
            }

            {
                s32 touch_val = 2;
                if ((obj->attack & 0xc) == 0) touch_val = side;
                opp->touch = touch_val;
            }

            if (opp->dead != 0) {
                pos.x = (obj->pos.x + opp_pos.x) * 0.5f;
                pos.z = (obj->pos.z + opp_pos.z) * 0.5f;
                if ((obj->attack & 0xc) != 0) {
                    pos.y = obj->bot * obj->SCALE + obj->pos.y;
                } else {
                    pos.y = ((opp->objbot + opp->objtop) * 0.5f
                        + (obj->bot + obj->top) * obj->SCALE * 0.5f + obj->pos.y) * 0.5f;
                }
                AddGameDebris(0x82, &pos);

                if ((obj->attack & 2) != 0) {
                    if (opp->oldobjtop - YTOL <= obj->oldobjbot
                        || obj->oldobjtop <= opp->oldobjbot + YTOL) {
                        obj->mom.y = 0.0f;
                    } else {
                        obj->mom.x *= 0.25f;
                        obj->mom.z *= 0.25f;
                    }
                }
                goto post_contact;
            }
        } else {
            /* XZ overlap exists - check vertical */
            miss = 0;
            if (obj->objtop < opp->objbot || obj->objbot > opp->objtop) {
                miss = 1;
                goto check_above;
            }

            /* Full 3D overlap */
            if (VEHICLECONTROL == 2 || (VEHICLECONTROL == 1 && obj->vehicle == 0x20)) {
                if ((obj->attack & opp->vulnerable & 0x82) != 0) {
                    opp->dead = 0x15;
                } else if (mask_active != 0 && mask_active <= 2) {
                    lose_mask = 1;
                    opp->dead = 0x15;
                } else {
                    obj->dead = 1;
                }
                NuVecScale(0.25f, &obj->mom, &obj->mom);
                goto contact_assign;
            }

            /* Check old positions to determine collision direction */
            if (obj->oldobjbot >= opp->oldobjtop - YTOL) {
                /* TOP collision: player on top of opponent */
                side = 2;
                attack = obj->attack;
                bounce = 0;
                set = 0;
                if ((attack & opp->vulnerable & 0x8e) != 0) {
                    {
                        s32 dead_val = 1;
                        if ((attack & 0x8c) != 0) dead_val = 4;
                        opp->dead = dead_val;
                    }
                    if ((obj->attack & 2) != 0) {
                        obj->mom.y = 0.0f;
                    }
                } else if ((attack & opp->vulnerable & 0x20) != 0) {
                    opp->dead = 4;
                    bounce = 1;
                } else {
                    if ((attack & 2) != 0 && (opp->attack & 2) != 0) {
                        bounce = 1;
                    } else {
                        if (opp->attack == 0 && (opp->flags & 0x100) == 0) {
                            set = 1;
                        } else if (mask_active != 0 && mask_active <= 2) {
                            opp->dead = 4;
                            lose_mask = 1;
                        } else {
                            obj->dead = 1;
                            set = 1;
                            if ((opp->attack & 2) != 0) {
                                bounce = 1;
                            }
                        }
                    }
                }

                if (bounce == 0) {
                    if ((opp->flags & 0x40) != 0) {
                        bounce = 1;
                    }
                    if (bounce == 0) {
                        if ((opp->flags & 0x80) != 0) {
                            bounce = 2;
                        }
                        if (bounce == 0 && set == 0) goto top_touch;
                    }
                }

                obj->ground = 2;
                obj->mom.y = 0.0f;
                obj->pos.y = opp->objtop - obj->bot * obj->SCALE;
                obj->surface_type = 0xf;
                obj->shadow = opp->objtop;
                obj->got_shadow = 1;
                obj->vSN = v010;

                if (bounce != 0) {
                    obj->boing |= bounce;
                    GameSfx(2, &obj->pos);
                    NewRumble((struct rumble_s *)((char *)player + 0xca4), 0x7f);
                    NewBuzz((struct rumble_s *)((char *)player + 0xca4), 0xc);
                } else if (set != 0) {
                    f32 new_x, new_z;
                    new_x = obj->pos.x + opp->mom.x;
                    obj->pos.x = new_x;
                    new_z = obj->pos.z + opp->mom.z;
                    obj->pos.z = new_z;
                    if (opp->dyrot != 0) {
                        u16 a;
                        s32 new_a;
                        obj->hdg += opp->dyrot;
                        dx = new_x - opp->pos.x;
                        dz = new_z - opp->pos.z;
                        a = NuAtan2D(dx, dz);
                        yrot = a;
                        d = NuFsqrt(dz * dz + dx * dx);
                        new_a = yrot + (s32)(s16)opp->dyrot;
                        obj->pos.x += NuTrigTable[(u16)new_a] * d - NuTrigTable[(u16)yrot] * d;
                        obj->pos.z += NuTrigTable[(u16)(new_a + 0x4000)] * d - NuTrigTable[(u16)(yrot + 0x4000)] * d;
                    }
                }
            top_touch:
                opp->touch = 2;
                goto contact_assign;
            }

            if (obj->oldobjtop <= opp->oldobjbot + YTOL) {
                /* BOTTOM collision: player below opponent */
                side = 4;
                attack = obj->attack;
                set = 0;
                if ((attack & opp->vulnerable & 0x8e) != 0) {
                    {
                        s32 dead_val = 1;
                        if ((attack & 0x8c) != 0) dead_val = 4;
                        opp->dead = dead_val;
                    }
                    if ((obj->attack & 2) != 0) {
                        obj->mom.y = 0.0f;
                        if ((obj->flags & 1) != 0 && player->jump != 0) {
                            player->jump = 6;
                            player->jump_type = 4;
                            player->jump_frame = player->jump_frames;
                        }
                    }
                } else if ((attack & opp->vulnerable & 0x40) != 0) {
                    opp->dead = 4;
                    set = 1;
                } else {
                    if ((attack & 2) != 0 && (opp->attack & 2) != 0) {
                        goto bottom_done;
                    }
                    if (opp->attack != 0) {
                        if (mask_active != 0 && mask_active <= 2) {
                            opp->dead = 4;
                            lose_mask = 1;
                        } else {
                            obj->dead = 1;
                            set = 1;
                        }
                        goto bottom_done;
                    }
                    set = 1;
                }
            bottom_done:
                if (set != 0) {
                    obj->mom.y = 0.0f;
                    obj->pos.y = opp->objbot - obj->top * obj->SCALE;
                }
                opp->touch = 4;
                goto contact_assign;
            }

        check_above:
            if (miss != 0) {
                f32 otop = opp->objtop;
                if (otop < (obj->objtop + obj->objbot) * 0.5f
                    && (obj->got_shadow == 0 || obj->shadow < otop)) {
                    obj->shadow = otop;
                    obj->surface_type = 0;
                    obj->got_shadow = 1;
                    obj->vSN = v010;
                }
                continue;
            }

            /* SIDE collision */
            side = 1;
            attack = obj->attack;
            set = 0;
            if ((attack & opp->vulnerable & 0x9e) != 0) {
                {
                    s32 dead_val = 1;
                    if ((attack & 0x80) != 0) dead_val = 4;
                    opp->dead = dead_val;
                }
            } else {
                if ((attack & 2) != 0 && (opp->attack & 2) != 0) {
                    set = 1;
                    goto side_touch;
                }
                if (opp->attack != 0) {
                    if (mask_active != 0 && mask_active <= 2) {
                        opp->dead = 4;
                        lose_mask = 1;
                        set = 0;
                        goto side_push;
                    }
                    obj->dead = 1;
                }
                set = 1;
            }
        side_push:
            if (set != 0) {
                if (opp->character == 0x39) {
                    gamesfx_effect_volume = 0x9ffd;
                    GameSfx(0x49, &obj->pos);
                }

                if (cuboid != 0) {
                    yrot = (s32)temp_cuboid_bounce_angle;
                } else {
                    yrot = NuAtan2D(obj->pos.x - opp->pos.x, obj->pos.z - opp->pos.z);
                }

                {
                    f32 speed;
                    if ((opp->attack & 2) != 0) {
                        speed = 0.1f;
                    } else {
                        speed = 0.05f;
                    }
                    obj->mom.x = NuTrigTable[(u16)yrot] * speed;
                    obj->mom.z = NuTrigTable[(u16)(yrot + 0x4000)] * speed;
                }
                temp_xzmomset = 1;
            }
        side_touch:
            opp->touch = side;
        contact_assign:
            opp->contact = obj;
            opp->contact_type = side;
        }

    post_contact:
        if (opp->dead == 0) {
            if (obj->dead != 0) goto player_die;
        } else {
            pos.x = (obj->pos.x + opp_pos.x) * 0.5f;
            pos.z = (obj->pos.z + opp_pos.z) * 0.5f;
            if (side == 2) {
                pos.y = obj->bot * obj->SCALE + obj->pos.y;
            } else if (side == 4) {
                pos.y = obj->top * obj->SCALE + obj->pos.y;
            } else {
                pos.y = ((opp->objbot + opp->objtop) * 0.5f
                    + (obj->bot + obj->top) * obj->SCALE * 0.5f + obj->pos.y) * 0.5f;
            }
            AddGameDebris(0x82, &pos);
            NewRumble((struct rumble_s *)((char *)player + 0xca4), 0x7f);
            NewBuzz((struct rumble_s *)((char *)player + 0xca4), 6);

            if ((opp->flags & 0x40000) != 0 && (obj->attack & 0x80) == 0) {
                opp->dead = 0;
                opp->kill_contact = 1;
            } else {
                if (opp->dead == 1) {
                    FlyGameObject(opp, NuAtan2D(opp->pos.x - obj->pos.x, opp->pos.z - obj->pos.z));
                }
                KillGameObject(opp, (s32)(s8)opp->dead);
            }
        }
    }

    if (lose_mask != 0) {
        if (obj->invincible == 0) {
            LoseMask(obj);
        }
    } else if (obj->dead != 0) {
    player_die:
        if (obj->invincible != 0) {
            obj->dead = 0;
        } else {
            s32 die = (s32)(s8)(*(u8 *)((char *)opp->parent + 0x21c));
            if (die == -1) {
                die = GetDieAnim(obj, -1);
            }
            KillGameObject(obj, die);
        }
    }

    if (VEHICLECONTROL == 1 && obj->vehicle == 0x53) {
        ObjectToAtlas(obj, player);
    }
}

s32 ObjectCylinderCollision(struct obj_s *obj, struct obj_s *cyl, f32 scale, s32 flag) {
    f32 dz;
    f32 dx;
    f32 r;
    f32 dist_sq;
    s32 side;
    s32 result;
    u16 a;
    f32 obj_bot_scaled;
    f32 cyl_top_y;
    f32 cyl_bot_y;
    f32 obj_top_scaled;

    dz = obj->pos.z - cyl->pos.z;
    dx = obj->pos.x - cyl->pos.x;
    r = obj->RADIUS + cyl->RADIUS;
    dist_sq = dx * dx + dz * dz;
    if (flag != 0) {
        r *= scale;
    }
    if (dist_sq > r * r) {
        if (flag == 0) {
            return 0;
        }
        a = NuAtan2D(dx, dz);
        obj->pos.x = NuTrigTable[a] * r + cyl->pos.x;
        obj->pos.z = NuTrigTable[(u16)(a + 0x4000)] * r + cyl->pos.z;
    }

    side = 0;
    obj_top_scaled = obj->top * obj->SCALE;

    if (obj->pos.y + obj_top_scaled < cyl->bot * cyl->SCALE + cyl->pos.y) {
        side = 1;
    } else {
        obj_bot_scaled = obj->bot * obj->SCALE;
        cyl_top_y = cyl->top * cyl->SCALE + cyl->pos.y;
        if (obj->pos.y + obj_bot_scaled > cyl_top_y) {
            side = 1;
        } else {
            cyl_bot_y = cyl->bot * cyl->SCALE + cyl->pos.y;

            if (obj->bot * obj->old_SCALE + obj->oldpos.y >= cyl->top * cyl->old_SCALE + cyl->oldpos.y - YTOL) {
                obj->pos.y = cyl_top_y - obj_bot_scaled;
                obj->ground = 3;
                result = 2;
                obj->got_shadow = 1;
                obj->surface_type = 0;
                obj->shadow = cyl->top * cyl->SCALE + cyl->pos.y;
                obj->vSN = v010;
                obj->mom.y = cyl->mom.y;
                goto apply;
            }
            if (obj->top * obj->old_SCALE + obj->oldpos.y <= cyl->bot * cyl->old_SCALE + cyl->oldpos.y + YTOL) {
                result = 4;
                obj->pos.y = cyl_bot_y - obj_top_scaled;
                obj->mom.y = cyl->mom.y - YTOL;
                goto apply;
            }
        }
    }

    if (side != 0) {
        f32 mid_y;
        f32 cyl_top_point;

        mid_y = (obj->bot + obj->top) * obj->SCALE * 0.5f + obj->pos.y;
        cyl_top_point = cyl->top * cyl->SCALE + cyl->pos.y;
        if (cyl_top_point < mid_y) {
            if (obj->got_shadow == 0 || cyl_top_point > obj->shadow) {
                obj->shadow = cyl_top_point;
                obj->got_shadow = 1;
                obj->surface_type = 0;
                obj->vSN = v010;
            }
        }
        result = 0;
    } else {
        result = 1;
        a = NuAtan2D(obj->pos.x - cyl->pos.x, obj->pos.z - cyl->pos.z);
        obj->mom.x = NuTrigTable[a] * 0.050000004f;
        obj->mom.z = NuTrigTable[(u16)(a + 0x4000)] * 0.050000004f;
    }

apply:
    if (result & 6) {
        f32 new_x;
        f32 new_z;

        obj->vSN = v010;
        new_x = obj->pos.x + cyl->mom.x;
        obj->pos.x = new_x;
        new_z = obj->pos.z + cyl->mom.z;
        obj->pos.z = new_z;
        if (cyl->dyrot != 0) {
            if ((result & 2) || ((result & 4) && (cyl->flags & 0x200))) {
                f32 dist;
                u16 new_a;

                obj->hdg += cyl->dyrot;
                dx = new_x - cyl->pos.x;
                dz = new_z - cyl->pos.z;
                a = NuAtan2D(dx, dz);
                dist = NuFsqrt(dz * dz + dx * dx);
                new_a = a + cyl->dyrot;
                obj->pos.x += NuTrigTable[new_a] * dist - NuTrigTable[a] * dist;
                obj->pos.z += NuTrigTable[(u16)(new_a + 0x4000)] * dist - NuTrigTable[(u16)(a + 0x4000)] * dist;
            }
        }
    }

    return result;
}

//NGC MATCH
void ClearGameObjects(void) {
  s32 i;
  
  for (i = 0; i < 0x40; i++) {
    pObj[i] = NULL;
  }
  return;
}

//NGC MATCH
void CountGameObjects() 
{
    int i;

    for (i = 64; i > 0; i--) 
    { 
        if (pObj[i - 1] != NULL) 
        {
            break;
        }
    }
    
    GAMEOBJECTCOUNT = i; 
}

//NGC MATCH
void ResetGameObject(struct obj_s *obj) {
  memset(obj,0,0x188);
  obj->reflect_y = 2000000.0f;
  obj->SCALE = 1.0f;
  (obj->RPos).iRAIL = -1;
  (obj->RPos).iALONG = -1;
  obj->layer_type = -1;
  obj->roof_type = -1;
  obj->scale = 1.0f;
  return;
}

//NGC MATCH
s32 AddGameObject(struct obj_s *obj,void *parent) {
  s32 i;
  s32 ok;
  
  ResetGameObject(obj);
  i = 0;
  while ((i < 0x40) && (pObj[i] != NULL)) {
    i++;
  } 
  if (i < 0x40) {
    ok = 1;
    pObj[i] = obj;
    obj->parent = (struct obj_s *)parent;
    obj->i = i;
  }
  else {
    ok = 0;
  }
  CountGameObjects();
  return ok;
}

//NGC MATCH
void RemoveGameObject(struct obj_s *obj) 
{
    int i;
    
    for (i = 0; i < 64; i++) 
    {
        if (pObj[i] == obj) 
        {
            pObj[i] = NULL;
            
            i = 64;
        }
    }
    
    CountGameObjects();
}

//NGC MATCH
float GameObjectRadius(struct obj_s *obj) {
  float r;

  r = obj->radius * obj->SCALE;
  if ((obj->attack & 8) != 0) {
    return r + 0.5f;
  }
  if ((obj->attack & 4) != 0) {
    return r + 0.25f;
  }
  if ((obj->attack & 2) != 0) {
      if ((obj->character == 1) && (*(short *)((int)&obj->parent[8].startpos.y + 2) != 0)) {
        return r * 2.5f;
      } else {
        return r + r;
      }
  }
  return r;
}

//NGC 99% (regswap)
s32 CylinderCuboidOverlapXZ(struct nuvec_s* pos, float radius, struct obj_s* cub, struct nuvec_s* cub_pos) {
    struct nuvec_s vNew;
    float x0;  // f5
    float z0;  // f7
    float x1;  // f6
    float z1;  // f11
    float dx;  // f1
    float dz;  // f2
    float xc;  // f30
    float zc;  // f31
    float _x0; // f8
    float _z0; // f12
    float _x1; // f9
    float _z1; // f0
    unsigned short yrot; // r8
    s32 corner; // r9

    NuVecSub(&vNew, pos, cub_pos);
    NuVecRotateY(&vNew, &vNew, -cub->hdg);
    
    _x0 = cub->min.x * cub->SCALE;
    x0 = _x0 - radius;
    if (vNew.x < x0) {
        return 0;
    }

    _x1 = cub->max.x * cub->SCALE;
    x1 = _x1 + radius;
    if (vNew.x > x1) {
        return 0;
    }

    _z0 = cub->min.z * cub->SCALE;
    z0 = (_z0 - radius);
    if (vNew.z < z0) {
        return 0;
    }

    _z1 = cub->max.z * cub->SCALE;
    z1 = (_z1 + radius);
    if (vNew.z > z1) {
        return 0;
    }

    yrot = 0;
    if (vNew.x < _x0) {
        if (vNew.z < _z0) {
            yrot = 10;
            xc = _x0;
            zc = _z0;

            temp_cuboid_side = ((_z0 - vNew.z) < (_x0 - vNew.x)) ? 0x20 : 0x40;
        } else if (vNew.z > _z1) {
            yrot = 9;
            xc = _x0;
            zc = _z1;

            temp_cuboid_side = ((vNew.z - _z1) < (_x0 - vNew.x)) ? 0x10 : 0x40;
        }
    } else if (vNew.x > _x1) {
        if (vNew.z < _z0) {
            yrot = 6;
            xc = _x1;
            zc = _z0;

            temp_cuboid_side = ((_z0 - vNew.z) < (vNew.x - _x1)) ? 0x20 : 0x80;
        } else if (vNew.z > _z1) {
            yrot = 5;
            xc = _x1;
            zc = _z1;

            temp_cuboid_side = ((vNew.z - _z1) < (vNew.x - _x1)) ? 0x10 : 0x80;
        }
    }
    
    if (yrot != 0) {
        dx = vNew.x - xc;
        dz = vNew.z - zc;
        if ((dx * dx) + (dz * dz) > (radius * radius)) {
            return 0;
        } else {
            temp_cuboid_bounce_angle = NuAtan2D(dx, dz) + cub->hdg;
            return 1;
        }
    }
    
    
    if ((vNew.x - x0) < (x1 - vNew.x)) {
        if ((vNew.z - z0) < (z1 - vNew.z)) {
            corner = ((vNew.z - z0) < (vNew.x - x0)) ? 0x8000 : 0xC000;
        } else {
            corner = ((z1 - vNew.z) < (vNew.x - x0)) ? 0 : 0xC000;
        }
    } else {
        if ((vNew.z - z0) < (z1 - vNew.z)) {
            corner = ((vNew.z - z0) < (x1 - vNew.x)) ? 0x8000 : 0x4000;
        } else {
            corner = ((z1 - vNew.z) < (x1 - vNew.x)) ? 0 : 0x4000;
        }
    }
    
    temp_cuboid_bounce_angle = corner + cub->hdg;

    if (corner == 0) {
        temp_cuboid_side = 0x10;
    } else if (corner == 0x4000) {
        temp_cuboid_side = 0x80;
    } else if (corner == 0x8000) {
        temp_cuboid_side = 0x20;
    } else {
        temp_cuboid_side = 0x40;
    }

    return 1;
}

//NGC MATCH
s32 CylinderCylinderOverlapXZ(struct nuvec_s *p0,float r0,struct nuvec_s *p1,float r1) {
  float dz;
  float dx;
  
  dx = p1->x - p0->x;
  dz = p1->z - p0->z;
  if (dx * dx + dz * dz <= r0 * r0 + r1 * r1) {
     return 1;
  }
  return 0;
}

//NGC MATCH
s32 GameObjectOverlap(struct obj_s *p0,struct obj_s *p1,s32 flag) {
  float dx;
  float dz;
  float r;
  struct nuvec_s pos0;
  struct nuvec_s pos1;
  
  if ((((p0->flags & 0x2000) == 0) ||
      (((p0->draw_frame != 0 && (p0->pLOCATOR != NULL)) && ((p0->model == NULL || (p0->model->pLOCATOR[0] != NULL)))))) &&
     (((p1->flags & 0x2000) == 0 || (((p1->draw_frame != 0 && (p1->pLOCATOR != NULL)) &&
       ((p1->model == NULL || (p1->model->pLOCATOR[0] != NULL)))))))) {
    if ((p0->flags & 0x2000) != 0) {
      pos0.x = p0->pLOCATOR->_30;
      pos0.y = p0->pLOCATOR->_31;
      pos0.z = p0->pLOCATOR->_32;
    }
    else {
      pos0 = (p0->pos);
    }
    if ((p1->flags & 0x2000) != 0) {
      pos1.x = p1->pLOCATOR->_30;
      pos1.y = p1->pLOCATOR->_31;
      pos1.z = p1->pLOCATOR->_32;
    }
    else {
      pos1 = (p1->pos);
    }
    if (!(p0->bot * p0->SCALE + pos0.y > p1->top * p1->SCALE + pos1.y) &&
       !(p1->bot * p1->SCALE + pos1.y > p0->top * p0->SCALE + pos0.y)) {
      dz = pos1.z - pos0.z;
      r = p0->RADIUS + p1->RADIUS;
      dx = pos1.x - pos0.x;
      if (dx * dx + dz * dz <= r * r) {
        if (flag != 0) {
          temp_yrot = NuAtan2D(dx,dz);
        }
        return 1;
      }
    }
  }
  return 0;
}

//NGC MATCH
void FlyGameObject(struct obj_s *obj, u16 yrot) {
  (obj->mom).x = 0.0f;
  (obj->mom).y = 0.0f;
  (obj->mom).z = 0.1666667f;
  NuVecRotateX(&obj->mom,&obj->mom,-0x400);
  NuVecRotateY(&obj->mom,&obj->mom,yrot);
  GameSfx(0x37,&obj->pos);
  return;
}

//NGC MATCH
void GetTopBot(struct creature_s *c) {
  if (c == player) {
    if ((VEHICLECONTROL == 1) && (c->obj.vehicle != -1)) {
      c->obj.bot = CData[c->obj.vehicle].min.y;
      c->obj.top = CData[c->obj.vehicle].max.y;
      return;
    }
    if ((c == player) && (VEHICLECONTROL == 2)) {
      c->obj.bot = CData[115].min.y;
      c->obj.top = CData[115].max.y;
      return;
    }
  }
  if ((c->obj.flags & 0x10000) != 0) {
    c->obj.bot = -c->obj.max.y;
    c->obj.top = -c->obj.min.y;
    return;
  }
  if (c->obj.dangle != 0) {
    c->obj.top = c->obj.max.y;
    if (c->obj.dangle == 2) {
      c->obj.bot = (c->obj.min.y + c->obj.max.y) * 0.5f;
      return;
    }
    c->obj.bot = c->obj.min.y;
    return;
  }
  c->obj.bot = c->obj.min.y;
  if ((((c->obj.ground != 0) || (c->obj.old_ground != 0)) &&
      ((c->crawl != 0 || (((c->slide != 0 && (c->obj.character != 1)) || (c->slam_wait != 0)))))) 
      || (((c->obj.ground == 0 && (c->slam != 0)) && (c->slam < 3)))) {
          c->obj.top = (c->obj.min.y + c->obj.max.y) * 0.5f;
          return;
  }
  c->obj.top = c->obj.max.y;
  return;
}

//NGC MATCH
void NewTopBot(struct obj_s *obj) {
  obj->objbot = obj->bot * obj->SCALE + (obj->pos).y;
  obj->objtop = obj->top * obj->SCALE + (obj->pos).y;
  return;
}

//NGC MATCH
void OldTopBot(struct obj_s *obj) {
  obj->oldobjbot = obj->objbot;
  obj->oldobjtop = obj->objtop;
  return;
}

//NGC MATCH
float CreatureTopBelow(struct nuvec_s *pos, u32 obj_flags) {
    struct obj_s *obj;
    struct nuvec_s obj_pos;
    float top;
    float shadow;
    float dz;
    float dx;
    s32 i;
    
    shadow = 2000000.0f;
    for(i = 0; i < GAMEOBJECTCOUNT; i++) {
        obj = pObj[i];
        if (((obj != NULL) && (obj->invisible == 0)) && ((obj->flags & obj_flags) != 0)) {
            if ((obj->flags & 0x2000) != 0) {
                if (obj->draw_frame == 0) {
                    continue;
                }
                if ((obj->pLOCATOR != NULL) && (obj->model->pLOCATOR[0] != NULL)) {
                    obj_pos.x = obj->pLOCATOR->_30;
                    obj_pos.y = obj->pLOCATOR->_31;
                    obj_pos.z = obj->pLOCATOR->_32;
                } else {
                    goto here;
                }
            }
            else
            {
                here:
                obj_pos = obj->pos;
            }
            if ((obj->flags & 0x8000) != 0) {
                if (CylinderCuboidOverlapXZ(pos, 0.0f, obj, &obj_pos) == 0) {
                    continue;
                }
            }
            else {
                dx = pos->x - obj_pos.x;
                dz = pos->z - obj_pos.z;
                dx *= dx;
                dz *= dz;
                if ((dx + dz) > (obj->RADIUS * obj->RADIUS)) {
                    continue;
                }
            }
            
            top = (obj->top * obj->SCALE + obj->pos.y);
            if ((shadow == 2000000.0f) || (top > shadow)) {
                shadow = top;
            }
        }
    }
    return shadow;
}

//NGC MATCH
s32 WumpaCollisions(struct obj_s *obj) {
    struct wumpa_s *wumpa;
    struct winfo_s* info;
    float dx;
    float dz;
    float y;
    float r2;
    s32 i;
    s32 got;
    s32 attack;
    s32 key;
    
    if (TimeTrial == 0) {
        
        if (level_part_2 != 0) {
            return 0;
        }
        
        r2 = GameObjectRadius(obj) + 0.2f;
        r2 *= r2;
        attack = obj->attack & 2;
        wumpa = Wumpa;
    
        for(i = 0; i < 320; i++, wumpa++) {
            if (wumpa->active == 2) {
                info = &WInfo[i & 7];
                y = wumpa->pos.y + info->dy;
                if ((obj->bot * obj->SCALE + obj->pos.y) < (y + 0.1f) && (obj->top * obj->SCALE + obj->pos.y) > (y - 0.1f)) {
                    dx = obj->pos.x - wumpa->pos.x;
                    dz = obj->pos.z - wumpa->pos.z;
                    if ((dx * dx + dz * dz) < r2) {
                        if ((attack != 0)) {
                            FlyWumpa(wumpa);
                            break; 
                        }
                        if (obj->dead == 0) {
                            AddScreenWumpa(wumpa->pos.x, y, wumpa->pos.z, 1);
                            key = -1;
                            AddFiniteShotDebrisEffect(&key, GDeb[130].i, &wumpa->pos, 1);
                            wumpa->active = 0;
                            GameSfx(0x2c, &obj->pos);
                        }
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

//NGC MATCH
void MoveLoopXZ(struct obj_s *obj,u16 *ay) {
  float temp;
  u16 yrot;
  struct nuvec_s mom;
  
  if (ay != NULL) {
    yrot = *ay;
  }
  else {
    yrot = obj->hdg;
  }
  NuVecRotateY(&mom,&obj->mom,-yrot);
  if (mom.x > 0.0f) {
    mom.x = mom.x - in_s_friction;
    if (mom.x < 0.0f) {
      mom.x = 0.0f;
    }    
  }
  else if (mom.x < 0.0f) {
        mom.x += in_s_friction;
        if (mom.x > 0.0f) {
            mom.x = 0.0f;
        }
  }
  temp = -0.075f;
  if ((mom.x < -0.075f) || (temp = 0.075f, 0.075f < mom.x)) {
    mom.x = (temp - mom.x) * 0.25f + mom.x;
  }
  obj->side = mom.x;
  obj->abs_side = NuFabs(mom.x);
  if (mom.z > in_speed) {
    mom.z -= in_f_friction;
    if (mom.z < in_speed) {
        mom.z = in_speed;
    }
  }
  else if (mom.z < in_speed) {
    mom.z += in_f_friction;
        if (mom.z > in_speed) {
            mom.z = in_speed;
        }
  }
  obj->forward = mom.z;
  obj->abs_forward = NuFabs(mom.z);
  NuVecRotateY(&obj->mom,&mom,yrot);
  return;
}

//NGC MATCH
void FindAnglesZX(struct nuvec_s *src) {
  struct nuvec_s v;
  unsigned short a;

  a = NuAtan2D(src->z, src->y);
  temp_xrot = a;
  NuVecRotateX(&v, src, -a);
  a = NuAtan2D(v.x, v.y);
  temp_zrot = -a;
}

//NGC MATCH
void GetSurfaceInfo(struct obj_s *obj,s32 ter,float surface_y) {
  s32 surface_type;
  s32 temp;
  s32 old;
  u16 yrot;
  struct nuvec_s v;

  if (surface_y != 2000000.0f) {
    surface_type = ShadowInfo();
    if ((u32)surface_type > 0xf) {
      surface_type = 0;
    }
    obj->reflect_y = ((TerSurface[surface_type].flags & 4) == 0) ? 2000000.0f : surface_y;
  }
  else {
    obj->reflect_y = 2000000.0f;
    surface_type = 0;
  }
  if (ter != 0) {
    old = obj->surface_type;
    obj->surface_type = (char)surface_type;
    if (obj->shadow == 2000000.0f) {
      obj->shadow = SAFEY;
      (obj->vSN) = v010;
    }
    else {
      (obj->vSN) = ShadNorm;
      if ((((obj->flags & 1) != 0) && (obj->ground != 0)) && (1 < ter)) {
        if (obj->surface_type == 6) {
          ShadowDir(&v);
          temp = NuAtan2D(v.x,v.z);
          yrot = temp + 0x4000U & 0xffff;
          plr_conveyor_yrot = (u16)(temp + 0x4000U);
          obj->pos.x = NuTrigTable[yrot] * 0.025f + obj->pos.x;
          obj->pos.z = NuTrigTable[(yrot + 0x4000) & 0x2ffff] * 0.025f + obj->pos.z;
          NuVecSub(&plr_conveyor_mom,&obj->pos,&obj->oldpos);
        }
        else if (old == 6) {
          temp = NuAtan2D(plr_conveyor_mom.x,plr_conveyor_mom.z);
          if (NuFabs(RotDiff(plr_conveyor_yrot,(u16)temp)) < 16384.0f) {
            (obj->mom).x = NuTrigTable[plr_conveyor_yrot] * 0.025f + NuTrigTable[plr_conveyor_yrot] * 0.025f;
            (obj->mom).z = NuTrigTable[(plr_conveyor_yrot + 0x4000) & 0x2ffff] * 0.025f;
            (obj->mom).z += (obj->mom).z;
          }
        }
      }
    }
  }
  obj->layer_shadow = EShadY;
  if ((EShadY != 2000000.0f) && (obj->bot * obj->SCALE + obj->pos.y < EShadY)) {
        obj->layer_type = (char)EShadowInfo();
        if (((u32)obj->layer_type & 0xff) > 10) {
          obj->layer_type = 0;
        }
        if ((TerLayer[obj->layer_type].flags & 0x40) == 0) {
          (obj->vLN) = EShadNorm;
          FindAnglesZX(&obj->vLN);
          obj->layer_xrot = temp_xrot;
          obj->layer_zrot = temp_zrot;
        } else {
          obj->layer_type = -1;
          (obj->vLN) = v010;
          obj->layer_xrot = 0;
          obj->layer_zrot = 0;
        }
  } else {
      obj->layer_type = -1;
      (obj->vLN) = v010;
      obj->layer_xrot = 0;
      obj->layer_zrot = 0;
  }
  obj->roof_y = ShadRoofY;
  if ((ShadRoofY != 2000000.0f) && (obj->bot * obj->SCALE + obj->pos.y < ShadRoofY)) {
    obj->roof_type = ShadowRoofInfo();
    if (0xf < ((u32)obj->roof_type & 0xff)) {
      obj->roof_type = 0;
    }
    (obj->vRN).x = -ShadRoofNorm.x;
    (obj->vRN).y = -ShadRoofNorm.y;
    (obj->vRN).z = -ShadRoofNorm.z;
    FindAnglesZX(&obj->vRN);
    temp_roof_xrot = temp_xrot;
    temp_roof_zrot = temp_zrot;
  }
  else {
    obj->roof_type = -1;
    (obj->vRN) = v010;
    temp_roof_xrot = 0;
    temp_roof_zrot = 0;
  }
  return;
}

f32 CrateBottomAbove(struct nuvec_s *pos);

void ObjectRotation(struct obj_s *obj, s32 mode, s32 speedmul) {
    struct nuvec_s v;
    u16 ax;
    u16 az;
    s32 hack;
    s32 vehicle;
    s32 ground;
    f32 f;

    hack = -1;
    if (obj == &player->obj) {
        if (VEHICLECONTROL == 1) {
            if (obj->vehicle != -1) {
                hack = obj->vehicle;
            }
        }
    }

    az = 0;
    ground = (obj->bot * obj->SCALE + obj->pos.y - obj->shadow) < 0.25f;

    if (obj->vSN.x == 0.0f && obj->vSN.y == 1.0f && obj->vSN.z == 0.0f) {
        temp_xrot = 0;
        temp_zrot = 0;
    } else {
        if (speedmul != 0) {
            v = obj->vSN;
            f = obj->xz_distance / (1.0f / 6.0f);
            if (f > 1.0f) {
                f = 1.0f;
            }
            v.x = (v.x - v010.x) * f + v010.x;
            v.y = (v.y - v010.y) * f + v010.y;
            v.z = (v.z - v010.z) * f + v010.z;
            FindAnglesZX(&v);
            ax = temp_xrot;
            vehicle = temp_zrot;
            az = 1;
        }
        FindAnglesZX(&obj->vSN);
    }

    if (obj->flags & 0x10000) {
        if (mode == 1) {
            obj->surface_xrot = temp_xrot;
            obj->zrot = temp_roof_zrot;
            obj->surface_zrot = temp_zrot;
            obj->xrot = temp_roof_xrot;
            obj->roof_xrot = temp_roof_xrot;
            obj->roof_zrot = temp_roof_zrot;
        } else if (mode == 2) {
            obj->surface_xrot = SeekRot(obj->surface_xrot, temp_xrot, 3);
            obj->surface_zrot = SeekRot(obj->surface_zrot, temp_zrot, 3);
            obj->roof_xrot = SeekRot(obj->roof_xrot, temp_roof_xrot, 3);
            obj->roof_zrot = SeekRot(obj->roof_zrot, temp_roof_zrot, 3);
            obj->xrot = SeekRot(obj->xrot, obj->roof_xrot, 2);
            obj->zrot = SeekRot(obj->zrot, obj->roof_zrot, 2);
        }
    } else {
        if (mode == 1) {
            obj->surface_xrot = temp_xrot;
            obj->surface_zrot = temp_zrot;
            if (ground != 0 && hack != 0x3B) {
                obj->zrot = temp_zrot;
                obj->xrot = temp_xrot;
            } else {
                obj->xrot = 0;
                obj->zrot = 0;
            }
        } else if (mode == 2) {
            if (obj == &player->obj && VEHICLECONTROL == 2) {
                obj->surface_xrot = SeekRot(obj->surface_xrot, temp_xrot, 3);
                obj->surface_zrot = SeekRot(obj->surface_zrot, temp_zrot, 3);

                if (obj->pad_speed == 0.0f || (f32)(s16)(u16)(s32)NuFabs(obj->pad_dx) < (1.0f / 3.0f)) {
                    ax = (u16)(s32)(obj->pad_dz * 16384.0f);
                } else {
                    ax = (u16)(0x4000 - abs(RotDiff(0, obj->pad_angle)));
                }

                if (obj->shadow != 2000000.0f && ax > 0x8000 && ax <= 0xF554) {
                    f = obj->bot * obj->SCALE + obj->pos.y - obj->shadow;
                    if (f < 1.0f) {
                        ax = (u16)(-(s32)(f * 16384.0f));
                        goto vehicle_seekrot;
                    }
                }

                if (obj->roof_y != 2000000.0f && !(ax & 0x8000) && ax > 0x0AAB) {
                    f = obj->roof_y - (obj->top * obj->SCALE + obj->pos.y);
                    if (f < 1.0f) {
                        goto apply_constraint;
                    }
                }

                v.x = obj->pos.x;
                v.y = (obj->bot + obj->top) * obj->SCALE * 0.5f + obj->pos.y;
                v.z = obj->pos.z;
                f = CrateBottomAbove(&v);
                if (f != 2000000.0f && !(ax & 0x8000)) {
                    f = f - (obj->top * obj->SCALE + obj->pos.y);
                    if (f < 1.0f) {
                        if (f < 0.0f) {
                            f = 0.0f;
                        }
                        goto apply_constraint;
                    }
                }
                goto vehicle_seekrot;

            apply_constraint:
                ax = (u16)(s32)(f * 16384.0f);
            vehicle_seekrot:
                obj->xrot = SeekRot(obj->xrot, ax, 5);
                obj->zrot = 0;

            } else if (hack == 0x20) {
                obj->surface_xrot = SeekRot(obj->surface_xrot, temp_xrot, 3);
                obj->surface_zrot = SeekRot(obj->surface_zrot, temp_zrot, 3);
                if (obj->pad_speed > 0.0f) {
                    ax = (u16)(0x4000 - abs(RotDiff(0, obj->pad_angle)));
                } else {
                    ax = 0;
                }
                obj->xrot = SeekRot(obj->xrot, ax, 5);
                obj->zrot = 0;

            } else {
                obj->surface_xrot = SeekRot(obj->surface_xrot, temp_xrot, 3);
                obj->surface_zrot = SeekRot(obj->surface_zrot, temp_zrot, 3);

                if (hack != 0x3B) {
                    if (ground != 0) {
                        if (az == 0) {
                            vehicle = obj->surface_zrot;
                            ax = obj->surface_xrot;
                        }
                    } else {
                        vehicle = 0;
                        ax = 0;
                    }

                    if (!(obj->flags & 0x20000)) {
                        if (ground != 0 || hack != 0x99) {
                            obj->xrot = SeekRot(obj->xrot, ax, 2);
                            obj->zrot = SeekRot(obj->zrot, vehicle, 2);
                        }
                    }
                } else {
                    obj->xrot = 0;
                    obj->zrot = 0;
                }
            }
        }
    }
}

s32 KillGameObject(struct obj_s *obj, s32 die) {
    struct nuvec_s objmid;
    s32 sfx;
    s32 vehicle;
    u16 hdg_facecamera;
    s32 check_anim_flag;
    f32 extra_time;
    s16 character;
    s8 dead;
    s16 die_action_val;
    s32 idx;
    struct nuanimdata_s *anm;
    struct animlist *al;
    f32 dur;

    sfx = -1;
    vehicle = -1;
    obj->dead = (char)die;

    objmid.x = obj->pos.x;
    objmid.y = (obj->bot + obj->top) * obj->SCALE * 0.5f + obj->pos.y;
    objmid.z = obj->pos.z;

    if (obj == &player->obj && VEHICLECONTROL == 1 && obj->vehicle != -1) {
        vehicle = (s32)obj->vehicle;
    }

    if (die == 0x14) {
        die = 6;
        if (obj->flags & 1) {
            if (VEHICLECONTROL == 2) {
                die = 0xe;
            } else if (VEHICLECONTROL == 1 && obj->vehicle != -1) {
                die = 0xb;
            } else if (Level == 0xf) {
                die = 0xd;
            }
        }
        obj->dead = (char)die;
    }

    if (obj->flags & 1) {
        if (obj->transporting != 0 || InvincibilityCHEAT != 0) {
            obj->dead = 0;
            return 0;
        }
        if (vehicle == 0x44 || vehicle == 0xb2 || vehicle == 0x20 ||
            vehicle == 0x3b || vehicle == 0x53) {
            obj->dead = 0x16;
        } else if (die == 0x11 && VEHICLECONTROL == 1 && obj->vehicle == 0x20) {
            obj->dead = 1;
        }
        if (obj->dead != 0) {
            old_bonus_crates = bonus_crates_destroyed;
            NewBuzz(&player->rumble, 0x3c);
            NewRumble(&player->rumble, 0xff);
            if (vehicle == 0xa1) {
                sfx = 0x71;
            }
        }
        if (sfx != -1) {
            GameSfx(sfx, &obj->pos);
        }
        obj->target_yrot = 0;
        obj->target_xrot = 0;
    }

    extra_time = 0.0f;
    sfx = -1;
    obj->die_time = 0.0f;

    hdg_facecamera = NuAtan2D(GameCam[0].pos.x - obj->pos.x, GameCam[0].pos.z - obj->pos.z);

    dead = (s8)obj->dead;
    check_anim_flag = 1;
    if (dead == 0x16) {
        check_anim_flag = 0;
    }

    character = obj->character;

    if (character == 0) {
        sfx = 0x15;
        switch (dead) {
        case 3:
            obj->die_model[0] = CRemap[0];
            obj->die_model[1] = CRemap[7];
            obj->die_action = 0x1d;
            goto done_switch;
        case 4:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[0];
            check_anim_flag = 0;
            obj->die_action = -1;
            extra_time = 3.0f;
            goto done_switch;
        case 5:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[10];
            sfx = 8;
            obj->die_action = 0x19;
            extra_time = 1.0f;
            goto done_switch;
        case 6:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[10];
            sfx = 0x16;
            obj->die_action = 0x18;
            extra_time = 1.0f;
            goto done_switch;
        case 8:
            obj->die_model[0] = CRemap[0];
            sfx = 9;
            obj->die_model[1] = CRemap[0x4F];
            obj->die_action = 0xf;
            goto done_switch;
        case 9:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[0];
            obj->die_action = 0xf;
            goto done_switch;
        case 0xa:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[0];
            obj->die_action = 0x17;
            extra_time = 1.0f;
            goto done_switch;
        case 0xb: {
            s32 v = (s32)obj->vehicle;
            obj->die_model[0] = CRemap[0];
            if (v != 99 && v != 0x36 && v != 0x81 && v != 0x53 && v != 0x8b) {
                obj->die_model[1] = CRemap[v];
            }
            obj->die_action = 0x59;
            extra_time = 1.0f;
            goto done_switch;
        }
        case 0xc: {
            s32 v = (s32)obj->vehicle;
            obj->die_model[0] = CRemap[0];
            if (v != 99 && v != 0x36 && v != 0x81 && v != 0x53 && v != 0x8b) {
                obj->die_model[1] = CRemap[v];
            }
            obj->die_action = 0x5b;
            extra_time = 1.0f;
            goto done_switch;
        }
        case 0xd:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[0x97];
            sfx = 7;
            obj->die_action = 1;
            extra_time = 1.0f;
            goto done_switch;
        case 0xe:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[0x9D];
            obj->die_action = 0x18;
            extra_time = 1.0f;
            goto done_switch;
        case 0xf:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[0x9E];
            obj->die_action = 0xe;
            extra_time = 1.0f;
            goto done_switch;
        case 0x10:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[0];
            sfx = 0x1d;
            obj->die_action = 0x10;
            extra_time = 1.0f;
            goto done_switch;
        case 0x11:
            if ((((u32 *)&LBIT)[0] & 4) || (((u32 *)&LBIT)[1] & 0x40)) {
                obj->die_model[0] = CRemap[0x73];
                obj->die_model[1] = CRemap[0xB0];
            } else {
                obj->die_model[0] = CRemap[0];
                obj->die_model[1] = CRemap[0xAF];
            }
            obj->die_action = 0x12;
            extra_time = 2.0f;
            goto done_switch;
        case 0x12:
            obj->die_model[0] = CRemap[0];
            sfx = 0x23;
            extra_time = 1.0f;
            obj->die_model[1] = CRemap[0xA8];
            obj->die_action = 0x10;
            goto done_switch;
        case 0x13:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[0];
            sfx = 0x24;
            obj->die_action = 0x11;
            extra_time = 1.0f;
            goto done_switch;
        default:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[0];
            sfx = 0x14;
            obj->die_action = 0xe;
            extra_time = 1.0f;
            goto done_switch;
        }
    } else if (character == 1) {
        switch (dead) {
        case 0xa:
            obj->die_action = 0x17;
            obj->die_model[0] = CRemap[1];
            extra_time = 1.0f;
            obj->die_model[1] = (char)0xff;
            goto done_switch;
        case 0xb: {
            s32 v = (s32)obj->vehicle;
            obj->die_model[0] = CRemap[1];
            extra_time = 1.0f;
            obj->die_model[1] = CRemap[v];
            obj->die_action = 0x59;
            goto done_switch;
        }
        case 0xc: {
            s32 v = (s32)obj->vehicle;
            obj->die_model[0] = CRemap[1];
            if (v != 0x81) {
                obj->die_model[1] = CRemap[v];
            }
            obj->die_action = 0x5b;
            extra_time = 1.0f;
            goto done_switch;
        }
        case 3:
            obj->die_model[0] = CRemap[1];
            obj->die_model[1] = CRemap[7];
            obj->die_action = 0x1d;
            goto done_switch;
        default:
            obj->die_model[1] = (char)0xff;
            obj->die_model[0] = CRemap[1];
            obj->die_action = 0xe;
            goto done_switch;
        }
    } else if (character == 0xf) {
        obj->die_action = 0xe;
        obj->die_model[0] = CRemap[character];
        if (dead == 6) {
            obj->die_model[0] = CRemap[16];
            if ((s8)CRemap[16] != -1) {
                obj->die_action = 0x18;
            }
        }
        obj->die_model[1] = (char)0xff;
        goto done_switch;
    } else {
        obj->die_model[0] = CRemap[(s16)obj->character];
        obj->die_model[1] = (char)0xff;
        obj->die_action = -1;
        dead = (s8)obj->dead;
        if (dead != 4 && dead != 7 && dead != 0x15) {
            obj->dead = 1;
        }
        check_anim_flag = 0;
    }

done_switch:
    if ((s8)obj->die_model[0] == -1 ||
        (check_anim_flag && CModel[(s8)obj->die_model[0]].anmdata[(s16)obj->die_action] == NULL)) {
        obj->dead = 1;
    }

    if (obj->dead == 1) {
        obj->oldpos = obj->pos;
        obj->die_model[0] = CRemap[(s16)obj->character];
        obj->die_action = -1;
    }

    dead = (s8)obj->dead;
    switch (dead) {
    case 2:
    case 5:
    case 6:
    case 0xa:
    case 0xb:
    case 0xc:
    case 0xe:
    case 0x10:
    case 0x12:
    case 0x13:
    generic_anim:
        idx = (s8)obj->die_model[0] * (s32)0x7AC + (s16)obj->die_action * 4;
        al = *(struct animlist **)((u8 *)&CModel[0] + 0x1DC + idx);
        anm = *(struct nuanimdata_s **)((u8 *)&CModel[0] + 4 + idx);
        dur = (anm->time - 1.0f) * (1.0f / (al->speed * 30.0f));
        obj->die_duration = dur;
        if (dead == 2 && obj == &player->obj) {
            BlendGameCamera(GameCam, dur * 0.5f);
        }
        obj->die_duration = dur + extra_time;
        break;
    case 3:
        plr_invisibility_time = 0.0f;
        obj->hdg = (u16)hdg_facecamera + 0x8000;
        idx = (s8)obj->die_model[0] * (s32)0x7AC + (s16)obj->die_action * 4;
        al = *(struct animlist **)((u8 *)&CModel[0] + 0x1DC + idx);
        anm = *(struct nuanimdata_s **)((u8 *)&CModel[0] + 4 + idx);
        dur = (anm->time - 1.0f) * (1.0f / (al->speed * 30.0f)) + extra_time;
        obj->die_duration = dur;
        BlendGameCamera(GameCam, dur * 0.5f);
        break;
    case 4:
        dur = extra_time + 0.0f;
        obj->die_duration = dur;
        AddGameDebris(7, &objmid);
        break;
    case 7:
        obj->die_duration = 0.0f;
        break;
    case 8:
        obj->pos.y = obj->layer_shadow;
        obj->hdg = (u16)hdg_facecamera + 0x8000;
        BlendGameCamera(GameCam, 0.5f);
        idx = (s8)obj->die_model[0] * (s32)0x7AC + (s16)obj->die_action * 4;
        al = *(struct animlist **)((u8 *)&CModel[0] + 0x1DC + idx);
        anm = *(struct nuanimdata_s **)((u8 *)&CModel[0] + 4 + idx);
        dur = (anm->time - 1.0f) * (1.0f / (al->speed * 30.0f)) + extra_time;
        obj->die_duration = dur;
        BlendGameCamera(GameCam, dur * 0.5f);
        break;
    case 9:
    case 0xd:
        obj->hdg = (u16)hdg_facecamera;
        idx = (s8)obj->die_model[0] * (s32)0x7AC + (s16)obj->die_action * 4;
        al = *(struct animlist **)((u8 *)&CModel[0] + 0x1DC + idx);
        anm = *(struct nuanimdata_s **)((u8 *)&CModel[0] + 4 + idx);
        obj->die_duration = (anm->time - 1.0f) * (1.0f / (al->speed * 30.0f)) + extra_time;
        break;
    case 0xf:
        obj->pos.y = (obj->bot + obj->top) * obj->SCALE * 0.5f + obj->pos.y;
        obj->hdg = (u16)hdg_facecamera;
        obj->die_duration = 3.0f;
        BlendGameCamera(GameCam, 1.0f);
        break;
    case 0x11:
        obj->zrot = 0;
        obj->xrot = 0;
        obj->hdg = (u16)hdg_facecamera;
        idx = (s8)obj->die_model[0] * (s32)0x7AC + (s16)obj->die_action * 4;
        al = *(struct animlist **)((u8 *)&CModel[0] + 0x1DC + idx);
        anm = *(struct nuanimdata_s **)((u8 *)&CModel[0] + 4 + idx);
        obj->die_duration = (anm->time - 1.0f) * (1.0f / (al->speed * 30.0f)) + extra_time;
        break;
    case 0x14:
        goto generic_anim;
    case 0x15:
        obj->die_duration = 0.0f;
        sfx = 0x47;
        AddGameDebris(0x8e, &objmid);
        break;
    case 0x16:
        obj->die_model[0] = CRemap[(s16)obj->character];
        obj->die_model[1] = (char)0xff;
        sfx = 0x3b;
        obj->die_duration = 3.0f;
        obj->die_action = -1;
        AddMechanicalDebris(&obj->pos, vehicle);
        break;
    default:
        obj->die_duration = 0.5f;
        break;
    }

    if (obj->die_action != -1 && obj->dead != 0xa) {
        ResetAnimPacket(&obj->anim, (s32)obj->die_action);
    }

    if (obj->flags & 0x20) {
        s32 bd = 1;
        if (Game.hub[Hub].flags & 4) {
            bd = 2;
        }
        boss_dead = bd;
    }

    if (obj->character == 0x46) {
        AddExtraLife(&objmid, 2);
    }

    if (sfx != -1) {
        GameSfx(sfx, &obj->pos);
    }
    return 1;
}

//NGC MATCH
void KillItem(struct obj_s *obj) {
  struct creature_s *c;
  
  c = (struct creature_s *)obj->parent;
  obj->dead = 1;
  c->on = 0;
  c->off_wait = 2;
  return;
}

//NGC MATCH
void PickupCrystal(void) {
  plr_crystal.count = 1;
  plr_items |= 1;
  plr_crystal.frame = 0x1e;
  GameSfx(0x26,NULL);
  AddPanelDebris(0.0f,-0.7f,6,0.125f,0x10);
  return;
}

//NGC MATCH
void PickupCrateGem(void) {
  plr_crategem.count = 1;
  plr_items |= 2;
  plr_crategem.frame = 0x1e;
  GameSfx(0x26,NULL);
  AddPanelDebris(-0.2f,-0.7f,6,0.125f,0x10);
  return;
}

//NGC MATCH
void PickupBonusGem(unsigned int item) {
    plr_items |= item;
    plr_bonusgem.item = item;
    plr_bonusgem.count = 1;
    plr_bonusgem.frame = 0x1e;
    
    GameSfx(0x26, NULL);
    AddPanelDebris(0.2f,-0.7f, 6, 0.125f, 0x10);
}

//NGC MATCH
void PickupPower(s32 character) {
    switch (character) {
    case 0xA7:
        new_power = 0;
        break;
    case 0xA5:
        new_power = 1;
        break;
    case 0xA6:
        new_power = 2;
        break;
    case 0xA2:
        new_power = 3;
        break;
    case 0xA4:
        new_power = 4;
        break;
    case 0xA3:
        new_power = 5;
        break;
    }
    Game.powerbits = Game.powerbits | (1 << new_power);
    NewMenu(&Cursor,0x22,-1,-1);
    GameSfx(0x26,NULL);
    if ((Level != 0x15) && (Level != 0x18)) {
        player->slide = 0;
        player->obj.mom.x = 0.0f;
        player->obj.mom.z = 0.0f;
    }
}

//NGC MATCH
void PickupItem(struct obj_s* obj) {
    u32 item;

    if ((new_mode == -1) && (new_level == -1)) {
        switch (obj->character) {
            case 0x75:
                PickupCrystal();
                ClockOff();
                GameSfx(0x26, 0);
                break;
            case 0x76:
                StartTimeTrial(&obj->pos, 0);
                break;
            case 0x77:
                PickupCrateGem();
                ClockOff();
                GameSfx(0x26, 0);
                break;
            case 0x78:
            case 0x79:
            case 0x7A:
            case 0x7B:
            case 0x7C:
            case 0x7D:
                if (obj->character == 0x79) {
                    item = 8;
                } else if (obj->character == 0x7A) {
                    item = 0x20;
                } else if (obj->character == 0x7B) {
                    item = 0x10;
                } else if (obj->character == 0x7C) {
                    item = 0x40;
                } else if (obj->character == 0x7D) {
                    item = 0x80;
                } else {
                    item = 4;
                }
                PickupBonusGem(item);
                GameSfx(0x26, 0);
                break;
            case 0xA2:
            case 0xA3:
            case 0xA4:
            case 0xA5:
            case 0xA6:
            case 0xA7:
                if ((LBIT & 0x03E00000) != 0) {
                    boss_dead = 2;
                }
                PickupPower(obj->character);
                break;
        }
    }

    KillItem(obj);
    return;
}

//NGC MATCH
s32 HitItems(struct obj_s *obj) {
  struct obj_s *cyl;
  s32 i;
  
  if (level_part_2 != 0) {
    return 0;
  }
  for(i = 0; i < 64; i++) {
      cyl = pObj[i];
      if ((((cyl != NULL) && (cyl->dead == 0)) && (cyl->invisible == 0)) &&
         (((cyl->flags & 0x10) != 0 && (GameObjectOverlap(obj,cyl,0) != 0)))) {
        PickupItem(cyl);
        return 1;
      }
  }
  return 0;
}

//NGC MATCH
s32 HitCreatures(struct obj_s *obj, s32 destroy, s32 type) {
    struct obj_s *cyl;
    s32 i; 
    s32 temp;
  
    if (level_part_2 != 0) {
        return 0;
    }
    for (i = 0; i < 64; i++) { 
        cyl = pObj[i];
        
        if ((((cyl != NULL) && (cyl->dead == 0)) && (cyl->invisible == 0)) &&
        ((((!(cyl->flags & 1)) && (temp = cyl->flags, (cyl->flags & 4) != 0)) && (GameObjectOverlap(obj,cyl,1) != 0)))) 
        {
            if ((cyl->vulnerable & 0x100)) {
                if ((cyl->flags & 0x40000)) {
                    cyl->kill_contact = 1;
                }
                else if (destroy == 2) {
                    KillGameObject(cyl, 21);
                }
                else if (destroy != 0) {
                    KillGameObject(cyl, 4);
                }
                else {
                    FlyGameObject(cyl, temp_yrot);
                    KillGameObject(cyl, 1);   
                }
            }
          
            ((struct creature_s*)(cyl->parent))->hit_type = type;
            return 1; 
        }
    }
    return 0;    
}

//NGC MATCH
s32 WipeCreatures(struct RPos_s *rpos) {
  struct obj_s *cyl;
  struct nuvec_s pos;
  s32 i;
  
  if (level_part_2 != 0) {
      return 0;
  }
  for(i = 0; i < 0x40; i++) {
      cyl = pObj[i];
      if ((cyl != NULL) && (cyl->dead == 0) && (cyl->invisible == 0) && (cyl->flags & 1) == 0) {
            if ((cyl->flags & 4) != 0) {
              if ((cyl->flags & 0x2000) != 0) {
                if (cyl->draw_frame != 0) {
                    if (cyl->pLOCATOR != NULL) {
                      if (cyl->model != NULL) {
                        if (cyl->model->pLOCATOR[0] == NULL) continue;
                      }
                      pos.x = cyl->pLOCATOR->_30;
                      pos.y = cyl->pLOCATOR->_31;
                      pos.z = cyl->pLOCATOR->_32;
                      goto LAB_80041e60;
                    }
                  }
                } else {
                   pos = cyl->pos;
    LAB_80041e60:
                   GetALONG(&pos,&cyl->RPos,(s32)(cyl->RPos).iRAIL,(s32)(cyl->RPos).iALONG,1);
                   if (FurtherALONG((s32)rpos->iRAIL,(s32)rpos->iALONG,rpos->fALONG,
                                         (s32)(cyl->RPos).iRAIL,(s32)(cyl->RPos).iALONG,
                                         (cyl->RPos).fALONG) != 0) {
                      FlyGameObject(cyl,NuAtan2D((rpos->pos).x - pos.x,(rpos->pos).z - pos.z));
                      KillGameObject(cyl,1);
                   } 
                }
            }
      }
  }
  return 0;
}

//NGC MATCH
s32 CreatureRayCast(struct nuvec_s *p0,struct nuvec_s *p1) {
    struct nuvec_s v0;
    struct nuvec_s v1;
    struct nuvec_s obj_pos;
    struct nuvec_s min;
    struct nuvec_s max;
    struct obj_s *obj;
    s32 i;
    s32 face;
    float ratio;
    
    ratio = 1.0f;
    for(i = 0; i < GAMEOBJECTCOUNT; i++) {
        obj = pObj[i];
        if ((obj != NULL) && (obj->invisible == 0) && (obj->flags & 0x14) != 0) {
            if ((obj->flags & 0x2000) != 0) {
                if (obj->draw_frame == 0) {
                    continue;
                }
                if ((obj->pLOCATOR != NULL) && (obj->model->pLOCATOR[0] != NULL)) {
                    obj_pos.x = obj->pLOCATOR->_30;
                    obj_pos.y = obj->pLOCATOR->_31;
                    obj_pos.z = obj->pLOCATOR->_32;
                } else {
                    goto here;
                }
            } else {
                here:
                obj_pos = obj->pos;
            }
            if ((obj->flags & 0x8000) != 0) {
                NuVecSub(&v0,p0,&obj_pos);
                NuVecSub(&v1,p1,&obj_pos);
                NuVecRotateY(&v0,&v0,-(u32)obj->hdg);
                NuVecRotateY(&v1,&v1,-(u32)obj->hdg);
                min.x = (obj->min).x * obj->SCALE;
                min.y = (obj->min).y * obj->SCALE;
                min.z = (obj->min).z * obj->SCALE;
                max.x = (obj->max).x * obj->SCALE;
                max.y = (obj->max).y * obj->SCALE;
                max.z = (obj->max).z * obj->SCALE;
                if ((RayIntersectCuboid(&v0,&v1,&min,&max) != 0) && (temp_ratio < ratio)) {
                    face = temp_face;
                    ratio = temp_ratio;
                }
            }
            else {
                NuVecSub(&v0,p0,&obj_pos);
                NuVecSub(&v1,p1,&obj_pos);
                NuVecRotateY(&v0,&v0,-(u32)obj->hdg);
                NuVecRotateY(&v1,&v1,-(u32)obj->hdg);
                min.x = -obj->RADIUS;
                min.y = (obj->min).y * obj->SCALE;
                min.z = -obj->RADIUS;
                max.x = obj->RADIUS;
                max.y = (obj->max).y * obj->SCALE;
                max.z = obj->RADIUS;
                if ((RayIntersectCuboid(&v0,&v1,&min,&max) != 0) && (temp_ratio < ratio)) {
                    face = temp_face;
                    ratio = temp_ratio;
                }
            }
        }
    }
    temp_face = face;
    temp_ratio = ratio;
    return (ratio < 1.0f);
}

//NGC MATCH
s32 GetDieAnim(struct obj_s *obj,s32 die) {
  if ((VEHICLECONTROL == 1) && (obj->vehicle != -1)) {
    if ((die != 6) || (die = 0xc, obj->vehicle != 0x99)) {
      die = 0xb;
    }
  }
  else if (VEHICLECONTROL == 2) {
    die = 0xe;
  }
  else if (die == -1) {
    if ((Level == 0x17) || (qrand() < 0x8000)) {
      die = 2;
    }
    else {
      die = 3;
    }
  }
  return die;
}

//NGC MATCH
s32 KillPlayer(struct obj_s *player_obj,s32 die) {
  if ((((player_obj->dead != 0) || (player_obj->finished != 0)) ||
      (player_obj->invincible != 0)) || (((vtog_time < vtog_duration) && (vtog_blend != 0)))) {
    return 0;
  }
  if ((player_obj->mask != NULL) && ((player_obj->mask->active != 0 && ((LDATA->flags & 0xe00) == 0)))) {
        if (player_obj->mask->active < 3) {
          LoseMask(player_obj);
        }
        return 0;
  }
  return KillGameObject(player_obj,die);
}

//NGC MATCH
void ScaleFlatShadow(struct nuvec_s *s,float y,float shadow,float scale) {
  float dy;
  
  dy = y - shadow;
  if (dy <= 0.0f) {
    s->x = scale;
  }
  else {
    if (dy >= 2.0f) {
      s->x = scale * 0.5f;
    }
    else {
      s->x = scale - scale * 0.5f * dy * 0.5f;
    }
  }
  s->z = s->x;
  s->y = 0.0f;
  return;
}

//NGC MATCH
void ResetProjectiles(void) {
  s32 i;
  struct projectile_s *p;
  
  p = Projectile;
  for(i = 0; i < 0x10; i++) {
    if (p->active != 0) {
      RemoveGameObject(&p->obj);
      p->active = 0;
    }
    p++;
  }
  return;
}

void DrawProjectiles(void) {
    struct Mtx mtx;
    struct nuvec_s sv;
    struct nuvec_s shadow_pos;
    struct ldata_s *ld;
    struct projectile_s *proj;
    f32 draw_dist;
    f32 draw_dist_sq;
    f32 dist_sq;
    f32 time_ratio;
    f32 opacity;
    f32 dz, dx;
    s32 i;

    if (level_part_2 != 0) return;

    ld = LDATA;

    if ((ld->flags & 0x202) || Level == 0x1c) {
        draw_dist = (f32)(short)ld->farclip;
    } else {
        draw_dist = 25.0f;
    }
    if ((f32)(short)ld->farclip < draw_dist) {
        draw_dist = (f32)ld->farclip;
    }

    draw_dist_sq = draw_dist * draw_dist;

    proj = Projectile;
    for (i = 16; i != 0; i--, proj++) {
        if (!proj->active) continue;

        dz = pCam->pos.z - proj->obj.pos.z;
        dx = pCam->pos.x - proj->obj.pos.x;
        dist_sq = dx * dx + dz * dz;
        if (dist_sq >= draw_dist_sq) continue;

        time_ratio = proj->time / proj->duration;

        switch (proj->type) {
        case 3:
        case 8:
        case 9:
        case 14:
        case 15:
            NuMtxSetRotationX(&mtx, proj->obj.xrot);
            NuMtxRotateY(&mtx, proj->obj.hdg + 0x8000);
            break;
        case 4: {
            f32 sf;
            f32 s;
            if (proj->time < 0.2f) {
                sf = proj->time / 0.2f;
            } else {
                sf = 1.0f;
            }
            s = sf * proj->obj.SCALE;
            sv.x = s;
            sv.y = s;
            sv.z = s;
            NuMtxSetScale(&mtx, &sv);
            NuMtxRotateZ(&mtx, proj->obj.zrot);
            NuMtxRotateY(&mtx, proj->obj.hdg);
            break;
        }
        case 5:
            NuMtxSetRotationY(&mtx, proj->obj.hdg);
            break;
        case 7: {
            f32 s = proj->obj.SCALE;
            sv.x = s;
            sv.y = s;
            sv.z = s;
            NuMtxSetScale(&mtx, &sv);
            break;
        }
        case 10: {
            f32 sf;
            f32 s;
            if (proj->time < 0.2f) {
                sf = proj->time / 0.2f;
            } else {
                sf = 1.0f;
            }
            s = sf * proj->obj.SCALE;
            sv.x = s;
            sv.y = s;
            sv.z = s;
            NuMtxSetScale(&mtx, &sv);
            NuMtxRotateX(&mtx, (s32)(NuTrigTable[proj->obj.xrot] * 2731.0f));
            NuMtxRotateY(&mtx, proj->obj.hdg);
            break;
        }
        default:
            NuMtxSetTranslation(&mtx, &proj->obj.pos);
            goto after_translate;
        }
        NuMtxTranslate(&mtx, &proj->obj.pos);
    after_translate:

        {
            s16 idx = proj->i_objtab;
            if (idx != -1) {
                struct nuspecial_s *special = ObjTab[idx].obj.special;
                if (special != NULL && idx != 1) {
                    struct nuinstance_s *inst = special->instance;
                    struct nugscn_s *scene = ObjTab[idx].obj.scene;
                    NuRndrGScnObj(scene->gobjs[inst->objid], (struct numtx_s *)&mtx);
                }
            }
        }

        if (dist_sq < draw_dist_sq
            && (proj->obj.flags & 0x4400) == 0x400
            && proj->obj.shadow != 2000000.0f
            && ObjTab[21].obj.special != NULL) {

            switch (proj->type) {
            case 6:
            case 8:
                opacity = 1.0f - NuTrigTable[(u16)(s32)(time_ratio * 32768.0f)];
                break;
            case 14: {
                f32 diff = proj->obj.bot * proj->obj.SCALE + proj->obj.pos.y - proj->obj.shadow;
                if (diff < 4.0f) {
                    opacity = 5.0f;
                } else if (diff > 7.0f) {
                    opacity = 0.0f;
                } else {
                    opacity = 5.0f - (diff - 4.0f) * (5.0f / 3.0f);
                }
                break;
            }
            default:
                opacity = 2.0f;
                break;
            }

            shadow_pos.x = proj->obj.pos.x;
            shadow_pos.y = proj->obj.shadow + 0.025f;
            shadow_pos.z = proj->obj.pos.z;
            NuRndrAddShadow(&shadow_pos, opacity * proj->obj.RADIUS, 127,
                            proj->obj.surface_xrot, 0, proj->obj.surface_zrot);
        }
    }
}
