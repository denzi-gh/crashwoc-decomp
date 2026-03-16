#include "nufpar.h"
#include "nufile.h"
#include "numem.h"

#define LF 0xA	// '\n'
#define CR 0xD	// '\r'
// 0x3b ';'


s32 old_line_pos;

//99.78% NGC (regalloc: r3 vs r0 for buffend+bytesRead computation)
char NuGetChar(struct nufpar_s* fPar) {
    s32 bufferEndPos = fPar->buffend;
    s32 size;
    s32 tmp;

    if (bufferEndPos < 0) {
        bufferEndPos = 0;
    }
    if (fPar->cpos > fPar->buffend) {
        if (fPar->buffend + 1 < fPar->size) {
            size = fPar->size - bufferEndPos;
            tmp = NuFileRead(fPar->handle, fPar->fbuff, size > 0x1000 ? 0x1000 : size);
            fPar->buffstart = fPar->buffend + 1;
            fPar->buffend = fPar->buffend + tmp;
            if (tmp == 0) {
                return 0;
            }
        } else {
           return 0;
        }
    }
    {
        char ret = fPar->fbuff[fPar->cpos - fPar->buffstart];
        fPar->cpos++;
        return ret;
    }
}

//MATCH NGC
s32 NuFParGetWord(struct nufpar_s* fPar) {
    s32 len = 0;
    s32 inquotes = 0;
    char chr;
    
    old_line_pos = fPar->line_pos;
    while (fPar->lbuff[(fPar->line_pos) & 0xFF] != 0) {
        chr = fPar->lbuff[fPar->line_pos];
        switch (chr) {
        case 9:
        case 0x20:
        case 0x2c:
            if (inquotes == 0) {
                if (len != 0) {
                    (fPar->wbuff)[len & 0xFF] = 0;
                    return len;
                }
              break;
            }
        default:
                if (chr == 0x22) {
                    inquotes = 1 - inquotes;
                    break;
                } else {
                    fPar->wbuff[len & 0xFF] = chr;
                    len++;
                    break;
                }
            break;
        }
        fPar->line_pos++;
    }
    fPar->wbuff[len & 0xFF] = 0;
    return len;
}

//MATCH NGC
s32 NuFParGetInt(struct nufpar_s* fPar)
{
    NuFParGetWord(fPar);
    if (fPar->wbuff[0] != 0)
    {
        return atoi(fPar->wbuff);
    }
    return 0;
}

//MATCH NGC
s32 NuFParPushCom(struct nufpar_s* fPar, struct nufpcomjmp_s* jmp)
{
    s32 ind = fPar->compos;
    if (ind > 6)
    {
        return -1;
    }
    fPar->compos = ind + 1;
    fPar->comstack[ind + 1] = jmp;
    return fPar->compos;
}

//MATCH NGC
void NuFParClose(struct nufpar_s* fPar)
{
    NuMemFree(fPar);
}

//MATCH NGC
struct nufpar_s* NuFParOpen(s32 handle)
{
    struct nufpar_s* fPar = NuMemAlloc(sizeof(struct nufpar_s));	//size: 0x1244
    if (fPar != NULL)
    {
        s32 originalPos;

        memset(fPar, 0, sizeof(struct nufpar_s));
        fPar->handle = handle;
        fPar->buffend = -1;
        fPar->line_num = -1;
        fPar->compos = -1;
        originalPos = NuFilePos(handle);
        NuFileSeek(handle, 0, 2);
        fPar->size = filelength;
        NuFileSeek(handle, originalPos, 0);
    }

    return fPar;
}

//MATCH NGC
void NuFParDestroy(struct nufpar_s* fPar)
{
    fileHandle handle = fPar->handle;
    NuFParClose(fPar);
    NuFileClose(handle);
}

//MATCH NGC
struct nufpar_s* NuFParCreate(char* filename)
{
    s32 handle = NuFileOpen(filename, 0); //0= NUFILE_READ
    if (handle != 0)
    {
        struct nufpar_s* fPar = NuFParOpen(handle);
        if (fPar != NULL)
        {
            return fPar;
        }
        NuFileClose(handle);
    }
    return NULL;
}

//96.92% NGC (scheduler: li r30/lwz r9 order swapped at comment exit)
s32 NuFParGetLine(struct nufpar_s* fPar) {
    s32 i = 0;
    char ch;
    s32 lineNum = fPar->line_num;
    char nul = 0;

    fPar->line_pos = 0;

    for (;;) {
        lineNum++;
        fPar->line_num = lineNum;

        for (;;) {
            ch = NuGetChar(fPar);
            switch (ch) {
            case 0:
                goto exit_func;
            case CR:
                NuGetChar(fPar);
            case LF:
                if (i != 0) {
                    goto exit_func;
                }
                lineNum = fPar->line_num;
                break;
            exit_func:
                fPar->lbuff[i] = nul;
                return i;
            case 0x3B:
                if (i == 0) {
                    {
                        s32 done = 0;
                        do {
                            ch = NuGetChar(fPar);
                            switch (ch) {
                            case CR:
                                NuGetChar(fPar);
                            case 0:
                            case LF:
                                done = 1;
                                break;
                            }
                        } while (done == 0);
                        lineNum = fPar->line_num;
                    }
                    i = 0;
                    fPar->line_pos = i;
                    break;
                }
            default:
                fPar->lbuff[i & 0xFF] = ch;
                i++;
                continue;
            }
            break;
        }
    }
}

//MATCH NGC
s32 NuFParInterpretWord(struct nufpar_s* fPar) {
    s32 i;
    if (fPar->compos >= 0) {
        for (i = 0; fPar->comstack[fPar->compos][i].fname != NULL; i++) {
            if (strcasecmp((fPar->comstack[fPar->compos][i].fname), fPar->wbuff) == 0) {
                fPar->comstack[fPar->compos][i].func(fPar);
                return 1;
            }
        }
    }
    return 0;
}

