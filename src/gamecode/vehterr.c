#include "main.h"
#include <string.h>

struct VEHICLE {
    struct nuvec_s ActualWheelPosition[4];
    struct nuvec_s OldWheelPosition[4];
    s32 BigSpin[4];
    struct nuvec_s ActualPosition;
    struct nuvec_s Resolved;
    struct nuvec_s Velocity;
    struct nuvec_s WheelAxis[3];
    f32 FrontWheelSpeedAdj;
    s16 aTargetAngle;
    s16 aTarSurfRotX;
    s16 aTarSurfRotZ;
    s16 aActualAngle;
    s16 aActSurfRotX;
    s16 aActSurfRotZ;
    s16 ActFrontRotX;
    s16 ActRearRotX;
    s16 TarFrontRotX;
    s16 TarRearRotX;
    s32 AnyOnGroundBits;
    s32 AllOnGroundBits;
    s32 AllTouchingGroundBits;
    s32 AnyTouchingGroundBits;
    struct nuvec_s AirNormal;
    struct nuvec_s SurfaceNormal;
    s16 *TerrHandle;
    s32 FrontWheelGroundBits;
};

extern struct nuvec_s NullNuVec;
extern struct nuvec_s WorldAxis[3];
extern struct nuvec_s ShadNorm;
extern u16 JeepBaseAng;
extern s32 Level;

float DotProduct(struct nuvec_s *A, struct nuvec_s *B);
struct nuvec_s CrossProduct(struct nuvec_s *A, struct nuvec_s *B);
void SeekHalfLife(float *Val, float Target, float SeekTime, float DeltaTime);
void SeekHalfLifeNUVEC(struct nuvec_s *Val, struct nuvec_s *Target, float SeekTime, float DeltaTime);
struct nuvec_s GenerateJeepWheelPoint(s32 WheelId);
void NewGenerateJeepMatrix(struct numtx_s *Mat, short YAng, short SurfaceX, short SurfaceZ, short TiltX, short TiltZ, struct nuvec_s *Pos);
s32 NewRayCastSetHandel(struct nuvec_s *vpos, struct nuvec_s *vvel, float size, float timeadj, float impactadj, s16 *Handel);
void NewScanInit(void);
s16 *NewScanHandel(struct nuvec_s *vpos, struct nuvec_s *vvel, float size, s32 plats, s16 *handel);
float frand(void);

void FindVehicleNormalGiven4Points(struct nuvec_s *out, struct nuvec_s *points);
void FindSurfaceRotXZFromNormal(struct nuvec_s *normal, s16 *rotX, s16 *rotZ);
s32 TryUnembeddPointDir(struct nuvec_s *pos, struct nuvec_s *dir1, struct nuvec_s *dir2, s16 *handle, float radius);
s32 TryUnembeddPointSafe(struct nuvec_s *pos, struct nuvec_s *target, s16 *handle, float radius);
void BestGuessActualsJeep(struct VEHICLE *v);
s32 MyCast(struct nuvec_s *pos, struct nuvec_s *target, struct nuvec_s *normals, float radius, s32 count, s32 flags, s32 checkFirst, s16 *handle);

static struct nuvec_s TempX = {1.0f, 0.0f, 0.0f};
static struct nuvec_s TempY = {0.0f, 1.0f, 0.0f};
static struct nuvec_s TempZ = {0.0f, 0.0f, 1.0f};
struct nuvec_s xpnt = {-100000.0f, -100000.0f, -100000.0f};

static u16 OldBase;
static s32 LastJeepWithinBounds;

s32 TryUnembeddPointSafe(struct nuvec_s *pos, struct nuvec_s *target, s16 *handle, float radius)
{
    struct nuvec_s dir;
    struct nuvec_s test;
    s32 i;
    s32 count;
    s32 found;

    dir.x = pos->x - target->x;
    dir.y = pos->y - target->y;
    dir.z = pos->z - target->z;

    found = 0;
    count = (s32)(NuVecMag(&dir) * 10.0f);

    for (i = count; i >= 0; i--) {
        if (i != 0) {
            float fi = (float)i;
            float fc = (float)(count + 1);
            float ratio = fi / fc;
            test.x = dir.x * ratio + target->x;
            test.y = dir.y * ratio + target->y;
            test.z = dir.z * ratio + target->z;
        } else {
            test = *target;
        }

        found = ((NewRayCastSetHandel(&test, &NullNuVec, radius, 0.01f, 0.0f, handle) ^ 0x10) >> 4) & 1;

        if (found != 0) {
            *pos = test;
            break;
        }
    }

    return found;
}

s32 TryUnembeddPointDir(struct nuvec_s *pos, struct nuvec_s *dir1, struct nuvec_s *dir2, s16 *handle, float radius)
{
    struct nuvec_s test;
    struct nuvec_s saved;
    float bestScale;
    float cumBest;
    float curScale;
    float scale;
    float step;
    s32 j;
    s32 outer;
    s32 nextOuter;
    s32 innerMax;
    struct nuvec_s *bestDir;

    bestScale = 0.0f;
    step = 0.03f;
    curScale = bestScale;

    NuVecAdd(&saved, dir1, dir2);
    NuVecScale(0.5f, &saved, &saved);

    cumBest = curScale;
    innerMax = 5;
    outer = 1;

    do {
        nextOuter = outer + 1;

        for (j = 1; j <= innerMax; j++) {
            test = *pos;
            scale = step * (float)j + bestScale;
            NuVecScaleAccum(scale, &test, dir1);

            if (NewRayCastSetHandel(&test, &NullNuVec, radius, 0.01f, 0.0f, handle) & 0x10) {
                test = *pos;
                NuVecScaleAccum(scale, &test, dir2);

                if (NewRayCastSetHandel(&test, &NullNuVec, radius, 0.01f, 0.0f, handle) & 0x10) {
                    test = *pos;
                    NuVecScaleAccum(scale, &test, &saved);

                    if (NewRayCastSetHandel(&test, &NullNuVec, radius, 0.01f, 0.0f, handle) & 0x10) {
                        cumBest = scale;
                    } else {
                        curScale = scale;
                        bestDir = &saved;
                        goto outer_end;
                    }
                } else {
                    curScale = scale;
                    bestDir = dir2;
                    goto outer_end;
                }
            } else {
                curScale = scale;
                bestDir = dir1;
                goto outer_end;
            }
        }

    outer_end:
        step *= 0.5f;
        {
            s32 old = outer;
            outer = nextOuter;
            innerMax = 2;
            bestScale = cumBest;
            if (old > 4) break;
        }
    } while (1);

    if (curScale == 0.0f) {
        return 0;
    }

    NuVecScaleAccum(curScale, pos, bestDir);
    return 1;
}

s32 TryUnembeddPointDirSimple(struct nuvec_s *pos, struct nuvec_s *dir, s16 *handle, s32 iterations, float radius, float step)
{
    struct nuvec_s test;
    float bestScale;
    float cumBest;
    float innerBest;
    float curScale;
    s32 j;
    s32 outer;
    s32 nextOuter;
    s32 innerMax;

    bestScale = 0.0f;
    cumBest = bestScale;
    innerMax = 5;
    innerBest = cumBest;
    outer = 1;

    if (iterations <= 0) goto done;

    do {
        nextOuter = outer + 1;

        for (j = 1; j <= innerMax; j++) {
            test = *pos;
            curScale = step * (float)j + bestScale;
            NuVecScaleAccum(curScale, &test, dir);

            if (NewRayCastSetHandel(&test, &NullNuVec, radius, 0.01f, 0.0f, handle) == 0) {
                cumBest = curScale;
                goto inner_done;
            }

            innerBest = curScale;
        }

    inner_done:
        {
            s32 old = outer;
            bestScale = innerBest;
            outer = nextOuter;
            innerMax = 2;
            step *= 0.5f;
            if (old >= iterations) break;
        }
    } while (1);

done:
    if (cumBest == 0.0f) {
        return 0;
    }

    NuVecScaleAccum(cumBest, pos, dir);
    return 1;
}
void BestGuessActualsJeep(struct VEHICLE *v)
{
    struct nuvec_s wp[4];
    struct nuvec_s front;
    struct nuvec_s rear;
    struct nuvec_s *points;
    struct nuvec_s frontToRear;
    struct nuvec_s up;
    struct nuvec_s side;
    struct nuvec_s rotated;
    struct nuvec_s result;
    struct nuvec_s genpt;
    s32 i;
    u16 rotX;
    s16 rotZ;
    s16 diff;
    s16 baseDiff;

    points = wp;

    for (i = 0; i < 4; i++) {
        points[i] = v->ActualWheelPosition[i];
    }

    NuVecAdd(&front, &points[0], &points[1]);
    NuVecScale(0.5f, &front, &front);

    NuVecAdd(&rear, &points[2], &points[3]);
    NuVecScale(0.5f, &rear, &rear);

    NuVecSub(&frontToRear, &rear, &front);

    NuFsqrt(NuVecMagSqr(&frontToRear));

    side.x = ((points[1].x - points[0].x) + points[3].x - points[2].x) * 0.5f;
    side.y = ((points[1].y - points[0].y) + points[3].y - points[2].y) * 0.5f;
    side.z = ((points[1].z - points[0].z) + points[3].z - points[2].z) * 0.5f;

    NuVecCross(&up, &side, &frontToRear);
    NuVecNorm(&up, &up);

    rotX = NuAtan2D(up.z, up.y);
    NuVecRotateX(&rotated, &up, -rotX);

    rotZ = -NuAtan2D(rotated.x, rotated.y);

    v->aActSurfRotX = rotX;
    v->aActSurfRotZ = rotZ;

    if ((s16)rotX < -0x1555) {
        v->aActSurfRotX = -0x1555;
    } else if ((s16)rotX > 0x1555) {
        v->aActSurfRotX = 0x1555;
    }

    NuVecRotateX(&up, &frontToRear, -(s32)v->aActSurfRotX);
    NuVecRotateZ(&up, &up, -(s32)v->aActSurfRotZ);

    v->aActualAngle = NuAtan2D(-up.x, -up.z);

    baseDiff = JeepBaseAng - OldBase;
    diff = v->aActualAngle - (s16)JeepBaseAng;

    if ((u16)(baseDiff + 0xaab) > 0x1556) {
        LastJeepWithinBounds = 0;
    }

    if (LastJeepWithinBounds == 0) goto check_bounds;

    if (Level == 3) {
        if (v->ActualPosition.z >= 751.5f) goto check_bounds;
    }

    if (diff > 0x2000) {
        v->aActualAngle = (s16)JeepBaseAng + 0x2000;
    } else if (diff < -0x2000) {
        v->aActualAngle = (s16)JeepBaseAng - 0x2000;
    }
    goto end_clamp;

check_bounds:
    if ((u16)(diff + 0x1fff) <= 0x3ffe) {
        LastJeepWithinBounds = 1;
    }

end_clamp:
    result.x = (front.x + rear.x) * 0.5f;
    result.y = (front.y + rear.y) * 0.5f;
    result.z = (front.z + rear.z) * 0.5f;

    OldBase = JeepBaseAng;
    genpt = GenerateJeepWheelPoint(5);

    NuVecRotateY(&genpt, &genpt, v->aActualAngle);
    NuVecRotateZ(&genpt, &genpt, v->aActSurfRotZ);
    NuVecRotateX(&genpt, &genpt, v->aActSurfRotX);

    v->ActualPosition.x = result.x - genpt.x;
    v->ActualPosition.y = result.y - genpt.y;
    v->ActualPosition.z = result.z - genpt.z;
}

s32 MyCast(struct nuvec_s *pos, struct nuvec_s *target, struct nuvec_s *normals, float radius, s32 count, s32 flags, s32 checkFirst, s16 *handle)
{
    struct nuvec_s rel;
    struct nuvec_s temp;
    struct nuvec_s norm;
    struct nuvec_s refl;
    s32 i;
    s32 retval;

    memset(&norm, 0, 0xc);
    retval = 0;

    rel.x = target->x - pos->x;
    rel.y = target->y - pos->y;
    rel.z = target->z - pos->z;

    for (i = 0; i < count; ) {
        temp = rel;
        if (NewRayCastSetHandel(pos, &temp, radius, 0.01f, 0.0f, handle) & 0x10) {
            TryUnembeddPointDir(pos, &ShadNorm, &WorldAxis[1], handle, 0.23669f);
            temp = rel;
            NewRayCastSetHandel(pos, &temp, radius, 0.01f, 0.0f, handle);
        }

        pos->x = pos->x + temp.x;
        pos->y = pos->y + temp.y;
        pos->z = pos->z + temp.z;

        if (i == 0 && checkFirst == 0) {
            i = count;
            continue;
        }

        {
            struct nuvec_s shadnorm_copy;
            struct nuvec_s cross_temp;
            float d1, d2;
            s32 hitNorm;
            s32 hitSurface;
            s32 downDot;
            float absdot;
            float f31;
            float dot;

            shadnorm_copy = ShadNorm;

            d1 = shadnorm_copy.z * normals[1].z + shadnorm_copy.x * normals[1].x + shadnorm_copy.y * normals[1].y;
            d2 = shadnorm_copy.z * normals[2].z + shadnorm_copy.x * normals[2].x + shadnorm_copy.y * normals[2].y;

            hitNorm = (d2 >= 0.5f) ? 1 : 0;
            hitSurface = (d1 >= 0.0f) ? 0 : 1;

            if (hitNorm == 0) {
                absdot = shadnorm_copy.z * normals[0].z + shadnorm_copy.x * normals[0].x + shadnorm_copy.y * normals[0].y;
                absdot = NuFabs(absdot);
                hitNorm = (absdot < 0.707f) ? 0 : 1;
            }

            if (hitNorm) {
                cross_temp = CrossProduct(&normals[1], &shadnorm_copy);
                cross_temp = CrossProduct(&cross_temp, &cross_temp);
                f31 = DotProduct(&cross_temp, &rel);
                dot = DotProduct(&cross_temp, &normals[1]);

                if (f31 * dot <= 0.0f) {
                    hitNorm = 0;
                }
            }

            downDot = (DotProduct(&shadnorm_copy, &WorldAxis[1]) < -0.5f) ? 1 : 0;

            if (hitNorm == 0 && hitSurface == 0 && downDot == 0) {
                NuVecScale(0.005f, &refl, &shadnorm_copy);
                NuVecSub(&rel, &rel, &temp);

                if (hitNorm == 0 && checkFirst != 0 && hitSurface == 0) {
                    retval = 1;
                    NuVecScale(0.1f, &rel, &rel);
                }

                {
                    float d;
                    d = rel.x * shadnorm_copy.x + rel.y * shadnorm_copy.z + rel.z * shadnorm_copy.y;
                    d = d - 0.005f;
                    rel.x = rel.x - shadnorm_copy.x * d;
                    rel.y = rel.y - shadnorm_copy.z * d;
                    rel.z = rel.z - shadnorm_copy.y * d;
                }
            } else {
                float cx, cy, cz;
                float magcross, magrel;
                float dp;
                float t;

                cx = shadnorm_copy.x * normals[1].z - shadnorm_copy.y * normals[1].y;
                cy = shadnorm_copy.y * normals[1].x - shadnorm_copy.z * normals[1].z;
                cz = shadnorm_copy.z * normals[1].y - shadnorm_copy.x * normals[1].x;

                cross_temp.x = cy;
                cross_temp.y = cx;
                cross_temp.z = cz;

                magcross = NuFsqrt(cx * cx + cy * cy + cz * cz);
                magrel = NuFsqrt(rel.x * rel.x + rel.y * rel.y + rel.z * rel.z);

                dp = (cross_temp.y * rel.y + cross_temp.x * rel.x + cross_temp.z * rel.z) / (magcross * magrel);

                if (checkFirst != 0) {
                    if (dp >= 0.5f) {
                        t = 1.0f;
                    } else if (dp <= -0.5f) {
                        t = -1.0f;
                    } else {
                        t = dp;
                    }
                } else {
                    t = dp;
                }

                {
                    float ratio = t * magrel / magcross;
                    rel.x = cross_temp.x * ratio;
                    rel.y = cross_temp.y * ratio;
                    rel.z = cross_temp.z * ratio;
                }

                if (hitSurface != 0) {
                    refl.x = (shadnorm_copy.x + normals[1].x) * 0.005f;
                    refl.y = (shadnorm_copy.y + normals[1].y) * 0.005f;
                    refl.z = (shadnorm_copy.z + normals[1].z) * 0.005f;

                    rel.x = rel.x + refl.x;
                    rel.y = rel.y + refl.y;
                    rel.z = rel.z + refl.z;
                } else {
                    struct nuvec_s cr;
                    cr = CrossProduct(&cross_temp, &normals[1]);

                    if (DotProduct(&cr, &shadnorm_copy) > 0.0f) {
                        t = 1.0f / magcross;
                    } else {
                        t = -1.0f / magcross;
                    }

                    refl.x = (normals[0].x * 2.0f - cr.x * t) * 0.005f;
                    refl.y = (normals[1].x * 2.0f - cr.y * t) * 0.005f;
                    refl.z = (normals[1].y * 2.0f - cr.z * t) * 0.005f;

                    rel.x = rel.x + refl.x;
                    rel.y = rel.y + refl.y;
                    rel.z = rel.z + refl.z;
                }
            }

            NewRayCastSetHandel(pos, &refl, radius, 0.01f, 0.0f, handle);

            pos->x = pos->x + refl.x;
            pos->y = pos->y + refl.y;
            pos->z = pos->z + refl.z;

            norm = shadnorm_copy;
        }

        i++;
    }

    return retval;
}
void FindVehicleNormalGiven4Points(struct nuvec_s *out, struct nuvec_s *points)
{
    struct nuvec_s front, rear, frontToRear;
    struct nuvec_s side;

    NuVecAdd(&front, &points[0], &points[1]);
    NuVecScale(0.5f, &front, &front);

    NuVecAdd(&rear, &points[2], &points[3]);
    NuVecScale(0.5f, &rear, &rear);

    NuVecSub(&frontToRear, &rear, &front);

    side.x = ((points[1].x - points[0].x) + points[3].x - points[2].x) * 0.5f;
    side.y = ((points[1].y - points[0].y) + points[3].y - points[2].y) * 0.5f;
    side.z = ((points[1].z - points[0].z) + points[3].z - points[2].z) * 0.5f;

    NuVecCross(out, &side, &frontToRear);
    NuVecNorm(out, out);
}

void FindSurfaceRotXZFromNormal(struct nuvec_s *normal, s16 *rotX, s16 *rotZ)
{
    struct nuvec_s temp;
    short rx;

    rx = NuAtan2D(normal->z, normal->y);
    NuVecRotateX(&temp, normal, -rx);
    *rotX = rx;
    *rotZ = -NuAtan2D(temp.x, temp.y);
}

void FindSurfaceNormalAndUnembedd(struct VEHICLE *v, float deltaTime)
{
    struct nuvec_s temp;
    struct nuvec_s vel[4];
    struct nuvec_s newPos[4];
    struct nuvec_s offset;
    s32 scanResults[4];
    s32 i;
    s32 scan;

    temp = v->WheelAxis[1];
    NuVecScale(-0.1f, &offset, &temp);

    for (i = 0; i <= 3; i++) {
        scan = 1;

        do {
            vel[i] = offset;

            scanResults[i] = NewRayCastSetHandel(&v->ActualWheelPosition[i], &vel[i], 0.23669f, 0.01f, 0.0f, v->TerrHandle);

            if (scanResults[i] & 0x10) {
                if (TryUnembeddPointDir(&v->ActualWheelPosition[i], &ShadNorm, &WorldAxis[1], v->TerrHandle, 0.23669f) == 0) {
                    TryUnembeddPointSafe(&v->ActualWheelPosition[i], &v->OldWheelPosition[i], v->TerrHandle, 0.23669f);
                }
            } else {
                scan = 0;
            }

            if (scanResults[i] == 0 && i > 1) {
                if (v->FrontWheelGroundBits & 1) {
                    newPos[i] = v->ActualWheelPosition[i];
                    goto next_wheel;
                }
            }

            NuVecAdd(&newPos[i], &v->ActualWheelPosition[i], &vel[i]);

            {
                s32 old = scan;
                scan--;
                if (old <= 0) break;
            }
        } while (1);

    next_wheel:
        ;
    }

    v->FrontWheelGroundBits = v->FrontWheelGroundBits << 1;
    if (scanResults[0] != 0 || scanResults[1] != 0) {
        v->FrontWheelGroundBits |= 1;
    }

    v->AnyOnGroundBits <<= 1;
    v->AllOnGroundBits = (v->AllOnGroundBits << 1) | 1;

    for (i = 0; i < 4; i++) {
        if (scanResults[i] != 0) {
            v->AnyOnGroundBits |= 1;
        } else {
            v->AllOnGroundBits &= ~1;
        }
    }

    v->AllTouchingGroundBits = (v->AllTouchingGroundBits << 1) | 1;
    v->AnyTouchingGroundBits <<= 1;

    for (i = 0; i < 4; i++) {
        if (scanResults[i] == 0) {
            v->AllTouchingGroundBits &= ~1;
            continue;
        }

        if (NuVecMag(&vel[i]) <= 0.01f || (scanResults[i] & 0x10)) {
            v->AnyTouchingGroundBits |= 1;
        } else {
            v->AllTouchingGroundBits &= ~1;
        }
    }

    if (v->AnyOnGroundBits & 1) {
        FindVehicleNormalGiven4Points(&v->SurfaceNormal, newPos);
    } else {
        if (v->AnyOnGroundBits & 2) {
            v->AirNormal = WorldAxis[1];

            NuVecScaleAccum((frand() - 0.5f) * 1.2f, &v->AirNormal, &WorldAxis[0]);
            NuVecScaleAccum(frand() - 0.5f - 0.25f, &v->AirNormal, &WorldAxis[2]);

            NuVecRotateY(&temp, &temp, v->aActualAngle);
            NuVecNorm(&v->AirNormal, &v->AirNormal);
        }

        SeekHalfLifeNUVEC(&v->SurfaceNormal, &v->AirNormal, 0.3f, deltaTime);
    }

    FindSurfaceRotXZFromNormal(&v->SurfaceNormal, &v->aTarSurfRotX, &v->aTarSurfRotZ);
}

void TerrainVehicleSoft(struct nuvec_s *out, struct VEHICLE *v, struct nuvec_s *pos)
{
    struct numtx_s mtx;
    struct numtx_s mtx2;
    struct nuvec_s wp[4];
    struct nuvec_s wp2[4];
    struct nuvec_s genpt;
    struct nuvec_s genpt2;
    struct nuvec_s resolved;
    struct nuvec_s diff;
    struct nuvec_s minPt;
    struct nuvec_s maxPt;
    struct nuvec_s vel;
    struct nuvec_s oldPos;
    s32 scanResults[4];
    s32 i, j;
    s32 aOldAngle;
    struct nuvec_s *pAxis;
    float vehSize;

    aOldAngle = v->aActualAngle;
    oldPos = v->ActualPosition;

    for (i = 0; i < 4; i++) {
        v->OldWheelPosition[i] = v->ActualWheelPosition[i];
    }

    NewGenerateJeepMatrix(&mtx, v->aTargetAngle, v->aTarSurfRotX, v->aTarSurfRotZ, 0, 0, NULL);
    NewGenerateJeepMatrix(&mtx2, v->aActualAngle, v->aTarSurfRotX, v->aTarSurfRotZ, 0, 0, NULL);

    pAxis = &v->WheelAxis[0];
    genpt = GenerateJeepWheelPoint(4);

    for (i = 0; i <= 3; i++) {
        wp[i] = GenerateJeepWheelPoint(i);
        wp2[i] = wp[i];
        NuVecMtxRotate(&wp2[i], &wp2[i], &mtx);

        wp2[i].x = wp2[i].x + v->ActualPosition.x + pos->x;
        wp2[i].y = wp2[i].y + v->ActualPosition.y + pos->y;
        wp2[i].z = wp2[i].z + v->ActualPosition.z + pos->z;
    }

    NuVecMtxRotate(&genpt2, &genpt, &mtx2);
    NuVecMtxRotate(&resolved, &genpt, &mtx);

    diff.x = genpt2.x - resolved.x;
    diff.y = genpt2.y - resolved.y;
    diff.z = genpt2.z - resolved.z;

    for (i = 0; i < 4; i++) {
        wp2[i].x += diff.x;
        wp2[i].y += diff.y;
        wp2[i].z += diff.z;
    }

    NuVecMtxRotate(pAxis, &TempX, &mtx);
    NuVecMtxRotate(&v->WheelAxis[1], &TempY, &mtx);
    NuVecMtxRotate(&v->WheelAxis[2], &TempZ, &mtx);

    minPt = v->ActualWheelPosition[0];
    maxPt = minPt;

    for (j = 0; ; ) {
        for (i = j + 1; i <= 3; i++) {
            if (wp2[i].x < minPt.x) {
                minPt.x = wp2[i].x;
            } else if (wp2[i].x > maxPt.x) {
                maxPt.x = wp2[i].x;
            }

            if (wp2[i].y < minPt.y) {
                minPt.y = wp2[i].y;
            } else if (wp2[i].y > maxPt.y) {
                maxPt.y = wp2[i].y;
            }

            if (wp2[i].z < minPt.z) {
                minPt.z = wp2[i].z;
            } else if (wp2[i].z > maxPt.z) {
                maxPt.z = wp2[i].z;
            }
        }

        j = i;
        if (j > 3) break;
    }

    NuVecSub(&diff, &maxPt, &resolved);

    NewScanInit();

    vehSize = 0.23669f;
    v->TerrHandle = NewScanHandel(&resolved, &diff, vehSize + 0.5f, 0, NULL);

    for (i = 0; i <= 3; i++) {
        s32 flags;
        flags = (i <= 1) ? 1 : 0;
        scanResults[i] = MyCast(&v->ActualWheelPosition[i], &wp2[i], pAxis, vehSize, 4, flags, i, v->TerrHandle);
    }

    BestGuessActualsJeep(v);

    vel.x = v->ActualPosition.x - oldPos.x;
    vel.y = v->ActualPosition.y - oldPos.y;
    vel.z = v->ActualPosition.z - oldPos.z;

    NewGenerateJeepMatrix(&mtx2, v->aActualAngle, v->aActSurfRotX, v->aActSurfRotZ, 0, 0, NULL);

    NuVecMtxRotate(&genpt2, &genpt, &mtx2);
    NuVecMtxRotate(&vel, &genpt, &mtx2);

    {
        struct nuvec_s d2;
        d2.x = genpt2.x - vel.x;
        d2.y = genpt2.y - vel.y;
        d2.z = genpt2.z - vel.z;

        vel.x -= d2.x;
        vel.y -= d2.y;
        vel.z -= d2.z;
    }

    {
        float magVel, magB;
        magVel = NuVecMag(pos);
        magB = NuVecMag(&vel);

        if (magVel < magB) {
            NuVecScale(magVel / magB, &vel, &vel);
        }
    }

    {
        s16 angleDiff;
        s16 oldAngle;
        float seekTarget;

        angleDiff = v->aTargetAngle - (u16)aOldAngle;
        oldAngle = v->aActualAngle - (u16)aOldAngle;

        seekTarget = 0.0f;

        if (v->Resolved.z < 2.0f) {
            if (angleDiff > 0x16c) {
                if (oldAngle <= 0xb5) {
                    goto compute_seek;
                }
            }
            if (angleDiff < -0x16c) {
                if (oldAngle <= -0xb6) {
                    goto skip_seek;
                }
            } else {
                goto skip_seek;
            }

        compute_seek:
            seekTarget = (float)(angleDiff - oldAngle) * 10922.667f;
        }

    skip_seek:
        if (seekTarget == 0.0f) {
            SeekHalfLife(&v->FrontWheelSpeedAdj, seekTarget, 0.2f, 0.016666668f);
        } else {
            SeekHalfLife(&v->FrontWheelSpeedAdj, seekTarget, 0.1f, 0.016666668f);
        }
    }

    *out = vel;
}