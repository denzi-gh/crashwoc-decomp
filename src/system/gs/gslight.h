#pragma once

#include "system/gs.h"

void GS_SetMaterial(struct _D3DMATERIAL8 *pMaterial);
s32 GS_SetLight(s32 Index, const struct _D3DLIGHT8 *pLight);
s32 GS_LightEnable(s32 Index, s32 Enable);
void GS_XFormLightVec(struct _GS_VECTOR3 *XFlight_pos, struct _GS_VECTOR4 *light_pos, struct _GSMATRIX *curmat);
void GS_SetLightingNone(void);
void GS_SetPointLighting(void);
void GS_EnableLighting(int flag);
void GS_SetMaterialSourceAmbient(s32 src);
void GS_SetMaterialSourceEmissive(int src);
void GS_EnableColorVertex(int flag);
void GS_EnableSpecular(int flag);
void GS_Set3Lights(struct _GS_VECTOR4 *LIGHT1_POS, struct _GS_VECTOR4 *LIGHT2_POS, struct _GS_VECTOR4 *LIGHT3_POS,
                   struct _GS_VECTOR4 *LIGHT1_COLOR, struct _GS_VECTOR4 *LIGHT2_COLOR,
                   struct _GS_VECTOR4 *LIGHT3_COLOR, struct _GXColor *AMB_COLOR);
