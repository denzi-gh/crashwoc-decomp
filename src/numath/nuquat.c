#include "nuquat.h"
#include "nu_asm.h"

double acos(double x);
double sqrt(double x);
double sin(double x);

void NuMtxToQuat(struct Mtx* m, struct Quat* q)
{
    struct Quat tmp;
	f32 qw2 = m->m11 + m->m22 + m->m33;
	s32 weight[3] = {1, 2, 0};
	s32 mode;
	s32 p1;
	s32 p2;
	f32 qw2P2;

	if (qw2 > 0.0)
	{
		qw2P2 = (f32)sqrt(qw2 + 1.0f);
		qw2 = 0.5f / qw2P2;
		q->w = qw2P2 * 0.5f;
		q->x = (m->m23 - m->m32) * qw2;
		q->y = (m->m31 - m->m13) * qw2;
		q->z = (m->m12 - m->m21) * qw2;
	}
	else
	{
		mode = 0;
		if (m->m22 > m->m11)
		{
			mode = 1;
		}
		if (m->m33 > (&m->m11)[mode * 5])
		{
			mode = 2;
		}
		p1 = weight[mode];
		p2 = weight[p1];
		qw2P2 = (f32)sqrt((((&m->m11)[mode * 5] - ((&m->m11)[p1 * 5] + (&m->m11)[p2 * 5])) + 1.0f));
		qw2 = qw2P2;

		(&tmp.x)[mode] = qw2 * 0.5f;
		if (qw2 != 0.0f)
		{
			qw2 = 0.5f / qw2;
		}
		tmp.w = ((&m->m11)[p1 * 4 + p2] - (&m->m11)[p2 * 4 + p1]) * qw2;
		(&tmp.x)[p1] = ((&m->m11)[mode * 4 + p1] + (&m->m11)[p1 * 4 + mode]) * qw2;
		(&tmp.x)[p2] = ((&m->m11)[mode * 4 + p2] + (&m->m11)[p2 * 4 + mode]) * qw2;
		q->x = tmp.x;
		q->w = tmp.w;
		q->y = tmp.y;
		q->z = tmp.z;
	}
}

void NuQuatToMtx(struct Quat* q, struct Mtx* m)
{
	f32 x = q->x;
	f32 y = q->y;
	f32 z = q->z;
	f32 w = q->w;
	f32 y2;
	f32 diff;
	f32 xy;
	f32 xz;
	f32 xw;
	f32 yz;
	f32 yw;
	f32 zw;
	f32 zz;

	m->m44 = 1.0f;
	y2 = y * y;
	diff = w * w - x * x;
	m->m34 = 0.0f;
	m->m41 = 0.0f;
	m->m42 = 0.0f;
	m->m43 = 0.0f;
	m->m14 = 0.0f;
	m->m24 = 0.0f;
	xy = x * y;
	xz = x * z;
	xw = x * w;
	yz = y * z;
	yw = y * w;
	zw = z * w;
	xy = xy + xy;
	xz = xz + xz;
	xw = xw + xw;
	yz = yz + yz;
	yw = yw + yw;
	zw = zw + zw;
	zz = z * z;
	m->m12 = xy + zw;
	m->m13 = xz - yw;
	m->m23 = yz + xw;
	m->m33 = (diff - y2) + zz;
	m->m11 = ((w * w + x * x) - y2) - zz;
	m->m22 = (diff + y2) - zz;
	m->m21 = xy - zw;
	m->m31 = xz + yw;
	m->m32 = yz - xw;
}

void NuQuatMul(struct Quat* dest, struct Quat* a, struct Quat* b)
{
	dest->w = ((a->w * b->w - a->x * b->x) - a->y * b->y) - a->z * b->z;
	dest->x = (a->y * b->z + (a->w * b->x + a->x * b->w)) - a->z * b->y;
	dest->y = (a->z * b->x + (a->w * b->y + a->y * b->w)) - a->x * b->z;
	dest->z = (a->x * b->y + (a->w * b->z + a->z * b->w)) - a->y * b->x;
}

void NuQuatNormalise(struct Quat* dest, struct Quat* q)
{
	f32 mag = q->x * q->x + q->w * q->w + q->y * q->y + q->z * q->z;
	f32 scale;

	if (mag > 0.0)
	{
		mag = (f32)sqrt(mag);
		scale = 1.0f / mag;
		dest->w = q->w * scale;
		dest->x = q->x * scale;
		dest->y = q->y * scale;
		dest->z = q->z * scale;
	}
	else
	{
		*dest = *q;
	}
}

void NuQuatSlerp(f32 alpha, struct Quat* dest, struct Quat* a, struct Quat* b)
{
	f32 mag = a->x * a->x + a->y * a->y + a->z * a->z + a->w * a->w;
	f32 x = b->x;
	f32 y = b->y;
	f32 z = b->z;
	f32 w = b->w;
	f32 rot;
	f32 ac;
	f32 s;

	if (mag < 0.0)
	{
		x = -x;
		y = -y;
		z = -z;
		w = -w;
		mag = -mag;
	}
	rot = 1.0;
	if (1.0 - mag <= 0.0)
	{
		mag = 1.0 - alpha;
	}
	else
	{
		ac = acos(mag);
		s = sin(ac);
		mag = sin((rot - alpha) * ac);
		mag = mag / s;
		rot = sin(alpha * ac);
		alpha = rot / s;
	}
	dest->x = mag * a->x + alpha * x;
	dest->y = mag * a->y + alpha * y;
	dest->z = mag * a->z + alpha * z;
	dest->w = mag * a->w + alpha * w;
}
