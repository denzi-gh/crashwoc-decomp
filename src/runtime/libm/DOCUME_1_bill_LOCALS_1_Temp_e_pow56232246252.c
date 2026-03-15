extern double sqrt(double);
extern double fabs(double);
extern double scalbn(double, int);

static double
bp[] = {1.0, 1.5,},
dp_h[] = { 0.0, 5.84962487220764160156e-01,}, /* 0x3FE2B803, 0x40000000 */
dp_l[] = { 0.0, 1.35003920212974897128e-08,}, /* 0x3E4CFDEB, 0x43CFD006 */
zero    =  0.0,
one	=  1.0,
two	=  2.0,
two53   =  9007199254740992.0,	/* 0x43400000, 0x00000000 */
huge	=  1.0e300,
tiny    =  1.0e-300,
	/* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
L1  =  5.99999999999994648725e-01, /* 0x3FE33333, 0x33333303 */
L2  =  4.28571428578550184252e-01, /* 0x3FDB6DB6, 0xDB6FABFF */
L3  =  3.33333329818377432918e-01, /* 0x3FD55555, 0x518F264D */
L4  =  2.72728123808534006489e-01, /* 0x3FD17460, 0xA91D4101 */
L5  =  2.30660745775561754067e-01, /* 0x3FCD864A, 0x93C9DB65 */
L6  =  2.06975017800338417784e-01, /* 0x3FCA7E28, 0x4A454EEF */
P1   =  1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
P2   = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
P3   =  6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
P4   = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
P5   =  4.13813679705723846039e-08, /* 0x3E663769, 0x72BEA4D0 */
lg2  =  6.93147180559945286227e-01, /* 0x3FE62E42, 0xFEFA39EF */
lg2_h  =  6.93147182464599609375e-01, /* 0x3FE62E43, 0x00000000 */
lg2_l  = -1.90465429995776804525e-09, /* 0xBE205C61, 0x0CA86C39 */
ovt =  8.0085662595372944372e-17, /* 0x3C971547, 0x652B82FE */
cp    =  9.61796693925975554329071e-01, /* 0x3FEEC709, 0xDC3A03FD */
cp_h  =  9.61796700954437255859375e-01, /* 0x3FEEC709, 0xE0000000 */
cp_l  = -7.02846165095275826516e-09, /* 0xBE3E2FE0, 0x145B01F5 */
ivln2    =  1.44269504088896338700e+00, /* 0x3FF71547, 0x652B82FE */
ivln2_h  =  1.44269502162933349609375e+00, /* 0x3FF71547, 0x60000000 */
ivln2_l  =  1.92596299112661746887e-08; /* 0x3E54AE0B, 0xF85DDF44 */

double pow(double x, double y)
{
	volatile union { double d; int i[2]; } u;
	double z,ax,z_h,z_l,p_h,p_l;
	double y1,t1,t2,r,s,t,v,w;
	double s_h,s_l,s2,t_h,t_l;
	int i,j,k,yisint,n;
	int hx,hy,ix,iy;
	unsigned lx,ly;

	u.d = x; hx = u.i[0]; lx = u.i[1];
	u.d = y; hy = u.i[0]; ly = u.i[1];
	ix = hx&0x7fffffff;
	iy = hy&0x7fffffff;

    /* y==zero: x**0 = 1 */
	if((iy|ly)==0) return one; 	

    /* +-NaN return x+y */
	if(ix > 0x7ff00000 || ((ix==0x7ff00000)&&(lx!=0)) ||
	   iy > 0x7ff00000 || ((iy==0x7ff00000)&&(ly!=0))) 
		return x+y;	

    /* determine if y is an odd int when x < 0
     * yisint = 0	... y is not an integer
     * yisint = 1	... y is an odd int
     * yisint = 2	... y is an even int
     */
	yisint  = 0;
	if(hx<0) {	
	    if(iy>=0x43400000) yisint = 2; /* even integer y */
	    else if(iy>=0x3ff00000) {
		k = (iy>>20)-0x3ff;	   /* exponent */
		if(k>20) {
		    j = ly>>(52-k);
		    if((j<<(52-k))==(int)ly) yisint = 2-(j&1);
		} else if(ly==0) {
		    j = iy>>(20-k);
		    if((j<<(20-k))==iy) yisint = 2-(j&1);
		}
	    }		
	} 

    /* special value of y */
	if(ly==0) { 	
	    if (iy==0x7ff00000) {	/* y is +-inf */
	        if(((ix-0x3ff00000)|lx)==0)
		    return  y - y;	/* inf**+-1 is NaN */
	        else if (ix >= 0x3ff00000)/* (|x|>1)**+-inf = inf,0 */
		    return (hy>=0)? y: zero;
	        else			/* (|x|<1)**-+inf = inf,0 */
		    return (hy<0)?-y: zero;
	    } 
	    if(iy==0x3ff00000) {	/* y is  +-1 */
		if(hy<0) return one/x; else return x;
	    }
	    if(hy==0x40000000) return x*x; /* y is  2 */
	    if(hy==0x3fe00000) {	/* y is  0.5 */
		if(hx>=0)	/* x >= +0 */
		return sqrt(x);	
	    }
	}

	ax = fabs(x);
    /* special value of x */
	if(lx==0) {
	    if(ix==0x7ff00000||ix==0||ix==0x3ff00000){
		z = ax;			/*x is +-0,+-inf,+-1*/
		if(hy<0) z = one/z;	/* z = (1/|x|) */
		if(hx<0) {
		    if(((ix-0x3ff00000)|yisint)==0) {
			z = (z-z)/(z-z); /* (-1)**non-int is NaN */
		    } else if(yisint==1) 
			z = -z;		/* (x<0)**odd = -(|x|**odd) */
		}
		return z;
	    }
	}
    
    /* (x<0)**(non-int) is NaN */
	if(((((unsigned)hx>>31)-1)|yisint)==0) return (x-x)/(x-x);

    /* |y| is huge */
	if(iy>0x41e00000) { /* if |y| > 2**31 */
	    if(iy>0x43f00000){	/* if |y| > 2**64, must o/u flow */
		if(ix<=0x3fefffff) return (hy<0)? huge*huge:tiny*tiny;
		if(ix>=0x3ff00001) return (hy>0)? huge*huge:tiny*tiny;
	    }
	/* over/underflow if x is not close to one */
	    if(ix<0x3fefffff) return (hy<0)? huge*huge:tiny*tiny;
	    if(ix>0x3ff00000) return (hy>0)? huge*huge:tiny*tiny;
	/* now |1-x| is tiny <= 2**-20, suffice to compute 
	   log(x) by x-x^2/2+x^3/3-x^4/4 */
	    t = x-1;		/* t has 20 trailing zeros */
	    w = (t*t)*(0.5-t*(0.3333333333333333333333-t*0.25));
	    t1 = ivln2_h*t;	/* ivln2_h has 16 sig. bits */
	    t2 = t*ivln2_l-w*ivln2;
	    t1 += t2;
	    u.d = t1; u.i[1] = 0; t1 = u.d;
	    t2 = t2-(t1-ivln2_h*t);
	    p_h = t1;	p_l = t2;
	    goto compute_result;
	}

	n = 0;
    /* take care subnormal number */
	if(ix<0x00100000)
	    {ax *= two53; n -= 53; u.d = ax; ix = u.i[0];}
    /* normalize x */
	i  = ix&0x000fffff;
	n += ((ix)>>20)-0x3ff;
	ix = i|0x3ff00000;
	if(i<=0x3988e)		    /* |x|<sqrt(3/2) */
	    k = 0;
	else if(i<0xbb679)  	    /* |x|<sqrt(3)   */
	    k = 1;
	else
	    {k=0;n+=1;ix -= 0x00100000;}
	u.d = ax; u.i[0] = ix; ax = u.d;

    /* compute s = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
	t1 = ax-bp[k];		/* bp[0]=1.0, bp[1]=1.5 */
	v = one/(ax+bp[k]);
	s = t1*v;
	u.d = s; u.i[1] = 0; s_h = u.d;
	/* t_h=ax+bp[k] High */
	u.i[0] = ((ix>>1)|0x20000000)+0x00080000+(k<<18);
	u.i[1] = 0;
	t_h = u.d;
	t_l = ax - (t_h-bp[k]);
	s_l = v*((t1-s_h*t_h)-s_h*t_l);
    /* compute log(ax) */
	s2 = s*s;
	r = s2*s2*(L1+s2*(L2+s2*(L3+s2*(L4+s2*(L5+s2*L6)))));
	r += s_l*(s_h+s);
	s2  = s_h*s_h;
	t_h = 3.0+s2+r;
	u.d = t_h; u.i[1] = 0; t_h = u.d;
	t_l = r-((t_h-3.0)-s2);
    /* t1+v = s*(1+...) */
	t1 = s_h*t_h;
	v = s_l*t_h+t_l*s;
    /* 2/(3log2)*(s+...) */
	p_h = t1+v;
	u.d = p_h; u.i[1] = 0; p_h = u.d;
	p_l = v-(p_h-t1);
	z_h = cp_h*p_h;		/* cp_h+cp_l = 2/(3*log2) */
	z_l = cp_l*p_h+p_l*cp+dp_l[k];
    /* log2(ax) = (s+..)*2/(3*log2) = n + dp_h + z_h + z_l */
	t = (double)n;
	t1 = (((z_h+z_l)+dp_h[k])+t);
	u.d = t1; u.i[1] = 0; t1 = u.d;
	t2 = z_l-(((t1-t)-dp_h[k])-z_h);

compute_result:
    /* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
	s = one; /* s (sign of result -ve**odd = -ve) */
	if(((((unsigned)hx>>31)-1)|(yisint-1))==0) s = -one;

	u.d = y; u.i[1] = 0; y1 = u.d;
	p_l = (y-y1)*t1+y*t2;
	p_h = y1*t1;
	z = p_l+p_h;
	u.d = z; j = u.i[0]; i = u.i[1];
	if (j>=0x40900000) {				/* z >= 1024 */
	    if(((j-0x40900000)|i)!=0)			/* if z > 1024 */
		return s*huge*huge;			/* overflow */
	    else {
		if(p_l+ovt>z-p_h) return s*huge*huge;	/* overflow */
	    }
	} else if ((j&0x7fffffff)>=0x4090cc00 ) {	/* z <= -1075 */
	    if(((j-0xc0900000)|i)!=0)			/* z < -1075 */
		return s*tiny*tiny;			/* underflow */
	    else {
		if(p_l<=z-p_h) return s*tiny*tiny;	/* underflow */
	    }
	}
    /*
     * compute 2**(p_h+p_l)
     */
	k  = (j&0x7fffffff)>>20;
	n  = 0;
	if(j>0x3fe00000) {		/* if |z| > 0.5, set n = [z+0.5] */
	    n = j+(0x00100000>>(k-0x3fe));
	    k = ((n&0x7fffffff)>>20)-0x3ff;	/* new k for n */
	    u.d = zero;
	    u.i[0] = (n&~(0x000fffff>>k));
	    t = u.d;
	    n = ((n&0x000fffff)|0x00100000)>>(20-k);
	    if(j<0) n = -n;
	    p_h -= t;
	}
	t = p_l+p_h;
	u.d = t; u.i[1] = 0; t = u.d;
	v = (p_l-(t-p_h))*lg2+t*lg2_l;
	z = t*lg2_h+v;
	w = v-(z-t*lg2_h);
	t  = z*z;
	t1  = z - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
	r  = (z*t1)/(t1-two)-(w+z*w);
	z  = one-(r-z);
	u.d = z; j = u.i[0];
	j += (n<<20);
	if((j>>20)<=0) z = scalbn(z,n);	/* subnormal output */
	else { u.i[0] = j; z = u.d; }
	return s*z;
}
