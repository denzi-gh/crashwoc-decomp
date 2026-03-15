#include "nu_asm.h"

f32 NuFsqrt(f32 f)
{
	return (f32)sqrt(f);
}


f32 NuFdiv(f32 dividend, f32 divisor)
{
	if (divisor == 0.0f || dividend == 0.0f) {
		return 0.0f;
	}
	return dividend / divisor;
}

void NuVec4MtxTransformVU0(struct nuvec4_s* dest, struct nuvec4_s* a, struct Mtx* b)
{
	f32 x, y, z, w;

	x = a->x * b->m11 + a->y * b->m21 + a->z * b->m31 + a->w * b->m41;
	y = a->x * b->m12 + a->y * b->m22 + a->z * b->m32 + a->w * b->m42;
	z = a->x * b->m13 + a->y * b->m23 + a->z * b->m33 + a->w * b->m43;
	w = a->x * b->m14 + a->y * b->m24 + a->z * b->m34 + a->w * b->m44;

	dest->x = x;
	dest->y = y;
	dest->z = z;
	dest->w = w;
}
