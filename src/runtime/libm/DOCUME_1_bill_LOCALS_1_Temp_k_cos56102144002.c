typedef union {
    double value;
    struct {
        unsigned int msw;
        unsigned int lsw;
    } parts;
} ieee_double_shape_type;

#define GET_HIGH_WORD(i,d)                                      \
do {                                                            \
    ieee_double_shape_type gh_u;                                \
    gh_u.value = (d);                                           \
    (i) = gh_u.parts.msw;                                      \
} while (0)

#define INSERT_WORDS(d,ix0,ix1)                                 \
do {                                                            \
    ieee_double_shape_type iw_u;                                \
    iw_u.parts.msw = (ix0);                                    \
    iw_u.parts.lsw = (ix1);                                    \
    (d) = iw_u.value;                                           \
} while (0)

static double
one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
C1  =  4.16666666666666019037e-02, /* 0x3FA55555, 0x5555554C */
C2  = -1.38888888888741095749e-03, /* 0xBF56C16C, 0x16C15177 */
C3  =  2.48015872894767294178e-05, /* 0x3EFA01A0, 0x19CB1590 */
C4  = -2.75573143513906633035e-07, /* 0xBE927E4F, 0x809C52AD */
C5  =  2.08757232129817482790e-09, /* 0x3E21EE9E, 0xBDB4B1C4 */
C6  = -1.13596475577881948265e-11; /* 0xBDA8FAE9, 0xBE8838D4 */

static double half = 0.5;
static double Q1 = 0.28125;

double __kernel_cos(double x, double y)
{
    double a, hz, z, r, qx;
    int ix;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;

    if (ix < 0x3e400000 && (int)x == 0) {
        return one;
    }
    z = x * x;
    r = z * (C1 + z * (C2 + z * (C3 + z * (C4 + z * (C5 + z * C6)))));
    if (ix < 0x3FD33333) {
        return one - (z * half - (z * r - x * y));
    } else {
        if (ix > 0x3fe90000) {
            qx = Q1;
        } else {
            INSERT_WORDS(qx, ix - 0x00200000, 0);
        }
        a = one - qx;
        hz = z * half - qx;
        return a - (hz - (z * r - x * y));
    }
}
