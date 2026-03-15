#include "edobj.h"
#include "../nu.h"

extern struct nuanimdata_s *ObjectAnim[64];
extern u8 object_switches[128];
extern const struct numtx_s numtx_identity;

s32 PlatImpactInfo(struct nuvec_s *norm, s32 *info, s32 *extra);
s32 LookupDebrisEffect(char *name);
void AddVariableShotDebrisEffectMtx2(s32 type, struct nuvec_s *pos, s32 numdeb, struct numtx_s *emitrotmtx, struct numtx_s *rotmtx);
void edbitsSoundPlay(struct nuvec_s *pos, s32 sid);
s32 edbitsLookupSoundFX(char *name);

//NGC MATCH
float edobjPlayerObjectDistance(s32 objid) {
  if (edmainQueryLocVec() != NULL) {
    return NuVecDist(&ObjectInstance[objid].mtx._30,edmainQueryLocVec(),NULL);
  }
  else {
    return 0.0f;
  }
}

void edobjUpdateObjects(float frametime) {
    static float localframecount;
    struct nuvec_s norm;
    struct nuanimtime_s atime;
    struct numtx_s animtx;
    struct numtx_s emitrotmtx;
    struct numtx_s rotmtx;
    s32 platinfo;
    s32 platextra;
    s32 platid;
    struct nuvec_s *pnorm;
    s32 i;
    s32 j;
    s32 k;
    struct nuinstance_s *instance;
    struct nuinstanim_s *instanim;
    struct nuanimdata_s *animdata;
    float ltime;
    float totaltime;
    s32 pathoffset;

    platid = PlatImpactInfo(&norm, &platinfo, &platextra);
    pnorm = &norm;

    for (i = 0; i < 64; i++) {
        instance = &ObjectInstance[i];
        ltime = 0.0f;

        if (instance->objid == -1) goto next_object;

        instanim = instance->anim;
        NuMtxSetIdentity(&instance->mtx);
        totaltime = frametime;

        pathoffset = i * sizeof(struct object_path_s);

        instance->mtx._30 = ObjectPath[i].waypoint[0].x;
        instance->mtx._31 = ObjectPath[i].waypoint[0].y;
        instance->mtx._32 = ObjectPath[i].waypoint[0].z;

        if (instanim == NULL) goto skip_anim;
        if (ObjectAnim[instanim->anim_ix] == NULL) goto skip_anim;

        /* trigger type switch */
        switch (ObjectPath[i].trigger_type) {
        case 0:
            instanim->ipad[0] = instanim->ipad[0] | 0x80000000;
            break;
        case 1:
            instanim->ipad[0] = instanim->ipad[0] & 0x7FFFFFFF;
            if (ObjectPath[i].trigger_id == -1) goto trigger_done;
            if (object_switches[ObjectPath[i].trigger_id] == 0) goto trigger_done;
            instanim->ipad[0] = instanim->ipad[0] | 0x80000000;
            break;
        case 2:
            instanim->ipad[0] = instanim->ipad[0] & ~0x10000000;
            if (ObjectPath[i].trigger_id == -1) goto trigger_done;
            if (object_switches[ObjectPath[i].trigger_id] == 0) goto trigger_done;
            if (instanim->ipad[0] < 0) goto trigger_done;
            instanim->ipad[0] = instanim->ipad[0] | 0x80000000;
            instanim->ltime = 0.0f;
            instanim->ipad[0] = instanim->ipad[0] & ~0x40000000;
            break;
        case 3:
            instanim->ipad[0] = instanim->ipad[0] | 0x10000000;
            if (ObjectPath[i].trigger_id == -1) goto trigger_done;
            if (object_switches[ObjectPath[i].trigger_id] == 0) goto trigger_done;
            instanim->ipad[0] = instanim->ipad[0] | 0x90000000;
            break;
        case 4:
            if (edobjPlayerObjectDistance(i) < ObjectPath[i].trigger_var) {
                instanim->ipad[0] = instanim->ipad[0] | 0x80000000;
            } else {
                instanim->ipad[0] = instanim->ipad[0] & 0x7FFFFFFF;
            }
            break;
        case 5:
            instanim->ipad[0] = instanim->ipad[0] & ~0x10000000;
            if (edobjPlayerObjectDistance(i) < ObjectPath[i].trigger_var) {
                if (instanim->ipad[0] < 0) goto trigger_done;
                instanim->ipad[0] = instanim->ipad[0] | 0x80000000;
                instanim->ltime = 0.0f;
                instanim->ipad[0] = instanim->ipad[0] & ~0x40000000;
            }
            break;
        case 6:
            instanim->ipad[0] = instanim->ipad[0] | 0x10000000;
            if (edobjPlayerObjectDistance(i) < ObjectPath[i].trigger_var) {
                instanim->ipad[0] = instanim->ipad[0] | 0x80000000;
            }
            break;
        case 7:
            if (ObjectPath[i].terrplatid == platid) {
                ObjectPath[i].trigger_var = 10.0f;
            }
            if (ObjectPath[i].trigger_var > 0.0f) {
                ObjectPath[i].trigger_var -= frametime;
                instanim->ipad[0] = instanim->ipad[0] | 0x80000000;
            } else {
                instanim->ipad[0] = instanim->ipad[0] & 0x7FFFFFFF;
            }
            break;
        case 8:
            instanim->ipad[0] = instanim->ipad[0] & ~0x10000000;
            if (ObjectPath[i].terrplatid == platid) {
                if (instanim->ipad[0] < 0) goto trigger_done;
                instanim->ipad[0] = instanim->ipad[0] | 0x80000000;
                instanim->ltime = 0.0f;
                instanim->ipad[0] = instanim->ipad[0] & ~0x20000000;
                if (ObjectPath[i].trigger_wait > 0.0f) {
                    instanim->tfirst = ObjectPath[i].trigger_wait + ltime;
                    instanim->ipad[0] = instanim->ipad[0] | 0x20000000;
                }
                instanim->ipad[0] = instanim->ipad[0] & ~0x40000000;
            }
            break;
        case 9:
            instanim->ipad[0] = instanim->ipad[0] | 0x10000000;
            if (ObjectPath[i].terrplatid == platid) {
                instanim->ipad[0] = instanim->ipad[0] | 0x90000000;
            }
            break;
        }
trigger_done:
        /* animation playback */
        animdata = ObjectAnim[instanim->anim_ix];
        if (animdata == NULL || instanim->ipad[0] >= 0) goto get_ltime;

        instanim->ltime = instanim->ltime + frametime * instanim->tfactor;

        if (instanim->ipad[0] & 0x20000000) {
            if (instanim->ltime >= instanim->tfirst) {
                instanim->tfirst -= 1.0f;
                instanim->ltime -= instanim->tfirst;
                instanim->ipad[0] = instanim->ipad[0] & ~0x20000000;
            }
            if (instanim->ipad[0] & 0x20000000) goto anim_calc;
        }

        if (instanim->ltime >= animdata->time + instanim->tinterval) {
            if (instanim->ipad[0] & 0x10000000) {
                if (instanim->ipad[0] & 0x08000000) {
                    if (instanim->ipad[0] & 0x40000000) {
                        instanim->ipad[0] = (instanim->ipad[0] & ~0x40000000) | ((!(instanim->ipad[0] & 0x40000000)) << 30);
                    }
                    instanim->ltime = 1.0f;
                } else {
                    /* check non-oscillate, non-repeat end */
                    instanim->ipad[0] = instanim->ipad[0] & 0x7FFFFFFF;
                    if (ObjectPath[i].trigger_id != -1) {
                        object_switches[ObjectPath[i].trigger_id] = 0;
                    }
                }
            } else {
                /* auto-repeat with osc */
                if ((instanim->ipad[0] & 0x48000000) == 0x08000000) {
                    instanim->ltime = 1.0f;
                    instanim->ipad[0] = (instanim->ipad[0] & ~0x40000000) | ((!(instanim->ipad[0] & 0x40000000)) << 30);
                }
            }
        }

anim_calc:
        ltime = instanim->ltime;
        if (ltime > animdata->time) {
            ltime = animdata->time;
        }
get_ltime:
        if (instanim == NULL) ltime = instanim->ltime;

        if (instanim->ipad[0] & 0x40000000) {
            ltime = animdata->time + 1.0f - ltime;
        }

        NuAnimDataCalcTime(animdata, ltime, &atime);
        totaltime = frametime;
        NuAnimCurveSetApplyToMatrix(*animdata->chunks[atime.chunk]->animcurvesets, &atime, &animtx);

        /* copy matrix */
        {
            struct numtx_s *src = &animtx;
            struct numtx_s *dst = (struct numtx_s *)instanim;
            struct nuvec_s *pos = (struct nuvec_s *)&instance->mtx._30;
            s32 cnt = 0x30;
            do {
                cnt -= 0x18;
                ((u32 *)dst)[0] = ((u32 *)src)[0];
                ((u32 *)dst)[1] = ((u32 *)src)[1];
                ((u32 *)dst)[2] = ((u32 *)src)[2];
                ((u32 *)dst)[3] = ((u32 *)src)[3];
                ((u32 *)dst)[4] = ((u32 *)src)[4];
                ((u32 *)dst)[5] = ((u32 *)src)[5];
                src = (struct numtx_s *)((char *)src + 0x18);
                dst = (struct numtx_s *)((char *)dst + 0x18);
            } while (cnt != 0);
            ((u32 *)dst)[0] = ((u32 *)src)[0];
            ((u32 *)dst)[1] = ((u32 *)src)[1];
            ((u32 *)dst)[2] = ((u32 *)src)[2];
            ((u32 *)dst)[3] = ((u32 *)src)[3];
        }
        NuMtxTranslate((struct Mtx *)instanim, (struct nuvec_s *)&instance->mtx._30);

skip_anim:
        /* particle loop */
        for (j = (s32)totaltime, k = 0; k < ObjectPath[i].usedpart; k++) {
            if (ObjectPath[i].particle_type[k] == -1) {
                /* particle_name lookup */
                if (ObjectPath[i].particle_name[k][0] != 0) {
                    ObjectPath[i].particle_type[k] = LookupDebrisEffect(ObjectPath[i].particle_name[k]);
                    if (ObjectPath[i].particle_type[k] == -1) {
                        edobjParticleDestroy(i, k);
                        k--;
                        continue;
                    }
                }
            } else {
                /* check if particle_switch enabled */
                if (ObjectPath[i].particle_switch[k] != 0) {
                    if (instanim == NULL) goto particle_next;
                    if (instanim->ipad[0] >= 0) goto particle_next;
                    if (instanim->ltime >= ObjectAnim[instanim->anim_ix]->time) goto particle_next;
                }

                {
                    struct nuinstanim_s *mtxsrc;

                    if (instanim != NULL) {
                        mtxsrc = instanim;
                    } else {
                        mtxsrc = (struct nuinstanim_s *)instance;
                    }

                    if ((edobj_particle_mode != 0 || edobj_sound_mode != 0) && edobj_nearest == i) {
                        mtxsrc = (struct nuinstanim_s *)instance;
                    }

                    NuVecMtxTransform(&norm, &ObjectPath[i].particle_offset[k], (struct Mtx *)mtxsrc);

                    emitrotmtx = numtx_identity;

                    NuMtxRotateZ((struct Mtx *)&emitrotmtx, (s32)ObjectPath[i].particle_emitrotz[k]);
                    NuMtxRotateY((struct Mtx *)&emitrotmtx, (s32)ObjectPath[i].particle_emitroty[k]);
                    NuMtxMul((struct Mtx *)&rotmtx, (struct Mtx *)&emitrotmtx, (struct Mtx *)mtxsrc);

                    if (ObjectPath[i].particle_rate[k] > 0) {
                        AddVariableShotDebrisEffectMtx2(ObjectPath[i].particle_type[k], &norm, ObjectPath[i].particle_rate[k], &rotmtx, (struct numtx_s *)&numtx_identity);
                    } else if (ObjectPath[i].particle_rate[k] < 0) {
                        s32 rate = ObjectPath[i].particle_rate[k];
                        s32 absrate;
                        s32 rem;
                        s32 frame;
                        s32 prevframe;

                        absrate = rate < 0 ? -rate : rate;
                        frame = (s32)(localframecount + frametime);
                        prevframe = (s32)localframecount;
                        frame = frame / absrate;
                        prevframe = prevframe / absrate;
                        if (frame != prevframe) {
                            AddVariableShotDebrisEffectMtx2(ObjectPath[i].particle_type[k], &norm, 1, &rotmtx, (struct numtx_s *)&numtx_identity);
                        }
                    }
                }
            }
particle_next:
            ;
        }

        /* sound loop */
        for (j = 0; j < ObjectPath[i].usedsound; j++) {
            if (ObjectPath[i].sound_id[j] == -1) {
                if (ObjectPath[i].sound_name[j][0] != 0) {
                    ObjectPath[i].sound_id[j] = edbitsLookupSoundFX(ObjectPath[i].sound_name[j]);
                    if (ObjectPath[i].sound_id[j] == -1) {
                        edobjSoundDestroy(i, j);
                        j--;
                        continue;
                    }
                }
            } else {
                s32 sound_play = -1;

                if (ObjectPath[i].sound_type[j] == 1) {
                    s32 frame_next = (s32)(localframecount + frametime);
                    s32 period = (s32)ObjectPath[i].sound_time[j];
                    s32 frame_cur = (s32)localframecount;
                    if (frame_next / period > frame_cur / period) {
                        sound_play = ObjectPath[i].sound_id[j];
                    }
                } else {
                    if (ltime >= ObjectPath[i].sound_time[j]) {
                        if (!(ltime > ObjectPath[i].sound_last_time)) goto sound_check_repeat;
                        sound_play = ObjectPath[i].sound_id[j];
                    }
sound_check_repeat:
                    if (ObjectPath[i].oscillate != 0) {
                        float prevtime = ObjectPath[i].sound_last_time + 0.0f;  // just load
                        if (ltime <= prevtime) {
                            float abstime;
                            abstime = (float)((s32)ObjectPath[i].sound_last_time < 0 ? -(s32)ObjectPath[i].sound_last_time : (s32)ObjectPath[i].sound_last_time);
                            if (prevtime < abstime) {
                                sound_play = ObjectPath[i].sound_id[j + 0]; // ???
                            }
                        }
                    }
                }

                if (sound_play != -1) {
                    struct nuinstanim_s *mtxsrc;

                    if (instanim != NULL) {
                        mtxsrc = instanim;
                    } else {
                        mtxsrc = (struct nuinstanim_s *)instance;
                    }

                    if ((edobj_particle_mode != 0 || edobj_sound_mode != 0) && edobj_nearest == i) {
                        mtxsrc = (struct nuinstanim_s *)instance;
                    }

                    NuVecMtxTransform(&norm, &ObjectPath[i].sound_offset[j], (struct Mtx *)mtxsrc);
                    edbitsSoundPlay(&norm, sound_play);
                }
            }
        }

        if (ObjectPath[i].oscillate != 0) {
            ObjectPath[i].sound_last_time = -ltime;
        } else {
            ObjectPath[i].sound_last_time = ltime;
        }

next_object:
        ;
    }
    localframecount += frametime;
}

//NGC MATCH
s32 edobjRenderCutoffTest(struct nuvec_s *pos) {
  float x;
  float y;
  float z;
  
  x = pos->x - global_camera.mtx._30;
  y = pos->y - global_camera.mtx._31;
  z = pos->z - global_camera.mtx._32;
  if (NuFsqrt(x * x + y * y + z * z) > 80.0f) {
      return 0;
  }
  return 1;
}

//NGC MATCH
void edobjRenderObjects(struct nugscn_s *scn) {
    struct nuinstance_s *instance;
    s32 i;
    struct nuinstanim_s *instanim;

    instance = ObjectInstance;
    
    for (i = 0; i < 0x40; i++) {
        instance = &ObjectInstance[i];
        if (instance->objid != -1) {
            instanim = instance->anim;
            if (((edobj_particle_mode != 0) || (edobj_sound_mode != 0)) && (edobj_nearest == i)) {
                instance->flags.onscreen = NuRndrGScnObj(scn->gobjs[instance->objid],&instance->mtx);
            }
            else if (instanim != NULL) {
                if (edobjRenderCutoffTest((struct nuvec_s *)&(instanim->mtx)._30) != 0) {
                    instance->flags.onscreen = NuRndrGScnObj(scn->gobjs[instance->objid],&instanim->mtx);
                }
            }
            else {
                if (edobjRenderCutoffTest((struct nuvec_s *)&ObjectInstance[i].mtx._30) != 0) {
                    instance->flags.onscreen = NuRndrGScnObj(scn->gobjs[instance->objid], &instance->mtx);
                }
            }
        }
    }
}

//NGC MATCH
void edobjResetAnimsToZero(void) {
  s32 i;
  struct nuinstance_s *instance;
  struct nuinstanim_s *instanim;
  
  instance = ObjectInstance;
  for(i = 0; i < 0x40; i++) {
    ObjectPath[i].particle_type[0] = -1;
    if (instance[i].objid != -1) {
    instanim = instance[i].anim;
        if(instanim != NULL) {
          instanim->tfirst = 0.0f;
          instanim->waiting = 1;
          instanim->ltime = ObjectPath[i].start_offset;
          instanim->playing = 0;
          instanim->backwards = 0;
        }
    }
  }
  return;
}

//NGC MATCH
s32 edobjLookupInstanceIndex(s32 specialid) {
  s32 i;
  
  if (edobj_base_scene == NULL) {
    return -1;
  }
    for(i = 0; i < edobj_base_scene->numinstance; i++) {
      if (&edobj_base_scene->instances[i] == edobj_base_scene->specials[specialid].instance) {
        return i;
      }
    }
  return -1;
}

//NGC MATCH
void edobjRegisterBaseScene(struct nugscn_s *scn) {
  edobj_base_scene = scn;
  return;
}

//NGC MATCH
void edobjDetermineNearestObject(float ndist) {
  s32 i;
  float dist;
  struct nuvec_s distv;
  
    if ((edobj_nearest != -1)) {
        NuVecSub(&distv,&edobj_cam_pos,(struct nuvec_s *)&ObjectInstance[edobj_nearest].mtx._30);
        dist = (distv.x * distv.x + distv.y * distv.y + distv.z * distv.z);
        if (dist == 0.0) return;
    }
    edobj_nearest = -1;
    for(i = 0; i < 0x40; i++) {
      if (ObjectInstance[i].objid != -1) {
        NuVecSub(&distv,&edobj_cam_pos,(struct nuvec_s *)&ObjectInstance[i].mtx._30);
        dist = distv.x * distv.x + distv.y * distv.y + distv.z * distv.z;
        if ((ndist < 0.0f) || (dist < ndist)) {
          ndist = dist;
          edobj_nearest = i;
        }
      }
    }
  return;
}

//NGC MATCH
u32 reverse_endian_32(u32 arg0) {
    return (arg0 >> 0x18U) | ((arg0 >> 8U) & 0xFF00) | ((arg0 << 8) & 0xFF0000) | (arg0 << 0x18);
}

//NGC MATCH
void edobjSoundDestroy(s32 obj,s32 sound) {
  s32 i;

    for(i = sound; i < (s32)ObjectPath[obj].usedsound + -1; i++) {
      ObjectPath[obj].sound_id[i] = ObjectPath[obj].sound_id[i + 1];
      ObjectPath[obj].sound_time[i] = ObjectPath[obj].sound_time[i + 1];
      ObjectPath[obj].sound_offset[i] = ObjectPath[obj].sound_offset[i + 1];
      strcpy(ObjectPath[obj].sound_name[i],ObjectPath[obj].sound_name[i + 1]);
    }
  ObjectPath[obj].usedsound--;
  return;
}

//NGC MATCH
void edobjParticleDestroy(s32 obj,s32 ptl) {
  s32 i;
  
    for(i = ptl; i < ObjectPath[obj].usedpart - 1; i++) {
      ObjectPath[obj].particle_type[i] = ObjectPath[obj].particle_type[i + 1];
      ObjectPath[obj].particle_rate[i] = ObjectPath[obj].particle_rate[i + 1];
      ObjectPath[obj].particle_switch[i] = ObjectPath[obj].particle_switch[i + 1];
      ObjectPath[obj].particle_offset[i] = ObjectPath[obj].particle_offset[i + 1];
      strcpy(ObjectPath[obj].particle_name[i],ObjectPath[obj].particle_name[i + 1]);
    }
  ObjectPath[obj].usedpart--;
  return;
}

//NGC MATCH
s32 edobjFileLoadObjects(char *file) {
  s32 i;
  s32 j;
  s32 version;
  s32 numobjects;
  char tname[20];
  s32 dummyreads;
  struct nuvec_s dummyvec;
  char dummyname[16];
  
  if (EdFileOpen(file,NUFILE_READ) != 0) {
    version = EdFileReadInt();
    if (version > 0xd) {
        EdFileClose();
        return 0;
    } 
  } else {
      return 0;
  }
    numobjects = EdFileReadInt();
    for(i = 0; i < numobjects; i++) {
          if (version == 1) {
            ObjectPath[i].objid = EdFileReadInt();
          }
          else {
            EdFileRead(tname,0x14);
            ObjectPath[i].objid = edobjLookupInstance(tname);
          }
          j = i * 0x3ec;
          if (ObjectPath[i].objid != -1) {
            ObjectInstance[i].objid = (edobj_base_scene->specials[ObjectPath[i].objid].instance)->objid;
          }
          else {
            ObjectInstance[i].objid = -1;
          }
          ObjectPath[i].speed = EdFileReadFloat();
          ObjectPath[i].oscillate = EdFileReadInt();
          if (version > 5) {
            ObjectPath[i].repeat = EdFileReadInt();
          }
          else {
            ObjectPath[i].repeat = 1;
          }
          ObjectPath[i].pause = EdFileReadFloat();
          ObjectPath[i].usedway = EdFileReadInt();
          if (version > 8) {
            ObjectPath[i].usedpart = EdFileReadInt();
          }
          else {
            ObjectPath[i].usedpart = 0;
          }
          if (version > 9) {
            ObjectPath[i].usedsound = EdFileReadInt();
          }
          else {
            ObjectPath[i].usedsound = 0;
          }
          ObjectPath[i].terrplatid = -1;
          if (version > 5) {
            ObjectPath[i].trigger_type = EdFileReadInt();
            ObjectPath[i].trigger_id = EdFileReadInt();
            ObjectPath[i].trigger_var = EdFileReadFloat();
          }
          else {
            ObjectPath[i].trigger_type = 0;
            ObjectPath[i].trigger_id = -1;
            ObjectPath[i].trigger_var = 0.0f;
          }
          if (version > 7) {
            ObjectPath[i].trigger_wait = EdFileReadFloat();
          }
          else {
            ObjectPath[i].trigger_wait = 0.0f;
          }
          if (version > 8) {
                dummyreads = 0;
                if (8 < ObjectPath[i].usedpart) {
                  dummyreads = 8 - ObjectPath[i].usedpart;
                  ObjectPath[i].usedpart = 8;
                }
                for(j = 0; j < ObjectPath[i].usedpart; j++) {
                    EdFileRead(ObjectPath[i].particle_name[j],0x10);
                    ObjectPath[i].particle_type[j] = -1;
                    ObjectPath[i].particle_rate[j] = EdFileReadInt();
                    ObjectPath[i].particle_switch[j] = EdFileReadInt();
                    EdFileRead(&ObjectPath[i].particle_offset[j],0xc);
                    if (version > 0xc) {
                      ObjectPath[i].particle_emitrotz[j] = EdFileReadShort();
                      ObjectPath[i].particle_emitroty[j] = EdFileReadShort();
                    }
                    else {
                      ObjectPath[i].particle_emitrotz[j] = 0;
                      ObjectPath[i].particle_emitroty[j] = 0;
                    }
                }
                for(j = 0; j < dummyreads; j++) {
                    EdFileRead(dummyname,0x10);
                    EdFileReadInt();
                    EdFileReadInt();
                    EdFileRead(&dummyvec,0xc);
                    if (version > 0xc) {
                      EdFileReadShort();
                      EdFileReadShort();
                    }
                }
          }
          else if (version > 6) {
              EdFileRead(ObjectPath[i].particle_name,0x10);
              ObjectPath[i].particle_type[0] = -1;
              ObjectPath[i].particle_rate[0] = EdFileReadInt();
              ObjectPath[i].particle_switch[0] = EdFileReadInt();
              if (ObjectPath[i].particle_name[0][0] != '\0') {
                ObjectPath[i].usedpart = 1;
              }
          }
          if (version > 4) {
            ObjectPath[i].start_offset = EdFileReadInt();
          }
          else {
            ObjectPath[i].start_offset = 0;
          }
          dummyreads = 0;
          if (8 < ObjectPath[i].usedway) {
            dummyreads = 8 - ObjectPath[i].usedway;
            ObjectPath[i].usedway = 8;
          }
          ObjectPath[i].usedtime = 0;
          for(j = 0; j < ObjectPath[i].usedway; j++) {
              EdFileRead(&ObjectPath[i].waypoint[j],0xc);
              ObjectPath[i].waypoint_speed[j] = EdFileReadFloat();
              if (version > 2) {
                EdFileRead(&ObjectPath[i].waypoint_rot[j],0xc);
              }
              else {
                memset(&ObjectPath[i].waypoint_rot[j],0,0xc);
              }
              if (version > 3) {
                ObjectPath[i].waypoint_time[j] = EdFileReadInt();
              }
              else {
                if (j < ObjectPath[i].usedway - 1) {
                  ObjectPath[i].waypoint_time[j] = 2;
                }
                else {
                  ObjectPath[i].waypoint_time[j] = 0;
                }
              }
              ObjectPath[i].usedtime += ObjectPath[i].waypoint_time[j];
          }
          for(j = 0; j < dummyreads; j++) {
              EdFileRead(&dummyvec,0xc);
              EdFileReadFloat();
              if (2 < version) {
                EdFileRead(&dummyvec,0xc);
              }
              if (3 < version) {
                EdFileReadInt();
              }
          }
          dummyreads = 0;
          ObjectPath[i].sound_last_time = 0.99f;
          if (8 < ObjectPath[i].usedsound) {
            dummyreads = 8 - ObjectPath[i].usedsound;
            ObjectPath[i].usedsound = 8;
          }
          if (version > 10) {
            for(j = 0; j < ObjectPath[i].usedsound; j++) {
                EdFileRead(ObjectPath[i].sound_name[j],0x10);
                ObjectPath[i].sound_id[j] = -1;
                ObjectPath[i].sound_type[j] = EdFileReadInt();
                ObjectPath[i].sound_time[j] = EdFileReadFloat();
                EdFileRead(&ObjectPath[i].sound_offset[j],0xc);
            }
            for(j = 0; j < dummyreads; j++) {
                EdFileRead(dummyname,0x10);
                EdFileReadInt();
                EdFileReadFloat();
                EdFileRead(&dummyvec,0xc);
            }
          }
          else if (version == 10) {
            for(j = 0; j < ObjectPath[i].usedsound; j++) {
                ObjectPath[i].sound_id[j] = EdFileReadFloat();
                ObjectPath[i].sound_type[j] = 1;
                ObjectPath[i].sound_time[j] = EdFileReadFloat();
                EdFileRead(&ObjectPath[i].sound_offset[j],0xc);
            }
            for(j = 0; j < dummyreads; j++) {
                EdFileReadFloat();
                EdFileReadFloat();
                EdFileRead(&dummyvec,0xc);
            }
          }
          if (version > 0xb) {
            ObjectPath[i].playergrav = EdFileReadFloat();
            ObjectPath[i].tension = EdFileReadFloat();
            ObjectPath[i].damping = EdFileReadFloat();
          }
          else {
            ObjectPath[i].playergrav = 0.0f;
            ObjectPath[i].tension = 0.0f;
            ObjectPath[i].damping = 0.0f;
          }
      }
      EdFileClose();
      for(i = 0; i < numobjects; i++) {
          if (ObjectPath[i].objid != -1) {
            edobjConvertPathToAnim(i);
          }
      }
      edobjResetAnimsToZero();
      edobj_next_instance = numobjects;
      edobj_waypoint_mode = 0;
      edobj_copy_mode = 0;
      edobj_particle_mode = 0;
      edobj_sound_mode = 0;
      edobj_nearest = -1;
      edobjDetermineNearestObject(1.0f);
      return 1;
}

//NGC MATCH
void edobjObjectReset(void) {
  s32 i;

  for(i = 0; i < 64; i++) {
    ObjectInstance[i].objid = -1;
    ObjectPath[i].objid = -1;
  }
  edobj_next_instance = 0.0f;
  return;
}

//NGC MATCH
s32 edobjLookupInstance(char *name) {
  s32 i;
  
  if (edobj_base_scene != NULL) {
    for(i = 0; i < edobj_base_scene->numspecial; i++) {
        if (strncmp(edobj_base_scene->specials[i].name,name,0x13) == 0) {
            return i;
        }
    }
  }
  return -1;
}