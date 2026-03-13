#include "nurand.h"

u32 fseed = 0;
//global_rand.idum = 1;

long NuRand(struct nunrand_s* nrand)
{
	long k;

	if (nrand == NULL)
	{
		nrand = &global_rand;
	}
	k = (nrand->idum ^ 0x75bd924) / 0x31e5;
	nrand->idum = ((nrand->idum ^ 0x75bd924) % 0x31e5) * 0x41a7 - k * 0xb14;
	if (nrand->idum < 0)
	{
		nrand->idum += 0x7fffffff;
	}
	nrand->idum ^= 0x75bd924;

	return nrand->idum;
}

void NuRandSeed(u32 seed)
{
	fseed = seed;
}

f32 NuRandFloat(void)
{
	union { long l; f32 f; } itemp;

	fseed = fseed * 0x19660d + 0x3c6ef35f;
	itemp.l = (fseed & 0x7fffff) | 0x3f800000;
	return itemp.f - 1.0f;
}
