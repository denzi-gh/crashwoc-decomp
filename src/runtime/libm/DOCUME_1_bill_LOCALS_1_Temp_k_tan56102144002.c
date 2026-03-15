extern double fabs(double);

static double
one    =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
pio4   =  7.85398163397448278999e-01, /* 0x3FE921FB, 0x54442D18 */
pio4lo =  3.06161699786838301793e-17, /* 0x3C81A626, 0x33145C07 */
T[] = {
     3.33333333333334091986e-01, /* 0x3FD55555, 0x55555563 */
     1.33333333333201242699e-01, /* 0x3FC11111, 0x1110FE7A */
     5.39682539762260521377e-02, /* 0x3FA1BA1B, 0xA5D40300 */
     2.18694882948595424599e-02, /* 0x3F9664F4, 0x8406D637 */
     8.86323982359930005737e-03, /* 0x3F8226E3, 0xE96E8493 */
     3.59207914911390065437e-03, /* 0x3F6D6D22, 0xC9560328 */
     1.45620945432529025516e-03, /* 0x3F57DBC8, 0xFEE08315 */
     5.88041240820264096874e-04, /* 0x3F4344D8, 0xF2F26501 */
     2.46463134818469906812e-04, /* 0x3F3026F7, 0x1A8D1068 */
     7.81794442939557092300e-05, /* 0x3F147E88, 0xA03792A6 */
     7.14072491382608190305e-05, /* 0x3F12B80F, 0x32F0A7E9 */
    -1.85586374855275456654e-05, /* 0xBEF375CB, 0xDB605373 */
     2.59073051863633712884e-05, /* 0x3EFB2A71, 0x74D8514B */
};

double __kernel_tan(double x, double y, int iy)
{
    double z, r, v, w, s;
    volatile union { double d; int i[2]; } u;
    int ix, hx;

    u.d = x;
    hx = u.i[0];
    ix = hx & 0x7fffffff;

    if (ix < 0x3e300000) { /* |x| < 2**-28 */
        if ((int)x == 0) { /* generate inexact */
            u.d = x;
            if (((ix | u.i[1]) | (iy + 1)) == 0)
                return one / fabs(x);
            else if (iy == 1)
                return x;
            else {
                return -1.0 / x;
            }
        }
    }

    if (ix >= 0x3FE59428) { /* |x| >= 0.6744 */
        if (hx < 0) { x = -x; y = -y; }
        z = pio4 - x;
        w = pio4lo - y;
        x = z + w;
        y = 0.0;
    }

    z = x * x;
    w = z * z;

    r = T[1] + w * (T[3] + w * (T[5] + w * (T[7] + w * (T[9] + w * T[11]))));
    v = z * (T[2] + w * (T[4] + w * (T[6] + w * (T[8] + w * (T[10] + w * T[12])))));
    s = z * x;
    r = y + z * (s * (r + v) + y);
    r += T[0] * s;
    w = x + r;

    if (ix >= 0x3FE59428) {
        v = (double)iy;
        return (double)(1 - ((hx >> 30) & 2)) * (v - 2.0 * (x - (w * w / (w + v) - r)));
    }

    if (iy == 1)
        return w;
    else {
        double a, t;
        z = w;
        u.d = z;
        u.i[0] = u.i[0];
        u.i[1] = 0;
        z = u.d;
        v = r - (z - x);
        t = a = -1.0 / w;
        u.d = t;
        u.i[0] = u.i[0];
        u.i[1] = 0;
        t = u.d;
        s = 1.0 + t * z;
        return t + a * (s + t * v);
    }
}
