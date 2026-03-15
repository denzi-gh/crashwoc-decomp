#ifndef SKINNING_H
#define SKINNING_H

#include "gs.h"
#include "system/gs.h"

struct _GSMATRIX* CV_SKINMTX;
struct _GS_VERTEXSKIN* GS_SkinVertexSource;
struct _GS_VECTOR3* GS_BlendSource;
int MaxSkinVerts;
int SkinLights;
float c_one;

void VecMatMulAndWeight1(_GS_VERTEXNORM *vtx,_GS_VECTOR3 *source,_GSMATRIX *mtx);
void VecMatMulAndWeight3(struct nuvec_s *arg0,float *inputvert,_GSMATRIX *cvskinmtx1,
                        _GSMATRIX *cvskinmtx2,_GSMATRIX *cvskinmtx3,float *weights,float *c_one);

#endif // !SKINNING_H
