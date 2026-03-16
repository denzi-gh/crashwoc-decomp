#include "types.h"

/* OS function declarations */
s32 OSDisableInterrupts(void);
s32 OSRestoreInterrupts(s32 level);
void __OSMaskInterrupts(u32 mask);
void __OSUnmaskInterrupts(u32 mask);
void* __OSSetInterruptHandler(s32 interrupt, void* handler);

/* Static data (.sdata) */
static u8 SendCount = 0x80;

/* Static BSS (.sbss) - order determines memory layout */
static u32 MTRCallback;
static u32 DBGCallback;
static u32 SendMailData;
static u32 RecvDataLeng;
static u8* pEXIInputFlag;
static u8 EXIInputFlag;

static int DBGEXIImm(u8* data, s32 len, s32 type) {
    int i;
    u32 val;

    if (type) {
        val = 0;
        for (i = 0; i < len; i++) {
            val |= (u32)data[i] << ((3 - i) * 8);
        }
        *(volatile u32*)0xCC006838 = val;
    }

    *(volatile u32*)0xCC006834 = ((type << 2) | 1) | ((len - 1) << 4);
    while (*(volatile u32*)0xCC006834 & 1) ;

    if (!type) {
        val = *(volatile u32*)0xCC006838;
        for (i = 0; i < len; i++) {
            data[i] = (u8)(val >> ((3 - i) * 8));
        }
    }

    return 1;
}

static int DBGReadMailbox(u32* data) {
    volatile u32* exi = (volatile u32*)0xCC006800;
    u32 cmd;
    int err;

    u32 csr = exi[0x28 / 4];
    csr = (csr & 0x405) | 0xc0;
    exi[0x28 / 4] = csr;

    cmd = 0x60000000;
    err = !DBGEXIImm((u8*)&cmd, 2, 1);

    while (exi[0x34 / 4] & 1) ;

    err |= !DBGEXIImm((u8*)data, 4, 0);

    while (exi[0x34 / 4] & 1) ;

    csr = exi[0x28 / 4];
    exi[0x28 / 4] = csr & 0x405;

    return !err;
}

static int DBGRead(s32 cmd, u8* dest, s32 len) {
    volatile u32* exi = (volatile u32*)0xCC006800;
    u32 cmd_buf;
    int err;

    u32 csr = exi[0x28 / 4];
    csr = (csr & 0x405) | 0xc0;
    exi[0x28 / 4] = csr;

    cmd_buf = ((u32)(cmd << 8) & 0x01FFFC00) | 0x20000000;
    err = !DBGEXIImm((u8*)&cmd_buf, 4, 1);

    while (exi[0x34 / 4] & 1) ;

    while (len != 0) {
        u32 tmp;
        err |= !DBGEXIImm((u8*)&tmp, 4, 0);
        while (exi[0x34 / 4] & 1) ;
        *(u32*)dest = tmp;
        dest += 4;
        len -= 4;
        if (len < 0) len = 0;
    }

    csr = exi[0x28 / 4];
    exi[0x28 / 4] = csr & 0x405;

    return !err;
}

static int DBGWrite(s32 cmd, u8* src, s32 len) {
    volatile u32* exi = (volatile u32*)0xCC006800;
    u32 cmd_buf;
    int err;

    u32 csr = exi[0x28 / 4];
    csr = (csr & 0x405) | 0xc0;
    exi[0x28 / 4] = csr;

    cmd_buf = ((u32)(cmd << 8) & 0x01FFFC00) | 0xA0000000;
    err = !DBGEXIImm((u8*)&cmd_buf, 4, 1);

    while (exi[0x34 / 4] & 1) ;

    while (len != 0) {
        u32 tmp = *(u32*)src;
        err |= !DBGEXIImm((u8*)&tmp, 4, 1);
        while (exi[0x34 / 4] & 1) ;
        src += 4;
        len -= 4;
        if (len < 0) len = 0;
    }

    csr = exi[0x28 / 4];
    exi[0x28 / 4] = csr & 0x405;

    return !err;
}

static int DBGReadStatus(u32* data) {
    volatile u32* exi = (volatile u32*)0xCC006800;
    u32 cmd;
    int err;

    u32 csr = exi[0x28 / 4];
    csr = (csr & 0x405) | 0xc0;
    exi[0x28 / 4] = csr;

    cmd = 0x40000000;
    err = !DBGEXIImm((u8*)&cmd, 2, 1);

    while (exi[0x34 / 4] & 1) ;

    err |= !DBGEXIImm((u8*)data, 4, 0);

    while (exi[0x34 / 4] & 1) ;

    csr = exi[0x28 / 4];
    exi[0x28 / 4] = csr & 0x405;

    return !err;
}

static void MWCallback(s16 irq) {
    EXIInputFlag = 1;
    if (MTRCallback) {
        ((void (*)(s32))MTRCallback)(0);
    }
}

static void DBGHandler(s16 irq) {
    *(volatile u32*)0xCC003000 = 0x1000;
    if (DBGCallback) {
        ((void (*)(s16))DBGCallback)(irq);
    }
}

void DBInitComm(u8** ppFlag, u32 callback) {
    s32 level = OSDisableInterrupts();
    pEXIInputFlag = &EXIInputFlag;
    *ppFlag = pEXIInputFlag;
    MTRCallback = callback;
    __OSMaskInterrupts(0x18000);
    *(volatile u32*)0xCC006828 = 0;
    OSRestoreInterrupts(level);
}

void DBInitInterrupts(void) {
    __OSMaskInterrupts(0x18000);
    __OSMaskInterrupts(0x40);
    DBGCallback = (u32)MWCallback;
    __OSSetInterruptHandler(25, (void*)DBGHandler);
    __OSUnmaskInterrupts(0x40);
}

s32 DBQueryData(void) {
    s32 level = 0;
    u32 status;

    EXIInputFlag = 0;

    if (RecvDataLeng == 0) {
        level = OSDisableInterrupts();

        DBGReadStatus(&status);
        if (status & 1) {
            DBGReadMailbox(&status);
            status &= 0x1FFFFFFF;

            if ((status & 0x1F000000) == 0x1F000000) {
                SendMailData = status;
                RecvDataLeng = status & 0x7FFF;
                EXIInputFlag = 1;
            }
        }
    }

    OSRestoreInterrupts(level);

    return RecvDataLeng;
}

s32 DBRead(u8* data, s32 len) {
    s32 level = OSDisableInterrupts();
    s32 cmd;

    if (SendMailData & 0x10000) {
        cmd = 0x1000;
    } else {
        cmd = 0;
    }

    DBGRead(cmd + 0x1E000, data, (len + 3) & ~3);

    RecvDataLeng = 0;
    EXIInputFlag = 0;

    OSRestoreInterrupts(level);
    return 0;
}

s32 DBWrite(u8* data, s32 len) {
    s32 level;
    volatile u32* exi;
    u32 status;
    u32 cmd_buf;
    int err;
    s32 cmd;
    s32 roundedLen;

    level = OSDisableInterrupts();
    exi = (volatile u32*)0xCC006800;

    /* Wait until device is ready (status bit 1 clear) */
    do {
        u32 csr = *(volatile u32*)0xCC006828;
        volatile u32* csr_ptr = (volatile u32*)0xCC006828;
        csr &= 0x405;
        csr |= 0xc0;
        *csr_ptr = csr;

        cmd_buf = 0x40000000;
        err = !DBGEXIImm((u8*)&cmd_buf, 2, 1);

        while (exi[0x34 / 4] & 1) ;

        DBGEXIImm((u8*)&status, 4, 0);

        while (exi[0x34 / 4] & 1) ;

        csr = *(volatile u32*)0xCC006828;
        csr &= 0x405;
        *(volatile u32*)0xCC006828 = csr;
    } while (status & 2);

    /* Increment send count */
    SendCount++;
    if (SendCount & 1) {
        cmd = 0x1000;
    } else {
        cmd = 0;
    }

    roundedLen = (len + 3) & ~3;
    cmd = (cmd | 0x10000) | 0xC000;

    /* Send data - retry until success */
    do {
        err = DBGWrite(cmd, data, roundedLen);
    } while (err == 0);

    /* Write mailbox - wait for ack */
    do {
        u32 csr;
        volatile u32* csr_ptr = (volatile u32*)0xCC006828;

        csr = *csr_ptr;
        csr &= 0x405;
        csr |= 0xc0;
        *csr_ptr = csr;

        cmd_buf = 0x40000000;
        err = !DBGEXIImm((u8*)&cmd_buf, 2, 1);

        while (exi[0x34 / 4] & 1) ;

        DBGEXIImm((u8*)&status, 4, 0);

        while (exi[0x34 / 4] & 1) ;

        csr = *csr_ptr;
        csr &= 0x405;
        *csr_ptr = csr;
    } while (status & 2);

    /* Send mail notification */
    {
        u32 mail;
        u8 cnt = SendCount;
        mail = ((u32)cnt << 16) | 0x1F000000 | (u32)len;
        mail &= 0x1FFFFFFF;
        mail |= 0xC0000000;

        do {
            u32 csr;
            volatile u32* csr_ptr = (volatile u32*)0xCC006828;

            csr = *csr_ptr;
            csr &= 0x405;
            csr |= 0xc0;
            *csr_ptr = csr;

            err = !DBGEXIImm((u8*)&mail, 4, 1);

            while (exi[0x34 / 4] & 1) ;

            csr = *csr_ptr;
            csr &= 0x405;
            *csr_ptr = csr;
        } while (!err);

        /* Wait for status ack */
        do {
            u32 csr;
            volatile u32* csr_ptr = (volatile u32*)0xCC006828;

            csr = *csr_ptr;
            csr &= 0x405;
            csr |= 0xc0;
            *csr_ptr = csr;

            cmd_buf = 0x40000000;
            err = !DBGEXIImm((u8*)&cmd_buf, 2, 1);

            while (exi[0x34 / 4] & 1) ;

            DBGEXIImm((u8*)&status, 4, 0);

            while (exi[0x34 / 4] & 1) ;

            csr = *csr_ptr;
            csr &= 0x405;
            err = 0;
            *csr_ptr = csr;
        } while (err || (status & 2));
    }

    OSRestoreInterrupts(level);
    return 0;
}

void DBOpen(void) {
}

void DBClose(void) {
}

void EXI2_Init(void) {
}

void EXI2_EnableInterrupts(void) {
}

s32 EXI2_Poll(void) {
    return 0;
}

s32 EXI2_ReadN(void) {
    return 0;
}

s32 EXI2_WriteN(void) {
    return 0;
}

void EXI2_Reserve(void) {
}

void EXI2_Unreserve(void) {
}
