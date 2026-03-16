#include "nufloat.h"

f32 NuFabs(f32 f)
{
	*(u32*)&f &= 0x7fffffff;
	return f;
}

f32 NuFsign(f32 f)
{
	if (*(s32*)&f >= 0) {
		return 1.0f;
	}
	return -1.0f;
}

f32 NuFpDiv(f32 dividend, f32 divisor)
{
	if (divisor == 0.0f) {
		return 0.0f;
	}
	return dividend / divisor;
}