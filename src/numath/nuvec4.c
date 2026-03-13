#include "nuvec4.h"
#include "numtx.h"

void NuVec4Scale(struct nuvec4_s* v, struct nuvec4_s* v0, f32 k)
{
	v->x = v0->x * k;
	v->y = v0->y * k;
	v->z = v0->z * k;
	v->w = v0->w * k;
}

void NuVec4MtxTransform(struct nuvec4_s* v, struct nuvec_s* v0, struct numtx_s* m0)
{
	float x, y, z, w;

	x = v0->x * m0->_00 + v0->y * m0->_10 + v0->z * m0->_20 + m0->_30;
	y = v0->x * m0->_01 + v0->y * m0->_11 + v0->z * m0->_21 + m0->_31;
	z = v0->x * m0->_02 + v0->y * m0->_12 + v0->z * m0->_22 + m0->_32;
	w = v0->x * m0->_03 + v0->y * m0->_13 + v0->z * m0->_23 + m0->_33;

	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
}

void NuVec4MtxInvTransform(struct nuvec4_s* dest, struct nuvec4_s* a, struct Mtx* b)
{
	struct Mtx tmp;
	NuMtxInv(&tmp, b);
	NuVec4MtxTransform(dest, (struct nuvec_s*)a, (struct numtx_s*)&tmp);
}
