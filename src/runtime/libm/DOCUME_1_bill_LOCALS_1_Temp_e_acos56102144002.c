extern double sqrt(double);

static double
one       =  1.00000000000000000000e+00,
pi        =  3.14159265358979311600e+00,
pio2_hi   =  1.57079632679489655800e+00,
pio2_lo   =  6.12323399573676603587e-17,
pS0       =  1.66666666666666657415e-01,
pS1       = -3.25565818622400915405e-01,
pS2       =  2.01212532134862925881e-01,
pS3       = -4.00555345006794114027e-02,
pS4       =  7.91534994289814532176e-04,
pS5       =  3.47933107596021167570e-05,
qS1       = -2.40339491173441421878e+00,
qS2       =  2.02094576023350569471e+00,
qS3       = -6.88283971605453293030e-01,
qS4       =  7.70381505559019352791e-02;

static double zero = 0.0;
static double half = 0.5;

double acos(double x)
{
    double z, p, q, r, w, s, c, df;
    volatile union { double d; int i[2]; } u;
    int hx, ix;

    u.d = x;
    hx = u.i[0];
    ix = hx & 0x7fffffff;
    if (ix >= 0x3ff00000) { /* |x| >= 1 */
        u.d = x;
        if (((ix - 0x3ff00000) | u.i[1]) == 0) { /* |x|==1 */
            if (hx > 0)
                return zero;
            else
                return pi;
        }
        return (x - x) / (x - x);
    }
    if (ix < 0x3fe00000) { /* |x| < 0.5 */
        if (ix <= 0x3c600000)
            return pio2_hi;
        z = x * x;
        p = z * (pS0 + z * (pS1 + z * (pS2 + z * (pS3 + z * (pS4 + z * pS5)))));
        q = one + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
        r = p / q;
        return pio2_hi - (x - (pio2_lo - x * r));
    } else if (hx < 0) { /* x < -0.5 */
        z = (one + x) * half;
        p = z * (pS0 + z * (pS1 + z * (pS2 + z * (pS3 + z * (pS4 + z * pS5)))));
        q = one + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
        s = sqrt(z);
        r = p / q;
        w = r * s - pio2_lo;
        return pi - 2.0 * (s + w);
    } else { /* x >= 0.5 */
        int hi;
        z = (one - x) * half;
        s = sqrt(z);
        u.d = s;
        hi = u.i[0];
        u.i[0] = hi;
        u.i[1] = 0;
        df = u.d;
        c = (z - df * df) / (s + df);
        p = z * (pS0 + z * (pS1 + z * (pS2 + z * (pS3 + z * (pS4 + z * pS5)))));
        q = one + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
        r = p / q;
        w = r * s + c;
        return 2.0 * (df + w);
    }
}
