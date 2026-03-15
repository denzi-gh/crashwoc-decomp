static double huge = 1.0e300;
static double zero = 0.0;

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

double floor(double x)
{
    int i0, i1, j0;
    unsigned i, j;

    EXTRACT_WORDS(i0, i1, x);
    j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
    if (j0 < 20) {
        if (j0 < 0) {
            if (x + huge > zero) {
                if (i0 >= 0) {
                    i0 = 0;
                    i1 = 0;
                } else if (((i0 & 0x7fffffff) | i1) != 0) {
                    i0 = 0xbff00000;
                    i1 = 0;
                }
            }
        } else {
            i = (0x000fffff) >> j0;
            if (((i0 & i) | i1) == 0)
                return x;
            if (x + huge > zero) {
                if (i0 < 0)
                    i0 += (0x00100000) >> j0;
                i0 &= (~i);
                i1 = 0;
            }
        }
    } else if (j0 > 51) {
        if (j0 == 0x400)
            return x + x;
        else
            return x;
    } else {
        i = ((unsigned)(0xffffffff)) >> (j0 - 20);
        if ((i1 & i) == 0)
            return x;
        if (x + huge > zero) {
            if (i0 < 0) {
                if (j0 == 20)
                    i0 += 1;
                else {
                    j = i1 + (1 << (52 - j0));
                    i0 += (j < (unsigned)i1);
                    i1 = j;
                }
            }
            i1 &= (~i);
        }
    }
    INSERT_WORDS(x, i0, i1);
    return x;
}
