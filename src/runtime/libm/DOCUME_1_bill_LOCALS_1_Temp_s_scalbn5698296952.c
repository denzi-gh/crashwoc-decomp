extern double copysign(double, double);

static const double
two54   =  1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
twom54  =  5.55111512312578270212e-17, /* 0x3C900000, 0x00000000 */
huge    = 1.0e+300,
tiny    = 1.0e-300;

#define EXTRACT_WORDS(hi, lo, d)                                               \
    do {                                                                       \
        union {                                                                \
            double val;                                                        \
            int words[2];                                                      \
        } _ew_u;                                                               \
        _ew_u.val = (d);                                                       \
        (hi) = _ew_u.words[0];                                                 \
        (lo) = _ew_u.words[1];                                                 \
    } while (0)

#define INSERT_WORDS(d, hi, lo)                                                \
    do {                                                                       \
        union {                                                                \
            double val;                                                        \
            int words[2];                                                      \
        } _iw_u;                                                               \
        _iw_u.words[0] = (hi);                                                 \
        _iw_u.words[1] = (lo);                                                 \
        (d) = _iw_u.val;                                                       \
    } while (0)

double scalbn(double x, int n)
{
    int k, hx, lx;

    EXTRACT_WORDS(hx, lx, x);
    k = (hx & 0x7ff00000) >> 20;
    if (k == 0) {
        if ((lx | (hx & 0x7fffffff)) == 0)
            return x;
        x *= two54;
        EXTRACT_WORDS(hx, lx, x);
        k = ((hx & 0x7ff00000) >> 20) - 54;
        if (n < -50000)
            return tiny * x;
    }
    if (k == 0x7ff)
        return x + x;
    k = k + n;
    if (k > 0x7fe)
        return huge * copysign(huge, x);
    if (k > 0) {
        INSERT_WORDS(x, (hx & 0x800fffff) | (k << 20), lx);
        return x;
    }
    if (k <= -54) {
        if (n > 50000)
            return huge * copysign(huge, x);
        else
            return tiny * copysign(tiny, x);
    }
    k += 54;
    INSERT_WORDS(x, (hx & 0x800fffff) | (k << 20), lx);
    return x * twom54;
}
