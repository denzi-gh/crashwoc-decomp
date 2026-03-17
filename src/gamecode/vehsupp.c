#include "main.h"

double pow(double, double);
double fmod(double, double);
int rand(void);

void PointAlongSpline(struct nugspline_s *spl, float ratio, struct nuvec_s *dst, u16 *angle, u16 *tilt);
void SetNearestLights(struct Nearest_Light_s *l);
void UpdateAnimPacket(struct CharacterModel *mod, struct anim_s *anim, float dt, float xz_distance);
s32 DrawCharacterModel(struct CharacterModel *model, struct anim_s *anim, struct numtx_s *mC,
                       struct numtx_s *mS, s32 render, struct numtx_s *mR,
                       struct numtx_s *loc_mtx, struct nuvec_s *loc_mom, struct obj_s *obj);

extern s32 Level;
extern s32 gamesfx_effect_volume;
extern s32 gamesfx_pitch;
extern s32 Paused;
extern s32 ChrisJointOveride;
extern s32 ChrisNumJoints;
extern struct NUJOINTANIM_s *ChrisJointList;
extern f32 NuTrigTable[];

struct MYDRAW {
    struct anim_s Anim;
    struct CharacterModel *Model;
    s32 Character;
    s32 NumJoints;
    struct NUJOINTANIM_s *JointList;
    struct nuvec_s *Position;
    struct Nearest_Light_s Nearest_Light;
};

struct MYSPLINE {
    struct nugspline_s *Spline;
    f32 Cur;
    f32 Nex;
    f32 Act;
    f32 Inc;
    struct nuvec_s CurPos;
    struct nuvec_s NexPos;
    f32 LookaheadDist;
};

float Rationalise360f(float a);

s16 GetVolumeI(float vol) {
    s32 v = (s32)(vol * 16383.0f);
    if (v > 0x7FFF) {
        v = 0x7FFF;
    }
    return (s16)v;
}

void MyGameSfx(s32 Id, struct nuvec_s *Pos, s32 Vol) {
    if (Level == 0x15 && Pos != NULL) {
        Vol = Vol * 4 + Vol;
    }
    gamesfx_effect_volume = Vol;
    GameSfx(Id, Pos);
}

void MyGameSfxLoop(s32 Id, struct nuvec_s *Pos, s32 Vol) {
    if (Level == 0x15 && Pos != NULL) {
        Vol = Vol * 4 + Vol;
    }
    gamesfx_effect_volume = Vol;
    GameSfxLoop(Id, Pos);
}

void MyGameSfxLoopVolPitch(s32 Id, struct nuvec_s *Pos, s32 Vol, s32 Pitch) {
    if (Level == 0x15 && Pos != NULL) {
        Vol = Vol * 4 + Vol;
    }
    gamesfx_effect_volume = Vol;
    gamesfx_pitch = Pitch;
    GameSfxLoop(Id, Pos);
}

struct nuvec_s SetNuVec(f32 x, f32 y, f32 z) {
    struct nuvec_s v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

struct nuvec_s *SetNuVecPntr(f32 x, f32 y, f32 z) {
    static struct nuvec_s Ret;
    Ret.x = x;
    Ret.y = y;
    Ret.z = z;
    return &Ret;
}

struct nuvec_s *SetNuVecPntrA(f32 x, f32 y, f32 z) {
    static struct nuvec_s RetA;
    RetA.x = x;
    RetA.y = y;
    RetA.z = z;
    return &RetA;
}

struct Quat SetNuQuat(f32 x, f32 y, f32 z, f32 w) {
    struct Quat q;
    q.x = x;
    q.y = y;
    q.z = z;
    q.w = w;
    return q;
}

void GetLocatorMtx(struct CharacterModel *model, struct numtx_s *dest, float angle) {
    static struct numtx_s BaseMat;
    s32 i;
    struct numtx_s *d;

    if (model == NULL) {
        return;
    }

    NuMtxSetRotationY(&BaseMat, (s32)(angle * 182.04445f));
    NuHGobjEval(model->hobj, 0, 0, tmtx);

    d = dest;
    for (i = 0; i <= 15; i++) {
        if (model->pLOCATOR[i] != 0) {
            NuHGobjPOIMtx(model->hobj, (u8)i, &BaseMat, tmtx, d);
        }
        d++;
    }
}

void GetLocatorMtxMyDraw(struct MYDRAW *Draw, struct numtx_s *dest, struct numtx_s *baseMtx) {
    struct CharacterModel *model;
    s32 i;
    struct numtx_s *d;

    model = Draw->Model;
    if (model == NULL) {
        return;
    }

    NuHGobjEvalAnim(model->hobj, model->anmdata[Draw->Anim.action],
                    Draw->Anim.anim_time, 0, 0, tmtx);

    d = dest;
    for (i = 0; i <= 15; i++) {
        if (Draw->Model->pLOCATOR[i] != 0) {
            NuHGobjPOIMtx(Draw->Model->hobj, (u8)i, baseMtx, tmtx, d);
        }
        d++;
    }
}

void ApplyFriction(float *val, float rate, float dt) {
    float v = *val;
    float f = rate * dt;

    if (v > 0.0f) {
        if (f > v) {
            *val = 0.0f;
            return;
        }
        *val = v - f;
    } else {
        if (f > -v) {
            *val = 0.0f;
            return;
        }
        *val = v + f;
    }
}

void SeekHalfLife(float *dest, float target, float halflife, float dt) {
    float rate;

    if (halflife == 0.0f) {
        rate = 1.0f;
    } else {
        rate = 1.0f - 1.0f / (float)pow(2.0, dt / halflife);
    }
    *dest = (target - *dest) * rate + *dest;
}

void SeekHalfLifeLim(float *dest, float target, float limit, float halflife, float dt) {
    float rate;
    float change;
    float lim;
    float cur;

    if (halflife == 0.0f) {
        rate = 1.0f;
    } else {
        rate = 1.0f - 1.0f / (float)pow(2.0, dt / halflife);
    }

    cur = *dest;
    lim = limit * dt;
    change = (target - cur) * rate;

    if (change > lim) {
        change = lim;
    } else if (change < -lim) {
        change = -lim;
    }

    *dest = cur + change;
}

void SeekHalfLifeNUVEC(struct nuvec_s *src, struct nuvec_s *target, float halflife, float dt) {
    float rate;
    struct nuvec_s diff;

    if (halflife == 0.0f) {
        rate = 1.0f;
    } else {
        rate = 1.0f - 1.0f / (float)pow(2.0, dt / halflife);
    }

    diff.x = target->x - src->x;
    diff.y = target->y - src->y;
    diff.z = target->z - src->z;
    src->z = diff.z * rate + src->z;
    src->x = diff.x * rate + src->x;
    src->y = diff.y * rate + src->y;
}

void SeekAngHalfLife360f(float *dest, float target, float halflife, float dt) {
    float rate;
    float diff;

    if (halflife == 0.0f) {
        rate = 1.0f;
    } else {
        rate = 1.0f - 1.0f / (float)pow(2.0, dt / halflife);
    }

    diff = Rationalise360f(target - *dest);
    *dest = rate * diff + *dest;
}

void SeekAngLimHalfLife360f(float *dest, float target, float max_speed, float halflife, float dt) {
    float rate;
    float change;
    float lim;

    lim = max_speed * dt;

    if (halflife == 0.0f) {
        rate = 1.0f;
    } else {
        rate = 1.0f - 1.0f / (float)pow(2.0, dt / halflife);
    }

    change = rate * Rationalise360f(target - *dest);

    if (change > lim) {
        change = lim;
    }
    if (change < -lim) {
        change = -lim;
    }

    *dest += change;
}

void SeekAngHalfLife(u16 *dest, u16 target, float halflife, float dt) {
    float rate;
    float diff;
    float cur;
    float result;

    if (halflife == 0.0f) {
        rate = 1.0f;
    } else {
        rate = 1.0f - 1.0f / (float)pow(2.0, dt / halflife);
    }

    diff = (float)(s16)(target - *dest);
    cur = (float)(s16)*dest;
    result = rate * diff + cur;
    *dest = (u16)(s32)result;
}

float frand(void) {
    return (float)(rand() % 16384) * (1.0f / 16384.0f);
}

float frandPN(void) {
    return (float)(rand() % 16384) * (1.0f / 8192.0f) - 1.0f;
}

float fsign(float x) {
    if (x >= 0.0f) {
        return 1.0f;
    }
    return -1.0f;
}

float Rationalise360f(float a) {
    a = (float)fmod(a + 180.0f, 360.0) - 180.0f;
    if (a < -180.0f) {
        a += 360.0f;
    }
    return a;
}

s32 LimitAng360f(float *dest, float min, float max) {
    float a;
    s32 result = 0;

    a = (float)fmod(*dest + 180.0f, 360.0) - 180.0f;
    if (a < -180.0f) {
        a += 360.0f;
    }

    if (a < min) {
        a = min;
    } else if (a > max) {
        a = max;
    } else {
        result = 1;
    }

    *dest = a;
    return result;
}

float DotProduct(struct nuvec_s *A, struct nuvec_s *B) {
    return A->x * B->x + A->y * B->y + A->z * B->z;
}

void CrossProduct(struct nuvec_s *dest, struct nuvec_s *A, struct nuvec_s *B) {
    struct nuvec_s result;
    result.x = A->y * B->z - A->z * B->y;
    result.y = A->z * B->x - A->x * B->z;
    result.z = A->x * B->y - A->y * B->x;
    *dest = result;
}

s32 ProcessTimer(float *Timer) {
    if (Paused == 0) {
        *Timer -= 1.0f / 60.0f;
    }
    if (*Timer <= 0.0f) {
        *Timer = 0.0f;
        return 1;
    }
    return 0;
}

s32 MyInitModelNew(struct MYDRAW *Draw, s32 Character, s32 Action,
                   s32 NumJoints, struct NUJOINTANIM_s *JointList,
                   struct nuvec_s *Position) {
    if ((s8)CRemap[Character] == -1) {
        return 0;
    }

    Draw->Character = Character;
    Draw->NumJoints = NumJoints;
    Draw->JointList = JointList;
    Draw->Position = Position;
    Draw->Model = &CModel[(s8)CRemap[Character]];
    ResetAnimPacket(&Draw->Anim, Action);
    ResetLights(&Draw->Nearest_Light);
    return 1;
}

s32 MyDrawModelNew(struct MYDRAW *Draw, struct numtx_s *Mat, struct numtx_s *Locators) {
    if (Draw->Model == NULL) {
        return 0;
    }

    if (Draw->NumJoints != 0) {
        ChrisJointOveride = 1;
        ChrisNumJoints = Draw->NumJoints;
        ChrisJointList = Draw->JointList;
    }

    SetNearestLights(&Draw->Nearest_Light);
    DrawCharacterModel(Draw->Model, &Draw->Anim, Mat, NULL, 1, NULL, Locators, NULL, NULL);
    ChrisJointOveride = 0;
}

void MyAnimateModelNew(struct MYDRAW *Draw, float dt) {
    Draw->Anim.oldaction = Draw->Anim.action;
    UpdateAnimPacket(Draw->Model, &Draw->Anim, dt, 0.0f);
    GetLights(Draw->Position, &Draw->Nearest_Light, 1);
}

void MyResetAnimPacket(struct MYDRAW *Draw, s32 Action) {
    ResetAnimPacket(&Draw->Anim, Action);
    Draw->Anim.flags = 0;
}

void MyChangeAnim(struct MYDRAW *Draw, s32 Action) {
    Draw->Anim.flags = 0;
    Draw->Anim.oldaction = Draw->Anim.action;
    Draw->Anim.newaction = (s16)Action;
}

float ControlledDist(struct nuvec_s *A, struct nuvec_s *B, s32 control) {
    struct nuvec_s diff;
    float dist = 0.0f;
    float *p;
    s32 i;

    NuVecSub(&diff, A, B);

    p = &diff.x;
    for (i = 0; i < 3; i++) {
        float v = *p;
        if (v > 0.0f) {
            if (control & 1) {
                dist += v * v;
            }
        } else if (v < 0.0f) {
            if (control & 2) {
                dist += v * v;
            }
        }
        p++;
        control >>= 4;
    }

    return NuFsqrt(dist);
}

void FindSplineTargetPoint(struct MYSPLINE *Spline, s32 Control,
                           struct nuvec_s *Point, struct nuvec_s *Direction,
                           s32 Wrap, s32 BigLook) {
    float dist_cur, dist_nex;
    struct nuvec_s temp;

    if (Wrap == 0) {
        if (Spline->Inc > 0.0f) {
            if (Spline->Cur > Spline->Nex) {
                float tc = Spline->Cur;
                float tn = Spline->Nex;
                struct nuvec_s tp = Spline->CurPos;
                Spline->Cur = tn;
                Spline->Nex = tc;
                Spline->CurPos = Spline->NexPos;
                Spline->NexPos = tp;
            }
        } else if (Spline->Inc < 0.0f) {
            if (Spline->Cur < Spline->Nex) {
                float tc = Spline->Cur;
                float tn = Spline->Nex;
                struct nuvec_s tp = Spline->CurPos;
                Spline->Cur = tn;
                Spline->Nex = tc;
                Spline->CurPos = Spline->NexPos;
                Spline->NexPos = tp;
            }
        }
    }

    dist_cur = ControlledDist(&Spline->CurPos, Point, Control);
    dist_nex = ControlledDist(&Spline->NexPos, Point, Control);

    while (dist_nex < dist_cur) {
        dist_cur = dist_nex;
        Spline->CurPos = Spline->NexPos;
        Spline->Cur = Spline->Nex;
        Spline->Nex = Spline->Nex + Spline->Inc;

        if (Spline->Nex > 1.0f) {
            if (Wrap) {
                Spline->Nex -= 1.0f;
            } else {
                Spline->Nex = 1.0f;
            }
        } else if (Spline->Nex < 0.0f) {
            if (Wrap) {
                Spline->Nex += 1.0f;
            } else {
                Spline->Nex = 0.0f;
            }
        }

        PointAlongSpline(Spline->Spline, Spline->Nex, &Spline->NexPos, NULL, NULL);
        dist_nex = ControlledDist(&Spline->NexPos, Point, Control);
    }

    if (dist_nex < Spline->LookaheadDist) {
        goto keep_seeking;
    }
    if (dist_cur <= Spline->LookaheadDist) {
        goto done;
    }
    if (BigLook == 0) {
        goto done;
    }

keep_seeking:
    if (Spline->Nex == 1.0f && Wrap == 0) {
        goto done;
    }

    dist_cur = dist_nex;
    Spline->CurPos = Spline->NexPos;
    Spline->Cur = Spline->Nex;
    Spline->Nex = Spline->Nex + Spline->Inc;

    if (Spline->Nex > 1.0f) {
        if (Wrap) {
            Spline->Nex -= 1.0f;
        } else {
            Spline->Nex = 1.0f;
        }
    } else if (Spline->Nex < 0.0f) {
        if (Wrap) {
            Spline->Nex += 1.0f;
        } else {
            Spline->Nex = 0.0f;
        }
    }

    PointAlongSpline(Spline->Spline, Spline->Nex, &Spline->NexPos, NULL, NULL);
    dist_nex = ControlledDist(&Spline->NexPos, Point, Control);

    while (dist_nex < dist_cur) {
        dist_cur = dist_nex;
        Spline->CurPos = Spline->NexPos;
        Spline->Cur = Spline->Nex;
        Spline->Nex = Spline->Nex + Spline->Inc;

        if (Spline->Nex > 1.0f) {
            if (Wrap) {
                Spline->Nex -= 1.0f;
            } else {
                Spline->Nex = 1.0f;
            }
        } else if (Spline->Nex < 0.0f) {
            if (Wrap) {
                Spline->Nex += 1.0f;
            } else {
                Spline->Nex = 0.0f;
            }
        }

        PointAlongSpline(Spline->Spline, Spline->Nex, &Spline->NexPos, NULL, NULL);
        dist_nex = ControlledDist(&Spline->NexPos, Point, Control);
    }

done:
    NuVecSub(&temp, &Spline->NexPos, &Spline->CurPos);

    {
        float ratio;
        if (dist_nex <= Spline->LookaheadDist || dist_nex <= dist_cur) {
            ratio = 1.0f;
        } else if (dist_cur >= Spline->LookaheadDist) {
            ratio = 0.0f;
        } else {
            ratio = (Spline->LookaheadDist - dist_cur) / (dist_nex - dist_cur);
        }

        Spline->Act = ratio * Spline->Inc + Spline->Cur;

        if (Direction != NULL) {
            *Direction = Spline->CurPos;
            NuVecScaleAccum(ratio, Direction, &temp);
        }
    }
}

float FindSplineClosestPointAndDist(struct MYSPLINE *Spline, s32 Control,
                                    struct nuvec_s *Point,
                                    struct nuvec_s *TargetPoint,
                                    s32 Wrap, s32 BigLook) {
    float dist_cur, dist_nex;

    dist_cur = ControlledDist(&Spline->CurPos, Point, Control);
    dist_nex = ControlledDist(&Spline->NexPos, Point, Control);

    while (1) {
        if (dist_nex < dist_cur || Spline->Cur == Spline->Nex) {
            if (Spline->Nex == 1.0f && Wrap == 0) {
                break;
            }
        } else {
            break;
        }

        dist_cur = dist_nex;
        Spline->CurPos = Spline->NexPos;
        Spline->Cur = Spline->Nex;
        Spline->Nex = Spline->Nex + Spline->Inc;

        if (Spline->Nex > 1.0f) {
            if (Wrap) {
                Spline->Nex -= 1.0f;
            } else {
                Spline->Nex = 1.0f;
            }
        } else if (Spline->Nex < 0.0f) {
            if (Wrap) {
                Spline->Nex += 1.0f;
            } else {
                Spline->Nex = 0.0f;
            }
        }

        PointAlongSpline(Spline->Spline, Spline->Nex, &Spline->NexPos, NULL, NULL);
        dist_nex = ControlledDist(&Spline->NexPos, Point, Control);
    }

    if (TargetPoint != NULL) {
        *TargetPoint = Spline->CurPos;
    }

    return dist_cur;
}

float ASin360f(float val) {
    float sign;
    s32 hi, lo, mid, step;
    float t_lo, t_hi, range, frac;
    float lo_f, hi_f;

    if (val < 0.0f) {
        val = -val;
        sign = -1.0f;
    } else {
        sign = 1.0f;
    }

    if (val >= 1.0f) {
        return sign * 90.0f;
    }

    hi = 0x4000;
    lo = 0;
    step = 0x2000;

    while (step != 0) {
        mid = lo + step;
        if (NuTrigTable[mid] >= val) {
            hi = mid;
        } else {
            lo = mid;
        }
        step >>= 1;
    }

    t_lo = NuTrigTable[lo];
    t_hi = NuTrigTable[hi];
    range = t_hi - t_lo;
    frac = val - t_lo;
    hi_f = (float)hi;
    lo_f = (float)lo;

    if (range > 0.0f) {
        lo_f = lo_f + (hi_f - lo_f) * frac / range;
    }

    return sign * lo_f / 182.04445f;
}