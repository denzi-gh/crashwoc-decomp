#include "nuvport.h"

void NuVpUpdate(void);
#include "../system.h"

static struct nuviewport_s vpDevice;
static struct nuviewport_s vpCurrent;
static s32 vport_inval;
static struct numtx_s smtx;

//NGC
void NuVpInit(void)
{
    GS_GetViewport(&vpDevice);
    vpDevice.clipmin_x = (float)vpDevice.x;
    vpDevice.centre_x = (float)vpDevice.x + (float)vpDevice.width * 0.5f;
    vpDevice.clipmax_x = (float)vpDevice.x + (float)vpDevice.width;
    vpDevice.clipmin_y = (float)vpDevice.y;
    vpDevice.centre_y = (float)vpDevice.y + (float)vpDevice.height * 0.5f;
    vpDevice.clipmax_y = (float)vpDevice.y + (float)vpDevice.height;

    memcpy(&vpCurrent, &vpDevice, sizeof(struct nuviewport_s));
    vport_inval = 1;

    NuVpUpdate();
    return;
}


//NGC
static void NuVpSetScalingMtx(void)
{
  float x = (float)vpCurrent.x;
  float y = (float)vpCurrent.y;
  float w = (float)vpCurrent.width;
  float h = (float)vpCurrent.height;
  float zmin = vpCurrent.zmin;
  float zmax = vpCurrent.zmax;

  smtx._00 = w * 0.5f;
  smtx._01 = 0.0f;
  smtx._02 = 0.0f;
  smtx._03 = 0.0f;
  smtx._10 = 0.0f;
  smtx._11 = -h * 0.5f;
  smtx._12 = 0.0f;
  smtx._13 = 0.0f;
  smtx._20 = 0.0f;
  smtx._21 = 0.0f;
  smtx._22 = zmax - zmin;
  smtx._23 = 0.0f;
  smtx._30 = w * 0.5f + x;
  smtx._31 = y + h * 0.5f;
  smtx._32 = zmin;
  smtx._33 = 1.0f;
  return;
}

//NGC
void NuVpUpdate(void)
{
  if (vport_inval != 0) {
    vport_inval = 0;
    GS_SetViewport(&vpCurrent);
    NuVpSetClippingMtx();
    NuVpSetScalingMtx();
  }
  return;
}

void NuVpSetClippingMtx(void)
{
  NuMtxSetIdentity(&cmtx);
  return;
}

//NGC
void NuVpSetSize(float w,float h)
{
  vpCurrent.width = (u32)w;
  vpCurrent.height = (u32)h;
  vport_inval = 1;
  return;
}

//PS2
float NuVpPixelWidth(float param_1)
{
  return param_1 * (float)vpCurrent.width + (float)vpCurrent.x;
}

//PS2
float NuVpPixelHeight(float param_1)
{
  return param_1 * (float)vpCurrent.height + (float)vpCurrent.y;
}

//PS2
struct nuviewport_s * NuVpGetCurrentViewport(void)
{
  return &vpCurrent;
}

//PS2
void NuVpGetScalingMtx(struct numtx_s *mtx)
{
    if (mtx != NULL) {
      memcpy(mtx, &smtx, sizeof(struct numtx_s));
    }
    return;
}

//NGC MATCH
void NuVpGetClippingMtx(struct numtx_s *mtx) {
  if (mtx != NULL) {
    *mtx = cmtx;
  }
  return;
}
