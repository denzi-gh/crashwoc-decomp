#include "string.h"

struct DVDDiskID;

typedef void (*CARDCallback)(long chan, long result);

struct CARDDir {
    unsigned char gameName[4];
    unsigned char company[2];
    unsigned char _padding0;
    unsigned char bannerFormat;
    unsigned char fileName[32];
    unsigned long time;
    unsigned long iconAddr;
    unsigned short iconFormat;
    unsigned short iconSpeed;
    unsigned char permission;
    unsigned char copyTimes;
    unsigned short startBlock;
    unsigned short length;
    unsigned char _padding1[2];
    unsigned long commentAddr;
};

struct CARDFileInfo {
    long chan;
    long fileNo;
    long offset;
    long length;
    unsigned short iBlock;
};

struct CARDControl {
    int attached;
    long result;
    unsigned short size;
    char _pad0A[2];
    long sectorSize;
    unsigned short cBlock;
    char _pad12[2];
    long latency;
    unsigned char id[12];
    int mountStep;
    unsigned long scramble;
    char _pad2C[4];
    char task[0x50];
    void *workArea;
    struct CARDDir *currentDir;
    unsigned short *currentFat;
    char threadQueue[8];
    unsigned char cmd[9];
    char _pad9D[3];
    long cmdlen;
    unsigned long mode;
    int retry;
    int repeat;
    unsigned long addr;
    void *buffer;
    long xferred;
    unsigned short freeNo;
    unsigned short startBlock;
    struct CARDFileInfo *fileInfo;
    CARDCallback extCallback;
    CARDCallback txCallback;
    CARDCallback exiCallback;
    CARDCallback apiCallback;
    CARDCallback xferCallback;
    CARDCallback eraseCallback;
    CARDCallback unlockCallback;
    char alarm[0x28];
    unsigned long cid;
    struct DVDDiskID *diskID;
};

extern struct CARDControl __CARDBlock[2];

struct CARDDir *__CARDGetDirBlock(struct CARDControl *card);
unsigned short *__CARDGetFatBlock(struct CARDControl *card);
long __CARDGetControlBlock(long chan, struct CARDControl **pcard);
long __CARDPutControlBlock(struct CARDControl *card, long result);
long __CARDUpdateDir(long chan, CARDCallback callback);
long __CARDAllocBlock(long chan, unsigned long cBlock, CARDCallback callback);
int __CARDCompareFileName(struct CARDDir *ent, char *fileName);
long __CARDSync(long chan);
void __CARDDefaultApiCallback(long chan, long result);
void __CARDSyncCallback(long chan, long result);
long long OSGetTime(void);

static void CreateCallbackFat(long chan, long result) {
    struct CARDControl *card;
    CARDCallback callback;

    card = &__CARDBlock[chan];
    callback = card->apiCallback;
    card->apiCallback = 0;

    if (result < 0) goto error;

    {
        struct CARDDir *dir = __CARDGetDirBlock(card);
        struct CARDDir *ent = &dir[card->freeNo];

        memcpy(ent->gameName, card->diskID, 4);
        memcpy(ent->company, (unsigned char *)card->diskID + 4, 2);

        ent->permission = 4;
        ent->copyTimes = 0;
        ent->startBlock = card->startBlock;
        ent->bannerFormat = 0;
        ent->iconAddr = 0xFFFFFFFF;
        ent->iconFormat = 0;
        ent->iconSpeed = 0;
        ent->commentAddr = 0xFFFFFFFF;
        ent->iconSpeed = (ent->iconSpeed & ~3) | 1;

        card->fileInfo->offset = 0;
        card->fileInfo->iBlock = ent->startBlock;

        ent->time = (unsigned long)(OSGetTime() / (*(unsigned long *)0x800000F8 / 4));

        result = __CARDUpdateDir(chan, callback);
        if (result >= 0) return;
    }

error:
    __CARDPutControlBlock(card, result);
    if (callback) {
        callback(chan, result);
    }
}

long CARDCreateAsync(long chan, char *fileName, unsigned long length,
                     struct CARDFileInfo *fileInfo, CARDCallback callback) {
    struct CARDControl *card;
    struct CARDDir *dir;
    struct CARDDir *ent;
    unsigned short *fat;
    long freeSlot;
    long i;

    if (strlen(fileName) > 32) {
        return -12;
    }

    {
        long r = __CARDGetControlBlock(chan, &card);
        if (r < 0) {
            return r;
        }
    }

    if (length == 0 || length % card->sectorSize != 0) {
        return -128;
    }

    freeSlot = 0xFFFF;
    dir = __CARDGetDirBlock(card);
    ent = dir;

    for (i = 0; (unsigned short)i < 127; ent++, i++) {
        if (ent->gameName[0] == 0xFF) {
            if ((unsigned short)freeSlot == 0xFFFF) {
                freeSlot = i;
            }
        } else {
            if (memcmp(ent, card->diskID, 4) == 0) {
                if (memcmp(ent->company, (unsigned char *)card->diskID + 4, 2) == 0) {
                    if (__CARDCompareFileName(ent, fileName)) {
                        return __CARDPutControlBlock(card, -7);
                    }
                }
            }
        }
    }

    if ((unsigned short)freeSlot == 0xFFFF) {
        return __CARDPutControlBlock(card, -8);
    }

    fat = __CARDGetFatBlock(card);
    if (card->sectorSize * fat[3] < length) {
        return __CARDPutControlBlock(card, -9);
    }

    card->apiCallback = callback ? callback : __CARDDefaultApiCallback;

    ent = &dir[(unsigned short)freeSlot];
    card->freeNo = (unsigned short)freeSlot;
    ent->length = (unsigned short)(length / card->sectorSize);
    strncpy((char *)ent->fileName, fileName, 32);

    card->fileInfo = fileInfo;
    fileInfo->chan = chan;
    fileInfo->fileNo = (long)(unsigned short)freeSlot;

    {
        long result = __CARDAllocBlock(chan, length / card->sectorSize, CreateCallbackFat);
        if (result < 0) {
            return __CARDPutControlBlock(card, result);
        }
        return result;
    }
}

long CARDCreate(long chan, char *fileName, unsigned long length,
                struct CARDFileInfo *fileInfo) {
    long result;
    result = CARDCreateAsync(chan, fileName, length, fileInfo, __CARDSyncCallback);
    if (result < 0) {
        return result;
    }
    return __CARDSync(chan);
}

long __CARDSeek(struct CARDFileInfo *fileInfo, long length, long offset,
                struct CARDControl **pcard) {
    struct CARDControl *card;
    long result;

    result = __CARDGetControlBlock(fileInfo->chan, &card);
    if (result < 0) {
        return result;
    }

    if (fileInfo->iBlock < 5 || fileInfo->iBlock >= card->cBlock ||
        card->cBlock * card->sectorSize <= fileInfo->offset) {
        return __CARDPutControlBlock(card, -128);
    }

    {
        struct CARDDir *dir = __CARDGetDirBlock(card);
        struct CARDDir *ent = &dir[fileInfo->fileNo];
        long fileSize = card->sectorSize * ent->length;

        if (fileSize <= offset || fileSize < offset + length) {
            return __CARDPutControlBlock(card, -11);
        }

        card->fileInfo = fileInfo;
        fileInfo->length = length;

        if (offset < fileInfo->offset) {
            fileInfo->offset = 0;
            fileInfo->iBlock = ent->startBlock;

            if (fileInfo->iBlock < 5 || fileInfo->iBlock >= card->cBlock) {
                return __CARDPutControlBlock(card, -6);
            }
        }

        {
            unsigned short *fat = __CARDGetFatBlock(card);

            while ((unsigned long)fileInfo->offset <
                   ((unsigned long)offset & ~((unsigned long)card->sectorSize - 1))) {
                fileInfo->offset += card->sectorSize;
                fileInfo->iBlock = fat[fileInfo->iBlock];

                if (fileInfo->iBlock < 5 || fileInfo->iBlock >= card->cBlock) {
                    return __CARDPutControlBlock(card, -6);
                }
            }
        }

        fileInfo->offset = offset;
        *pcard = card;
        return 0;
    }
}
