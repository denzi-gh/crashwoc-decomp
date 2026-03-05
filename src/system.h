#ifndef SYSTEM_H
#define SYSTEM_H

#include "system/gs.h"

#define GXFIFO_ADDR 0xCC008000

struct nugeomitem_s;
struct nugeom_s;
struct nuprim_s;
void SetupShaders(struct nugeomitem_s* geomitem);
void ResetShaders(void);
short NuShaderAssignShader(struct nugeom_s* geom);
void NuShaderSetBypassShaders(s32 flag);
void NuShaderSetSkinningConstants(struct nugeomitem_s* item, struct nuprim_s* prim);

#endif // !SYSTEM_H
