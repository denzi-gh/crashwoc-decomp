#include "nuvec4.h"
#include "numtx.h"

static const unsigned long D3DSIMPLERENDERSTATEENCODE[82] = {
    0x00040260, 0x00040264, 0x00040268, 0x0004026C, 0x00040270, 0x00040274,
    0x00040278, 0x0004027C, 0x00040288, 0x0004028C, 0x00040A60, 0x00040A64,
    0x00040A68, 0x00040A6C, 0x00040A70, 0x00040A74, 0x00040A78, 0x00040A7C,
    0x00040A80, 0x00040A84, 0x00040A88, 0x00040A8C, 0x00040A90, 0x00040A94,
    0x00040A98, 0x00040A9C, 0x00040AA0, 0x00040AA4, 0x00040AA8, 0x00040AAC,
    0x00040AB0, 0x00040AB4, 0x00040AB8, 0x00040ABC, 0x00040AC0, 0x00040AC4,
    0x00040AC8, 0x00040ACC, 0x00040AD0, 0x00040AD4, 0x00040AD8, 0x00040ADC,
    0x000417F8, 0x00041E20, 0x00041E24, 0x00041E40, 0x00041E44, 0x00041E48,
    0x00041E4C, 0x00041E50, 0x00041E54, 0x00041E58, 0x00041E5C, 0x00041E60,
    0x00041D90, 0x00041E74, 0x00041E78, 0x00040354, 0x0004033C, 0x00040304,
    0x00040300, 0x00040340, 0x00040344, 0x00040348, 0x0004035C, 0x00040310,
    0x0004037C, 0x00040358, 0x00040374, 0x00040378, 0x00040364, 0x00040368,
    0x0004036C, 0x00040360, 0x00040350, 0x0004034C, 0x000409F8, 0x00040384,
    0x00040388, 0x00040330, 0x00040334, 0x00040338
};

static const unsigned long D3DTEXTUREDIRECTENCODE[4] = {
    0x00081B00, 0x00081B40, 0x00081B80, 0x00081BC0
};

static const unsigned long D3DDIRTYFROMRENDERSTATE[35] = {
    0x00002000, 0x00002000, 0x00002000, 0x00002000, 0x00002000, 0x00002000,
    0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x00001200, 0x00003000,
    0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000,
    0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000, 0x00001000,
    0x00000100, 0x00000100, 0x00000900, 0x00000100, 0x00000100, 0x00000100,
    0x00000100, 0x00000100, 0x00000000, 0x00000000, 0x00000000
};

static const unsigned long D3DDIRTYFROMTEXTURESTATE[22] = {
    0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F,
    0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F, 0x0000000F,
    0x0000480F, 0x00000800, 0x00000800, 0x00000800, 0x00000800, 0x00000800,
    0x00000800, 0x00000800, 0x00000800, 0x00000400
};

const f32 lbl_80120F4C __attribute__((section(".rodata"))) = 1.0f;
const f32 lbl_80120F50 __attribute__((section(".rodata"))) = 0.0f;
const f32 lbl_80120F54 __attribute__((section(".rodata"))) = 1.0f;

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
