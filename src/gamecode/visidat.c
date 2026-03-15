#include "main.h"
#include <stddef.h>
#include <string.h>

//NGC MATCH
void visiSetSplineKnot(struct visidata_s* vd, s32 spix, s32 knotix) {
    s32 n;
    s32 i;
    s32 ix;
    s32 len;
    unsigned long long mask; // r7

    if (vd == NULL) {
        return;
    }
    if ((spix == vd->curspline) && (knotix == vd->curknot)) {
        return;
    }
    if (spix < 0) {
        return;
    }
    if (spix >= vd->sc->numsplines) {
        return;
    }
    if (vd->vspline[spix] == NULL) {
        return;
    }
    
    len = vd->sc->splines[spix].len;
    
    knotix = (knotix < len - 1) ? knotix : len - 1;
    
    if (knotix < 0) {
        return;
    } else if (knotix >= len) {
        return;
    }
    vd->curspline = spix;
    vd->curknot = knotix;

    for (knotix; knotix >= 0; knotix--) {
        if (vd->vspline[spix][knotix] != NULL) {
            i = 0;
            for (ix = 0; ix < vd->binfosize; ix++) {
                mask = ((u64 ***)vd->vspline)[spix][knotix][ix];
                for (n = 0; n < 64; n++) {
                    vd->sc->instances[i++].flags.visitest = mask & 1;
                    mask >>= 1;
                    if (i >= vd->sc->numinstance)
                        break;
                }
                if (i >= vd->sc->numinstance) {
                    return;
                }
            }
            return;
        }
    }
}

//NGC MATCH
static s32 PtInsideFBound(struct nuvec_s *wpos,s32 spcnt,struct nuvec_s *pts) {
  struct nuvec_s* curr;
  struct nuvec_s* prev;
  struct nuvec_s ipnt;
  s32 icnt;
  s32 n;
  struct nuplane_s p;

  prev = &pts[spcnt - 1];
  curr = pts;
  p.a = 0.0f;
  p.b = 0.0f;
  p.c = 1.0f;
  p.d = -((wpos->x * p.a) + (wpos->y * p.b) + (wpos->z * p.c));
  for (icnt = 0, n = 0; icnt < spcnt; icnt++, curr++) {
      if ((NuPlnLine(&p,prev,curr,&ipnt) != 0) && (ipnt.x >= wpos->x)) {
        n++;
      }
      prev = curr;
 }
  return n & 1;
}

struct visbndassoc_s {
    s32 visspline;
    s32 bndspline;
    s32 visstart_ix;
    s32 visend_ix;
};

struct visidata_s* visiLoadData(char* fname, struct nugscn_s* sc, union variptr_u* buff) {
    struct visidata_s* vd;
    s32* fdatabase;
    s32* fdata;
    s32 numassoc;
    s32 numbnd;
    s32 splen;
    struct nugobj_s* g;
    struct nuvec_s wpos;
    s32 gcnt;
    s32 n;
    s32 m;
    s32 o;
    s32 p;
    s32 tmpsize;
    s32 spline_used;
    struct visbndassoc_s* assocs;
    unsigned long long* tmpalloc;
    unsigned long long* binfo;
    s32 binfosize;
    s32 ver;
    unsigned long long* binfo_data;
    s32 allocSize;
    s32 binfobytes;
    unsigned long long deltatot;
    unsigned long long delta;

    fdatabase = (s32*)NuFileLoad(fname);
    vd = NULL;
    if (fdatabase != NULL) {
        vd = (struct visidata_s*)buff->voidptr;
        fdata = fdatabase + 1;
        buff->intaddr += sizeof(struct visidata_s);
        memset(vd, 0, sizeof(struct visidata_s));

        vd->sc = sc;
        vd->curknot = -1;
        vd->curspline = -1;

        vd->vspline = (void***)buff->voidptr;
        buff->u32 += sc->numsplines;

        ver = fdatabase[0];
        if (ver < 0) {
            numbnd = fdatabase[1];
            fdata++;
        } else {
            numbnd = ver;
        }

        binfosize = (vd->sc->numinstance + 63) / 64;
        vd->binfosize = binfosize;

        allocSize = binfosize * numbnd * 8;
        tmpalloc = (unsigned long long*)NuMemAlloc(allocSize);
        if (tmpalloc == NULL) {
            NuErrorProlog(__FILE__, __LINE__)("unable to alloc %d bytes", allocSize);
        }

        binfobytes = binfosize * 8;

        for (o = 0; o < numbnd; o++) {
            unsigned long long mask;
            unsigned long long* base;

            splen = *fdata;
            fdata++;

            base = &tmpalloc[o * binfosize];
            base[0] = 0;

            mask = 1;
            for (n = 0; n < vd->sc->numinstance; n++) {
                g = sc->gobjs[sc->instances[n].objid];
                if (g->ngobjs > 0) {
                    s32 ngobj = g->ngobjs;
                    unsigned long long* curbase = base;
                    do {
                        NuVecMtxTransform(&wpos, &g->bounding_box_center, (struct Mtx*)&vd->sc->instances[n].mtx);
                        if (PtInsideFBound(&wpos, splen, (struct nuvec_s*)fdata)) {
                            *curbase |= mask;
                        }
                        g++;
                        ngobj--;
                    } while (ngobj != 0);
                }

                {
                    unsigned long long newmask = mask << 1;
                    mask = newmask;
                    if (mask == 0) {
                        mask = 1;
                        base++;
                    }
                }
            }

            fdata = (s32*)((char*)fdata + splen * sizeof(struct nuvec_s));
        }

        numassoc = *fdata;
        assocs = (struct visbndassoc_s*)(fdata + 1);

        if (ver < 0) {
            s32 numsplinenames;
            s32* spix;
            char* spname;
            s32* splused;

            splused = (s32*)((char*)assocs + numassoc * sizeof(struct visbndassoc_s));
            numsplinenames = *splused;

            if (numsplinenames != 0) {
                s32 ix;
                spix = (s32*)NuMemAlloc(numsplinenames * 4);
                spname = (char*)(splused + 1);

                for (ix = 0; (u32)ix < (u32)numsplinenames; ix++) {
                    spix[ix] = -1;

                    for (n = 0; n < sc->numsplines; n++) {
                        if (strcmp(sc->splines[n].name, spname) == 0) {
                            spix[ix] = n;
                            break;
                        }
                    }

                    while (*spname != '\0') {
                        spname++;
                    }
                    spname++;
                }

                if (numassoc > 0) {
                    struct visbndassoc_s* ap = assocs;
                    s32 cnt = numassoc;
                    do {
                        s32 resolved = spix[ap->visspline];
                        ap->visspline = resolved;
                        if (resolved < 0) {
                            ap->visspline = 0;
                            ap->visstart_ix = 0;
                            ap->visend_ix = 1;
                        }
                        ap++;
                        cnt--;
                    } while (cnt != 0);
                }

                NuMemFree(spix);
            }
        }

        binfo = (unsigned long long*)NuMemAlloc(binfobytes);
        tmpsize = 0;
        binfo_data = NULL;

        if (binfo == NULL) {
            NuErrorProlog(__FILE__, __LINE__)("unable to alloc %d bytes", binfobytes);
        }

        for (o = 0; o < vd->sc->numsplines; o++) {
            spline_used = 0;

            for (m = 0; m < numassoc; m++) {
                if (assocs[m].visspline != o) {
                    continue;
                }

                if (spline_used == 0) {
                    s32 needed;
                    s32 splinelen;

                    splinelen = vd->sc->splines[o].len;
                    needed = splinelen * binfosize;

                    if (needed > tmpsize) {
                        if (binfo_data != NULL) {
                            NuMemFree(binfo_data);
                        }
                        splinelen = vd->sc->splines[o].len;
                        tmpsize = splinelen * binfosize + 1;
                        binfo_data = (unsigned long long*)NuMemAlloc(tmpsize * 8);
                        if (binfo_data == NULL) {
                            NuErrorProlog(__FILE__, __LINE__)("unable to alloc %d bytes", tmpsize * 8);
                        }
                    }
                    memset(binfo_data, 0, tmpsize * 8);
                }

                spline_used = 1;

                {
                    s32 start_ix;
                    s32 end_ix;
                    s32 splinelen;

                    splinelen = vd->sc->splines[o].len;
                    start_ix = assocs[m].visstart_ix;
                    if (splinelen > start_ix) {
                        ;
                    } else {
                        start_ix = splinelen;
                    }
                    assocs[m].visstart_ix = start_ix;

                    end_ix = assocs[m].visend_ix;
                    splinelen = vd->sc->splines[o].len;
                    if (splinelen > end_ix) {
                        ;
                    } else {
                        end_ix = splinelen;
                    }

                    n = start_ix;
                    assocs[m].visend_ix = end_ix;

                    if (n > end_ix) {
                        continue;
                    }

                    do {
                        for (p = 0; p < binfosize; p++) {
                            binfo_data[n * binfosize + p] |= tmpalloc[assocs[m].bndspline * binfosize + p];
                        }
                        n++;
                    } while (n <= end_ix);
                }
            }

            if (spline_used != 0) {
                vd->vspline[o] = (void**)buff->voidptr;

                {
                    s32 splinelen;
                    splinelen = vd->sc->splines[o].len;
                    memset(vd->vspline[o], 0, splinelen * 8);
                }

                buff->intaddr += vd->sc->splines[o].len * 8;
                buff->intaddr = (buff->intaddr + 7) & ~7;

                for (n = 0; n < vd->sc->splines[o].len; n++) {
                    deltatot = 0;
                    for (p = 0; p < binfosize; p++) {
                        if (n == 0) {
                            delta = binfo_data[p];
                        } else {
                            delta = binfo_data[(n - 1) * binfosize + p] ^ binfo_data[n * binfosize + p];
                        }
                        binfo[p] = delta;
                        deltatot |= delta;
                    }

                    if (deltatot != 0) {
                        vd->vspline[o][n] = (void*)buff->u32;

                        for (p = 0; p < binfosize; p++) {
                            *buff->u64 = binfo_data[n * binfosize + p];
                            buff->u64++;
                        }
                    }
                }
            }
        }

        if (binfo_data != NULL) {
            NuMemFree(binfo_data);
        }

        if (ver != 0) {
            NuMemFree(binfo);
        }

        if (tmpalloc != NULL) {
            NuMemFree(tmpalloc);
        }

        NuMemFree(fdatabase);

        buff->intaddr = (buff->intaddr + 15) & ~15;
    }

    return vd;
}
