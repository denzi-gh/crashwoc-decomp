static char labtab[64][21];

#include "nutxanm.h"
#include "../nucore/nufpar.h"
#include "../nucore/numem.h"

int strcasecmp(const char *, const char *);
char *strcpy(char *, const char *);
unsigned int strlen(const char *);

s32 NuFParGetLine(struct nufpar_s *fp);
s32 NuFParInterpretWord(struct nufpar_s *fp);
long NuRand(void *);

static void pftaTex(struct nufpar_s *fp);
static void pftaTexAdj(struct nufpar_s *fp);
static void pftaWait(struct nufpar_s *fp);
static void pftaRate(struct nufpar_s *fp);
static void pftaOn(struct nufpar_s *fp);
static void pftaOff(struct nufpar_s *fp);
static void pftaLabel(struct nufpar_s *fp);
static void pftaXDef(struct nufpar_s *fp);
static void pftaGoto(struct nufpar_s *fp);
static void pftaXRef(struct nufpar_s *fp);
static void pftaBtex(struct nufpar_s *fp);
static void pftaGosub(struct nufpar_s *fp);
static void pftaRet(struct nufpar_s *fp);
static void pftaRepeat(struct nufpar_s *fp);
static void pftaRepend(struct nufpar_s *fp);
static void pftaUntiltex(struct nufpar_s *fp);
static void pftaEnd(struct nufpar_s *fp);
static void pftaScriptname(struct nufpar_s *fp);

static int EvalVars(int cc, int a, int b);
static int ParGetCC(struct nufpar_s *fp);
static int LabTabFind(char *name);
static int XDefLabTabFind(char *name);
static void NuTexAnimXCall(int xdef_id, struct nutexanimenv_s *env);

static struct nufpcomjmp_s nutexanimcomtab[19] = {
    {"tex", pftaTex},
    {"texadj", pftaTexAdj},
    {"wait", pftaWait},
    {"rate", pftaRate},
    {"on", pftaOn},
    {"off", pftaOff},
    {"label", pftaLabel},
    {"goto", pftaGoto},
    {"btex", pftaBtex},
    {"gosub", pftaGosub},
    {"ret", pftaRet},
    {"repeat", pftaRepeat},
    {"repend", pftaRepend},
    {"untiltex", pftaUntiltex},
    {"end", pftaEnd},
    {"scriptname", pftaScriptname},
    {"xdef", pftaXDef},
    {"xref", pftaXRef},
    {NULL, NULL}
};

s32 fpointers[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void NuTexAnimProgSysInit(void) {
    int i;

    sys_progs = NULL;
    for (i = 0; i < 63; i++) {
        ntalsysbuff[i].succ = &ntalsysbuff[i + 1];
    }
    ntal_free = &ntalsysbuff[0];
    ntalsysbuff[63].succ = NULL;
    nta_sig_on = 0;
    ntal_first = NULL;
    xdeflabtabcnt = 0;
    nta_sig_old = 0;
    nta_sig_off = 0;
}

struct nutexanimprog_s *NuTexAnimProgFind(char *name) {
    struct nutexanimprog_s *p;

    p = sys_progs;
    while (p != NULL) {
        if (strcasecmp(name, p->name) == 0) {
            break;
        }
        p = p->succ;
    }
    return p;
}

void NuTexAnimProgInit(struct nutexanimprog_s *p) {
    int i;

    if (p == NULL) return;

    for (i = 0; i < 32; i++) {
        p->off_sig[i] = -1;
        p->on_sig[i] = -1;
    }
    p->eop = 0;
    p->name[0] = 0;
    p->xdef_cnt = 0;
    p->dynalloc = 0;
    p->on_mask = 0;
    p->off_mask = 0;
}

void NuTexAnimProgAssembleEnd(struct nutexanimprog_s *p) {
    int pc;
    short op;

    pc = 0;
    while (pc < (short)p->eop) {
        op = p->code[pc];
        switch (op) {
            case 1:
                pc += 4;
                break;
            case 2:
                pc += 3;
                break;
            case 5:
                pc += 3;
                break;
            case 9:
            {
                int label_pos = pc + 3;
                pc += 4;
                p->code[label_pos] = nta_labels[p->code[label_pos]];
                break;
            }
            case 7:
            case 8:
            {
                int label_pos = pc + 1;
                pc += 2;
                p->code[label_pos] = nta_labels[p->code[label_pos]];
                break;
            }
            case 0xa:
                pc += 1;
                break;
            case 0xb:
                pc += 3;
                break;
            case 0xc:
                pc += 1;
                break;
            case 0xd:
                pc += 3;
                break;
            case 0xe:
                pc += 1;
                break;
            case 0:
            case 0xf:
                pc += 2;
                break;
            default:
                break;
        }
    }
}

struct nutexanimenv_s *NuTexAnimEnvCreate(union variptr_u *buff, struct numtl_s *mtl, s16 *tids, struct nutexanimprog_s *p) {
    struct nutexanimenv_s *env;

    if (buff != NULL) {
        env = (struct nutexanimenv_s *)((buff->intaddr + 3) & ~3);
        buff->intaddr = (unsigned int)env + 0xEC;
    } else {
        env = (struct nutexanimenv_s *)NuMemAlloc(0xEC);
    }

    if (env != NULL) {
        env->prog = p;
        env->mtl = mtl;
        env->tids = tids;
        env->tex_ix = 0;
        env->pc = 0;
        env->rep_ix = 0;
        env->ra_ix = 0;
        env->pause = 0;
        env->pause_r = 0;
        env->pause_cnt = 0;
        if (buff != NULL) {
            env->dynalloc = 0;
        } else {
            env->dynalloc = 1;
        }
    }

    return env;
}

static int EvalVars(int cc, int a, int b) {
    switch (cc) {
        case 0:
            if (a == b) return 1;
            break;
        case 1:
            if (a < b) return 1;
            break;
        case 2:
            if (a > b) return 1;
            break;
        case 3:
            if (a <= b) return 1;
            break;
        case 4:
            if (a >= b) return 1;
            break;
        case 5:
            if (a != b) return 1;
            break;
        default:
            NuErrorProlog(__FILE__, 382)("unknown condition code %d", cc);
            break;
    }
    return 0;
}

static int ParGetCC(struct nufpar_s *fp) {
    int cc;

    NuFParGetWord(fp);
    switch ((char)*(u8 *)((char *)fp + 0x1105)) {
        case '=':
            cc = 0;
            break;
        case '<':
            switch ((char)*(u8 *)((char *)fp + 0x1106)) {
                case '=':
                    cc = 3;
                    break;
                case '>':
                    cc = 5;
                    break;
                default:
                    cc = 1;
                    break;
            }
            break;
        case '>':
            cc = 2;
            if ((char)*(u8 *)((char *)fp + 0x1106) == '=') {
                cc = 4;
            }
            break;
        case '!':
            cc = 5;
            break;
        default:
            NuErrorProlog(__FILE__, 428)("unknown condition '%s' at line %d",
                (char *)fp + 0x1105, *(s32 *)((char *)fp + 0x1208));
            break;
    }
    return cc;
}

static int LabTabFind(char *name) {
    int i;
    int cnt;

    if (strlen(name) > 20) {
        name[20] = 0;
    }

    i = 0;
    cnt = labtabcnt;
    while (i < labtabcnt) {
        if (strcasecmp(labtab[i], name) == 0) {
            return i;
        }
        i++;
        cnt = labtabcnt;
    }

    if (cnt > 63) {
        NuErrorProlog(__FILE__, 457)("Tex Anim Assembler Fatal Error: too many labels");
    }

    strcpy(labtab[labtabcnt], name);
    labtabcnt++;
    return labtabcnt - 1;
}

static int XDefLabTabFind(char *name) {
    int i;
    int cnt;

    if (strlen(name) > 20) {
        name[20] = 0;
    }

    i = 0;
    cnt = xdeflabtabcnt;
    while (i < xdeflabtabcnt) {
        if (strcasecmp(xdeflabtab[i], name) == 0) {
            return i;
        }
        i++;
        cnt = xdeflabtabcnt;
    }

    if (cnt > 255) {
        NuErrorProlog(__FILE__, 478)("Tex Anim Assembler Fatal Error: too many global labels");
    }

    strcpy(xdeflabtab[xdeflabtabcnt], name);
    xdeflabtabcnt++;
    return xdeflabtabcnt - 1;
}

static void pftaTex(struct nufpar_s *fp) {
    int tex;

    tex = NuFParGetInt(fp);
    parprog->code[parprog->eop++] = 0;
    parprog->code[parprog->eop++] = tex;
}

static void pftaTexAdj(struct nufpar_s *fp) {
    int a, b, c;

    a = NuFParGetInt(fp);
    b = NuFParGetInt(fp);
    c = NuFParGetInt(fp);
    parprog->code[parprog->eop++] = 1;
    parprog->code[parprog->eop++] = a;
    parprog->code[parprog->eop++] = b;
    parprog->code[parprog->eop++] = c;
}

static void pftaWait(struct nufpar_s *fp) {
    int a, b;

    a = NuFParGetInt(fp);
    b = NuFParGetInt(fp);
    parprog->code[parprog->eop++] = 2;
    parprog->code[parprog->eop++] = a;
    parprog->code[parprog->eop++] = b;
}

static void pftaRate(struct nufpar_s *fp) {
    int a, b;

    a = NuFParGetInt(fp);
    b = NuFParGetInt(fp);
    parprog->code[parprog->eop++] = 5;
    parprog->code[parprog->eop++] = a;
    parprog->code[parprog->eop++] = b;
}

static void pftaOn(struct nufpar_s *fp) {
    int sig;

    sig = NuFParGetInt(fp);
    parprog->on_sig[sig] = (short)parprog->eop;
    parprog->on_mask |= (1 << sig);
}

static void pftaOff(struct nufpar_s *fp) {
    int sig;

    sig = NuFParGetInt(fp);
    parprog->off_sig[sig] = (short)parprog->eop;
    parprog->off_mask |= (1 << sig);
}

static void pftaLabel(struct nufpar_s *fp) {
    int idx;

    NuFParGetWord(fp);
    idx = LabTabFind((char *)fp + 0x1105);
    nta_labels[idx] = (short)parprog->eop;
}

static void pftaXDef(struct nufpar_s *fp) {
    int idx;
    int cnt;

    NuFParGetWord(fp);
    idx = XDefLabTabFind((char *)fp + 0x1105);
    cnt = parprog->xdef_cnt;
    parprog->xdef_ids[cnt] = idx;
    parprog->xdef_addrs[cnt] = parprog->eop;
    parprog->xdef_cnt++;
}

static void pftaGoto(struct nufpar_s *fp) {
    int idx;

    NuFParGetWord(fp);
    idx = LabTabFind((char *)fp + 0x1105);
    parprog->code[parprog->eop++] = 7;
    parprog->code[parprog->eop++] = idx;
}

static void pftaXRef(struct nufpar_s *fp) {
    int idx;

    NuFParGetWord(fp);
    idx = XDefLabTabFind((char *)fp + 0x1105);
    parprog->code[parprog->eop++] = 0xf;
    parprog->code[parprog->eop++] = idx;
}

static void pftaBtex(struct nufpar_s *fp) {
    int cc, val, idx;

    cc = ParGetCC(fp);
    val = NuFParGetInt(fp);
    NuFParGetWord(fp);
    idx = LabTabFind((char *)fp + 0x1105);
    parprog->code[parprog->eop++] = 9;
    parprog->code[parprog->eop++] = cc;
    parprog->code[parprog->eop++] = val;
    parprog->code[parprog->eop++] = idx;
}

static void pftaGosub(struct nufpar_s *fp) {
    int idx;

    NuFParGetWord(fp);
    idx = LabTabFind((char *)fp + 0x1105);
    parprog->code[parprog->eop++] = 8;
    parprog->code[parprog->eop++] = idx;
}

static void pftaRet(struct nufpar_s *fp) {
    parprog->code[parprog->eop++] = 0xa;
}

static void pftaRepeat(struct nufpar_s *fp) {
    int count, rand_range;

    count = NuFParGetInt(fp);
    if (count == 0) {
        count = 0x7FFFFFFF;
    }
    rand_range = NuFParGetInt(fp);
    parprog->code[parprog->eop++] = 0xb;
    parprog->code[parprog->eop++] = count;
    parprog->code[parprog->eop++] = rand_range;
}

static void pftaRepend(struct nufpar_s *fp) {
    parprog->code[parprog->eop++] = 0xc;
}

static void pftaUntiltex(struct nufpar_s *fp) {
    int cc, val;

    cc = ParGetCC(fp);
    val = NuFParGetInt(fp);
    parprog->code[parprog->eop++] = 0xd;
    parprog->code[parprog->eop++] = cc;
    parprog->code[parprog->eop++] = val;
}

static void pftaEnd(struct nufpar_s *fp) {
    parprog->code[parprog->eop++] = 0xe;
}

static void pftaScriptname(struct nufpar_s *fp) {
    NuFParGetWord(fp);
    *((char *)fp + 0x1125) = 0;
    strcpy(parprog->name, (char *)fp + 0x1105);
}

struct nutexanimprog_s *NuTexAnimProgReadScript(union variptr_u *buff, char *fname) {
    struct nutexanimprog_s *p;
    struct nufpar_s *fp;
    s32 wordlen;
    s32 linelen;

    if (buff != NULL) {
        p = (struct nutexanimprog_s *)((buff->intaddr + 3) & ~3);
    } else {
        p = (struct nutexanimprog_s *)NuMemAlloc(0x400);
    }

    memset(labtab, 0, 0x540);
    p = NULL;
    labtabcnt = 0;

    fp = NuFParCreate(fname);
    if (fp == NULL) {
        return p;
    }

    NuFParPushCom(fp, nutexanimcomtab);
    p = (struct nutexanimprog_s *)((buff != NULL) ?
        (void *)((buff->intaddr + 3) & ~3) : (void *)NuMemAlloc(0x400));

    NuTexAnimProgInit(p);
    parprog = p;

    while ((linelen = NuFParGetLine(fp)) != 0) {
        wordlen = NuFParGetWord(fp);
        if (wordlen == 0) continue;
        if (NuFParInterpretWord(fp) != 0) continue;
        if (*(u8 *)((char *)fp + 0x1105) == 0) continue;

        {
            char *word = (char *)fp + 0x1105;
            if (word[wordlen - 1] == ':') {
                word[wordlen - 1] = 0;
                {
                    int idx = LabTabFind(word);
                    nta_labels[idx] = (short)parprog->eop;
                }
            }
        }
    }

    if (buff != NULL) {
        buff->intaddr = (unsigned int)p + 0x1B8 + (short)p->eop * 2;
    }

    NuFParDestroy(fp);
    NuTexAnimProgAssembleEnd(p);

    p->succ = sys_progs;
    if (sys_progs != NULL) {
        sys_progs->prev = p;
    }
    p->prev = NULL;
    sys_progs = p;

    return p;
}

static void NuTexAnimXCall(int xdef_id, struct nutexanimenv_s *env) {
    struct nutexanimlist_s *list;
    struct nutexanim_s *nta;
    struct nutexanimenv_s *other_env;
    struct nutexanimprog_s *prog;
    int i;
    int cnt;

    list = ntal_first;
    if (list == NULL) return;

    do {
        nta = list->nta;
        if (nta != NULL) {
            do {
                other_env = nta->env;
                if (other_env != NULL && other_env != env) {
                    prog = other_env->prog;
                    if (prog != NULL && prog->xdef_cnt > 0) {
                        cnt = prog->xdef_cnt;
                        for (i = 0; i < cnt; i++) {
                            if (prog->xdef_ids[i] == xdef_id) {
                                other_env->rep_ix = 0;
                                other_env->pc = prog->xdef_addrs[i];
                                other_env->pause_cnt = 0;
                                other_env->ra_ix = 0;
                                break;
                            }
                        }
                    }
                }
                nta = nta->succ;
            } while (nta != NULL);
        }
        list = list->succ;
    } while (list != NULL);
}

void NuTexAnimEnvProc(struct nutexanimenv_s *env) {
    struct nutexanimprog_s *prog;
    short *code;
    int done;
    int pc;
    int new_val;

    done = 0;
    prog = env->prog;
    if (prog == NULL) return;

    /* Check off signals */
    if (nta_sig_off & prog->off_mask) {
        int i;
        for (i = 0; i < 32; i++) {
            if (nta_sig_off & (1 << i) & prog->off_mask) {
                env->pause_cnt = 0;
                env->pc = prog->off_sig[i];
                env->ra_ix = 0;
                env->rep_ix = 0;
                break;
            }
        }
    }

    /* Check on signals */
    if (nta_sig_on & prog->on_mask) {
        int i;
        for (i = 0; i < 32; i++) {
            if (nta_sig_on & (1 << i) & prog->on_mask) {
                env->rep_ix = 0;
                env->pc = prog->on_sig[i];
                env->pause_cnt = 0;
                env->ra_ix = 0;
                break;
            }
        }
    }

    /* Check pause countdown */
    if (env->pause_cnt != 0) {
        env->pause_cnt--;
        return;
    }

    code = prog->code;

    do {
        pc = env->pc;
        switch (code[pc]) {
            case 0: /* tex */
                env->tex_ix = code[pc + 1];
                env->mtl->tid = env->tids[env->tex_ix];
                env->pc = pc + 2;
                env->pause_cnt = env->pause;
                if (env->pause_r != 0) {
                    env->pause_cnt += NuRand(0) % env->pause_r;
                }
                done = 1;
                break;

            case 1: /* texadj */
                new_val = env->tex_ix + code[pc + 1];
                if (new_val < code[pc + 2]) new_val = code[pc + 2];
                if (new_val > code[pc + 3]) new_val = code[pc + 3];
                env->tex_ix = new_val;
                env->mtl->tid = env->tids[env->tex_ix];
                env->pc = pc + 4;
                env->pause_cnt = env->pause;
                if (env->pause_r != 0) {
                    env->pause_cnt += NuRand(0) % env->pause_r;
                }
                done = 1;
                break;

            case 2: /* wait */
                env->pause_cnt = code[pc + 1];
                if (code[pc + 2] != 0) {
                    env->pause_cnt += NuRand(0) % code[pc + 2];
                }
                env->pc = pc + 3;
                done = 1;
                break;

            case 5: /* rate */
                env->pause = code[pc + 1];
                env->pause_r = code[pc + 2];
                env->pc = pc + 3;
                break;

            case 7: /* goto */
                env->pc = code[pc + 1];
                break;

            case 8: /* gosub */
                if (env->ra_ix > 15) {
                    NuErrorProlog(__FILE__, 904)("TexAnim Processor Alert: Call Stack Overflow at (%d)", env->pc);
                }
                env->ra[env->ra_ix] = pc + 2;
                env->ra_ix++;
                env->pc = code[pc + 1];
                break;

            case 9: /* btex */
                if (EvalVars(code[pc + 1], env->tex_ix, code[pc + 2])) {
                    env->pc = code[pc + 3];
                } else {
                    env->pc = pc + 4;
                }
                break;

            case 0xa: /* ret */
                if (env->ra_ix == 0) {
                    NuErrorProlog(__FILE__, 912)("TexAnim Processor Alert: Call Stack Underflow at (%d)", env->pc);
                }
                env->ra_ix--;
                env->pc = env->ra[env->ra_ix];
                break;

            case 0xb: /* repeat */
                if (env->rep_ix > 15) {
                    NuErrorProlog(__FILE__, 920)("TexAnim Processor Alert: Too Many Nested Repeat Loops at (%d)", env->pc);
                }
                env->rep_count[env->rep_ix] = code[pc + 1];
                if (code[pc + 2] != 0) {
                    env->rep_count[env->rep_ix] += NuRand(0) % code[pc + 2];
                }
                env->pc = pc + 3;
                env->rep_start[env->rep_ix] = env->pc;
                env->rep_ix++;
                break;

            case 0xc: /* repend */
                if (env->rep_ix == 0) {
                    NuErrorProlog(__FILE__, 930)("TexAnim Processor Alert: REPEND without REPEAT at (%d)", env->pc);
                }
                if (env->rep_count[env->rep_ix - 1] == 0) {
                    env->rep_ix--;
                    env->pc = pc + 1;
                } else {
                    env->pc = env->rep_start[env->rep_ix - 1];
                    env->rep_count[env->rep_ix - 1]--;
                }
                break;

            case 0xd: /* untiltex */
                if (env->rep_ix == 0) {
                    NuErrorProlog(__FILE__, 944)("TexAnim Processor Alert: UNTILTEX without REPEAT at (%d)", env->pc);
                }
                if (EvalVars(code[pc + 1], env->tex_ix, code[pc + 2])) {
                    env->pc = pc + 3;
                    env->rep_ix--;
                } else if (env->rep_count[env->rep_ix - 1] == 0) {
                    env->pc = env->rep_start[env->rep_ix - 1];
                    env->rep_count[env->rep_ix - 1]--;
                } else {
                    env->pc = pc + 3;
                    env->rep_ix--;
                }
                break;

            case 0xe: /* end */
                done = 1;
                break;

            case 0xf: /* xref */
                NuTexAnimXCall(code[pc + 1], env);
                env->pc = pc + 2;
                break;

            default:
                break;
        }
    } while (!done);
}

void NuTexAnimSetSignals(u32 sig) {
    u32 old;
    u32 diff;

    old = nta_sig_old;
    nta_sig_old = sig;
    diff = sig ^ old;
    nta_sig_off = diff & ~sig;
    nta_sig_on = diff & sig;
}

void NuTexAnimProcessList(struct nutexanim_s *nta) {
    while (nta != NULL) {
        if (nta->env != NULL) {
            NuTexAnimEnvProc(nta->env);
        }
        nta = nta->succ;
    }
}

void NuTexAnimAddList(struct nutexanim_s *nta) {
    struct nutexanimlist_s *entry;

    entry = ntal_free;
    if (entry == NULL) return;

    ntal_free = entry->succ;
    entry->prev = NULL;
    entry->nta = nta;
    entry->succ = ntal_first;
    if (ntal_first != NULL) {
        ntal_first->prev = entry;
    }
    ntal_first = entry;
}

void NuTexAnimRemoveList(struct nutexanim_s *nta) {
    struct nutexanimlist_s *cur;
    struct nutexanimlist_s *head;
    struct nutexanimlist_s *free_list;

    cur = ntal_first;
    if (cur == NULL) return;

    free_list = ntal_free;
    head = cur;

    while (cur != NULL) {
        if (cur->nta == nta) {
            if (cur->succ != NULL) {
                cur->succ->prev = cur->prev;
            }
            if (cur->prev != NULL) {
                cur->prev->succ = cur->succ;
            } else {
                head = cur->succ;
            }
            cur->succ = free_list;
            free_list = cur;
            break;
        }
        cur = cur->succ;
    }

    ntal_free = free_list;
    ntal_first = head;
}

void NuTexAnimProcess(void) {
    struct nutexanimlist_s *list;

    list = ntal_first;
    while (list != NULL) {
        NuTexAnimProcessList(list->nta);
        list = list->succ;
    }
}
