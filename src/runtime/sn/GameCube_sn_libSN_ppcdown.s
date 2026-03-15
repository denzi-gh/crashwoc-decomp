.include "macros.inc"
.file "GameCube_sn_libSN_ppcdown.s"

# .text
.text
.balign 4

.sym SNMemCpy, local

.fn pad_01_800D5020_text, local
    # SNMemCpy
    andi.   r6, r3, 0x3
    bne     bytecpy
    andi.   r6, r4, 0x3
    bne     bytecpy
    andi.   r6, r5, 0x3
    bne     bytecpy
    subi    r3, r3, 0x4
    subi    r4, r4, 0x4
    add     r5, r5, r4
.sym mclp, local
    lwzu    r6, 0x4(r4)
    stwu    r6, 0x4(r3)
    cmplw   r4, r5
    blt     mclp
    blr
.sym bytecpy, local
    subi    r3, r3, 0x1
    subi    r4, r4, 0x1
    add     r5, r5, r4
.sym bclp, local
    lbzu    r6, 0x1(r4)
    stbu    r6, 0x1(r3)
    cmplw   r4, r5
    blt     bclp
    blr

.sym DBcallback, local
    lis     r6, 0x8016
    ori     r6, r6, 0x56f0
    lwz     r5, 0x0(r6)
    cmpwi   r5, 0x0
    bne     dbret
    lwz     r5, 0x19c(r4)
    ori     r5, r5, 0x400
    stw     r5, 0x19c(r4)
    lis     r6, 0x8021
    ori     r6, r6, 0x7794
    li      r5, 0x1
    stw     r5, 0x6bc(r6)
    lwz     r10, 0x198(r4)
    lis     r7, 0x800d
    ori     r7, r7, 0xf684
    cmplw   r10, r7
    blt     notinRAS
    lis     r8, 0x800d
    ori     r8, r8, 0xf694
    cmplw   r10, r8
    bgt     notinRAS
    stw     r7, 0x198(r4)
    mr      r10, r7
.sym notinRAS, local
    lwz     r7, 0x0(r10)
    rlwinm  r9, r7, 0, 11, 5
    lis     r8, 0x7c00
    ori     r8, r8, 0xa6
    cmpw    r9, r8
    bne     dbret
    rlwinm  r7, r7, 14, 24, 28
    stw     r7, 0x6c4(r6)
    stw     r10, 0x6c0(r6)
.sym dbret, local
    blr

.sym EnableMetroTRKInterrupts, global
    mflr    r0
    stw     r0, 0x4(r1)
    stwu    r1, -0x10(r1)
    stw     r31, 0x8(r1)
    lis     r31, 0x8021
    ori     r31, r31, 0x7794
    li      r5, 0x0
    stw     r5, 0x6bc(r31)
    lwz     r0, 0x698(r31)
    mtlr    r0
    blrl
    mr      r4, r3
    lis     r3, 0x8016
    ori     r3, r3, 0x5720
    .4byte  0x4800A189          /* bl OSReport (external) */
    lwz     r31, 0x8(r1)
    addi    r1, r1, 0x10
    lwz     r0, 0x4(r1)
    mtlr    r0
    blr

.sym SNDebugInit, global
    mflr    r0
    stw     r0, 0x4(r1)
    stwu    r1, -0x10(r1)
    lis     r4, 0x8016
    ori     r4, r4, 0x5758
    cmpwi   r3, 0x2
    beq     jinit
    lis     r4, 0x8016
    ori     r4, r4, 0x5774
    cmpwi   r3, 0x3
    beq     jinit
    li      r3, 0x1
    addi    r1, r1, 0x10
    lwz     r0, 0x4(r1)
    mtlr    r0
    blr

.sym jinit, local
    stw     r31, 0x8(r1)
    lis     r31, 0x8021
    ori     r31, r31, 0x7794
    addi    r3, r31, 0x694
    li      r5, 0x1c
    bl      SNMemCpy
    li      r5, 0x0
    stw     r5, 0x6bc(r31)
    addi    r3, r31, 0x6b0
    stw     r5, 0x0(r3)
    lis     r4, 0x800d
    ori     r4, r4, 0x5078
    lwz     r0, 0x694(r31)
    mtlr    r0
    blrl
    lwz     r0, 0x6a8(r31)
    mtlr    r0
    blrl
    lis     r3, 0x4
    ori     r3, r3, 0x106c
    lis     r4, 0x0
    ori     r4, r4, 0x146c
    bl      hookvecs
    li      r3, 0x0
    stw     r3, 0x688(r31)
    stw     r3, 0x68c(r31)
    stw     r3, 0x690(r31)

.sym snexit, local
    lis     r5, 0x800d
    ori     r5, r5, 0xce08
    lis     r3, 0x8016
    ori     r3, r3, 0x5790
.sym nextlong, local
    lwz     r4, 0x0(r3)
    cmpwi   r4, -0x1
    beq     patchppchalt
    lwz     r6, 0x0(r5)
    cmplw   r6, r4
    bne     ppchaltdone
    addi    r3, r3, 0x4
    addi    r5, r5, 0x4
    b       nextlong

.sym patchppchalt, local
    lis     r3, 0x800d
    ori     r3, r3, 0xce08
    li      r31, 0x1
    stw     r31, 0xc(r3)

.sym ppchaltdone, local
    .4byte  0x4BFFFD09          /* bl external (PPCHalt area) */
    lis     r3, 0x800f
    ori     r3, r3, 0xf988
    li      r4, 0x100
    .4byte  0x480094AD          /* bl external */
    lis     r3, 0x800d
    ori     r3, r3, 0x4fe4
    li      r4, 0x100
    .4byte  0x4800949D          /* bl external */
    lis     r3, 0x800f
    ori     r3, r3, 0xf988
    li      r4, 0x100
    .4byte  0x48009525          /* bl external */
    lis     r3, 0x800d
    ori     r3, r3, 0x4fe4
    li      r4, 0x100
    .4byte  0x48009515          /* bl external */
    lwz     r31, 0x8(r1)
    addi    r1, r1, 0x10
    lwz     r0, 0x4(r1)
    mtlr    r0
    blr

.sym comms, local
    lis     r1, 0x8021
    ori     r1, r1, 0x7774
    lis     r31, 0x8021
    ori     r31, r31, 0x7794
    li      r3, 0x0
    stw     r3, 0x6b4(r31)
.sym wait1, local
    lwz     r0, 0x69c(r31)
    mtlr    r0
    blrl
    cmpwi   r3, 0x0
    beq     wait1
    stw     r3, 0x6b4(r31)
    cmplwi  r3, 0x1008
    ble     sk1
    lis     r3, 0x0
    ori     r3, r3, 0x1008
.sym sk1, local
    mr      r4, r3
    stw     r4, 0x6b8(r31)
    addi    r3, r31, 0x6cc
    lwz     r0, 0x6a0(r31)
    mtlr    r0
    blrl
    cmpwi   r3, 0x0
    beq     sker
    lis     r3, 0x8016
    ori     r3, r3, 0x57a8
    .4byte  0x48009FC1          /* bl external (OSReport) */
    b       comms
.sym sker, local
    lwz     r3, 0x6b4(r31)
    lwz     r4, 0x6b8(r31)
    subf    r3, r4, r3
    stw     r3, 0x6b4(r31)
.sym dun, local
    lbz     r3, 0x6cc(r31)
    cmplwi  r3, 0xd
    blt     cmdok
    lis     r3, 0x8016
    ori     r3, r3, 0x57c1
    .4byte  0x48009F95          /* bl external (OSReport) */
    b       comms
.sym cmdok, local
    slwi    r3, r3, 2
    lis     r4, 0x800d
    ori     r4, r4, 0x58ac
    lwzx    r5, r3, r4
    mtlr    r5
    blrl
    lwz     r5, 0x6bc(r31)
    cmpwi   r5, 0x0
    beq     comms
    b       cmdGo_entry

.sym commserr, local
    lis     r3, 0x8016
    ori     r3, r3, 0x57f8
    .4byte  0x48009F5D          /* bl external (OSReport) */
    mtlr    r30
    li      r3, 0x2
    blr

.sym cmdNop, global
    blr

.sym cmdStop, local
    mflr    r30
    addi    r3, r31, 0x6cc
    li      r4, 0x8
    lwz     r0, 0x6a4(r31)
    mtlr    r0
    blrl
    lis     r3, 0x8016
    ori     r3, r3, 0x5808
    li      r4, 0x8
    lwz     r0, 0x6a4(r31)
    mtlr    r0
    blrl
    li      r5, 0x0
    stw     r5, 0x6bc(r31)
    mtlr    r30
    li      r3, 0x0
    blr

.sym cmdRecvRegs, local
    mflr    r30
    li      r6, 0x6d0
    lwbrx   r3, r6, r31
    addi    r4, r31, 0x0
    add     r3, r3, r4
    addi    r4, r31, 0x6d4
    li      r6, 0x6ce
    lhbrx   r5, r6, r31
    bl      SNMemCpy
    addi    r3, r31, 0x6cc
    li      r4, 0x8
    lwz     r0, 0x6a4(r31)
    mtlr    r0
    blrl
    mtlr    r30
    li      r3, 0x0
    blr

.sym cmdSendRegs, local
    mflr    r30
    li      r6, 0x6d0
    lwbrx   r4, r6, r31
    addi    r5, r31, 0x0
    add     r4, r4, r5
    addi    r3, r31, 0x6d4
    li      r6, 0x6ce
    lhbrx   r5, r6, r31
    mr      r29, r5
    bl      SNMemCpy
    addi    r3, r31, 0x6cc
    mr      r4, r29
    addi    r4, r4, 0x8
    lwz     r0, 0x6a4(r31)
    mtlr    r0
    blrl
    cmpwi   r3, 0x0
    bne     commserr
    mtlr    r30
    li      r3, 0x0
    blr

.sym cmdRecvMem, global
    mflr    r30
    li      r6, 0x6d0
    lwbrx   r3, r6, r31
    addi    r4, r31, 0x6d4
    li      r6, 0x6ce
    lhbrx   r5, r6, r31
    lwz     r7, 0x390(r31)
    andis.  r7, r7, 0x1000
    bne     normalrecv
    lis     r7, 0xe000
    cmplw   r4, r7
    blt     normalrecv
    ori     r7, r7, 0x3fff
    cmplw   r4, r7
    bgt     normalrecv
    li      r6, 0x6ce
    li      r4, 0x0
    sthbrx  r4, r6, r31
    addi    r3, r31, 0x6cc
    li      r4, 0x8
    b       norecv
.sym normalrecv, local
    mr      r28, r3
    mr      r29, r5
    bl      SNMemCpy
    mr      r3, r28
    mr      r4, r29
    .4byte  0x48009241          /* bl external (ICFlashInvalidate) */
    mr      r3, r28
    mr      r4, r29
    .4byte  0x480092CD          /* bl external (DCFlushRangeNoSync) */
    addi    r3, r31, 0x6cc
.sym norecv, local
    li      r4, 0x8
    lwz     r0, 0x6a4(r31)
    mtlr    r0
    blrl
    mtlr    r30
    li      r3, 0x0
    blr

.sym cmdSendMem, global
    mflr    r30
    li      r6, 0x6ce
    lis     r3, 0x0
    ori     r3, r3, 0x1000
    lhbrx   r5, r6, r31
    cmplw   r5, r3
    ble     not2big
    mr      r5, r3
    stwbrx  r5, r6, r31
.sym not2big, local
    mr      r29, r5
    addi    r3, r31, 0x6d4
    li      r6, 0x6d0
    lwbrx   r4, r6, r31
    lwz     r7, 0x390(r31)
    andis.  r7, r7, 0x1000
    bne     cachelocked
    lis     r7, 0xe000
    cmplw   r4, r7
    blt     normalcopy
    ori     r7, r7, 0x3fff
    cmplw   r4, r7
    bgt     normalcopy
    li      r6, 0x6ce
    li      r4, 0x0
    sthbrx  r4, r6, r31
    addi    r3, r31, 0x6cc
    li      r4, 0x8
    b       nocopy
.sym normalcopy, local
    bl      SNMemCpy
    b       noclear
.sym cachelocked, local
    mr      r28, r4
    bl      SNMemCpy
    lis     r7, 0xe000
    cmplw   r4, r7
    blt     clearcache
    ori     r7, r7, 0x3fff
    cmplw   r4, r7
    ble     noclear
.sym clearcache, local
    mr      r3, r28
    mr      r4, r29
    .4byte  0x48009171          /* bl external (ICFlashInvalidate) */
.sym noclear, local
    addi    r3, r31, 0x6cc
    mr      r4, r29
    addi    r4, r4, 0x8
.sym nocopy, local
    lwz     r0, 0x6a4(r31)
    mtlr    r0
    blrl
    cmpwi   r3, 0x0
    bne     commserr
    mtlr    r30
    li      r3, 0x0
    blr

.sym cmdSendStatus, local
    mflr    r30
    lwz     r3, 0x6bc(r31)
    stw     r3, 0x6d4(r31)
    li      r4, 0x4
    li      r6, 0x6ce
    sthbrx  r4, r6, r31
    addi    r3, r31, 0x6cc
    addi    r4, r4, 0x8
    lwz     r0, 0x6a4(r31)
    mtlr    r0
    blrl
    cmpwi   r3, 0x0
    bne     commserr
    mtlr    r30
    li      r3, 0x0
    blr

.sym cmdGetModuleList, local
    mflr    r30
    li      r4, 0x0
    addi    r5, r31, 0x6d8
    lis     r6, 0x8000
    lwz     r6, 0x30c8(r6)
    cmpwi   r6, 0x0
    beq     alldone
    li      r7, 0x0
    li      r8, 0x4
    li      r9, 0x8
    li      r10, 0xc
.sym buildlist, local
    addi    r4, r4, 0x1
    stwbrx  r6, r7, r5
    lwz     r3, 0x0(r6)
    stwbrx  r3, r8, r5
    lwz     r3, 0x14(r6)
    stwbrx  r3, r9, r5
    lwz     r3, 0x18(r6)
    stwbrx  r3, r10, r5
    addi    r5, r5, 0x10
    cmpwi   r4, 0x40
    beq     alldone
    lwz     r6, 0x4(r6)
    cmpwi   r6, 0x0
    bne     buildlist
.sym alldone, local
    addi    r3, r31, 0x6d4
    li      r6, 0x0
    stwbrx  r4, r6, r3
    li      r6, 0x6ce
    subf    r4, r3, r5
    stwbrx  r4, r6, r31
    addi    r4, r4, 0x8
    addi    r3, r31, 0x6cc
    lwz     r0, 0x6a4(r31)
    mtlr    r0
    blrl
    cmpwi   r3, 0x0
    bne     commserr
    mtlr    r30
    li      r3, 0x0
    blr
    b       commserr

.sym cmdGetOSData, local
    mflr    r30
.sym cmdThreadList, local
    li      r4, 0x0
    addi    r5, r31, 0x6d8
    lis     r6, 0x8000
    lwz     r6, 0xdc(r6)
    cmpwi   r6, 0x0
    beq     allthreadsdone
    li      r7, 0x0
    li      r8, 0x4
    li      r9, 0x8
    li      r10, 0xc
    li      r11, 0x10
    li      r12, 0x14
    li      r14, 0x18
    li      r15, 0x1c
    li      r16, 0x20
.sym buildthreadlist, local
    addi    r4, r4, 0x1
    stwbrx  r6, r7, r5
    lwz     r3, 0x2c8(r6)
    stwbrx  r3, r8, r5
    lwz     r3, 0x2cc(r6)
    stwbrx  r3, r9, r5
    lwz     r3, 0x2d0(r6)
    stwbrx  r3, r10, r5
    lwz     r3, 0x2d4(r6)
    stwbrx  r3, r11, r5
    lwz     r3, 0x2d8(r6)
    stwbrx  r3, r12, r5
    lwz     r3, 0x2f0(r6)
    stwbrx  r3, r14, r5
    li      r18, 0x0
    lwz     r3, 0x2e8(r6)
    addi    r17, r5, 0x24
.sym nextjoin, local
    cmpwi   r3, 0x0
    beq     donejoin
    stwbrx  r3, r18, r17
    addi    r18, r18, 0x4
    lwz     r3, 0x2e0(r3)
    b       nextjoin
.sym donejoin, local
    add     r17, r17, r18
    srwi    r18, r18, 2
    stwbrx  r18, r15, r5
    li      r18, 0x0
    lwz     r3, 0x2f4(r6)
.sym nextmutex, local
    cmpwi   r3, 0x0
    beq     donemutex
    stwbrx  r3, r18, r17
    addi    r18, r18, 0x4
    lwz     r3, 0x10(r3)
    b       nextmutex
.sym donemutex, local
    add     r17, r17, r18
    srwi    r18, r18, 2
    stwbrx  r18, r16, r5
    mr      r5, r17
    cmpwi   r4, 0x40
    beq     allthreadsdone
    lwz     r6, 0x2fc(r6)
    cmpwi   r6, 0x0
    bne     buildthreadlist
.sym allthreadsdone, local
    addi    r3, r31, 0x6d4
    li      r6, 0x0
    stwbrx  r4, r6, r3
    li      r6, 0x6ce
    subf    r4, r3, r5
    sthbrx  r4, r6, r31
    addi    r4, r4, 0x8
    addi    r3, r31, 0x6cc
    lwz     r0, 0x6a4(r31)
    mtlr    r0
    blrl
    cmpwi   r3, 0x0
    bne     commserr
    mtlr    r30
    li      r3, 0x0
    blr

.sym cmdThreadContext, local
    li      r4, 0x0
    mtlr    r30
    li      r3, 0x0
    blr

.sym cmdGo_entry, local
.sym cmdGo, global
    lis     r5, 0xcc00
    li      r4, 0x1000
    stw     r4, 0x3000(r5)
    li      r5, 0x0
    stw     r5, 0x6bc(r31)
    mr      r3, r31
    bl      restorespr
    bl      restorefp
    lwz     r5, 0x360(r3)
    mtcrf   255, r5
    lwz     r5, 0x358(r3)
    mtlr    r5
    lwz     r5, 0x6c8(r3)
    lis     r4, 0xcc00
    sth     r5, 0x4010(r4)
    lis     r5, 0x8016
    ori     r5, r5, 0x56f0
    li      r4, 0x0
    stw     r4, 0x0(r5)
    lwz     r31, 0xf8(r3)
    lwz     r30, 0xf0(r3)
    lwz     r29, 0xe8(r3)
    lwz     r28, 0xe0(r3)
    lwz     r27, 0xd8(r3)
    lwz     r26, 0xd0(r3)
    lwz     r25, 0xc8(r3)
    lwz     r24, 0xc0(r3)
    lwz     r23, 0xb8(r3)
    lwz     r22, 0xb0(r3)
    lwz     r21, 0xa8(r3)
    lwz     r20, 0xa0(r3)
    lwz     r19, 0x98(r3)
    lwz     r18, 0x90(r3)
    lwz     r17, 0x88(r3)
    lwz     r16, 0x80(r3)
    lwz     r15, 0x78(r3)
    lwz     r14, 0x70(r3)
    lwz     r13, 0x68(r3)
    lwz     r12, 0x60(r3)
    lwz     r11, 0x58(r3)
    lwz     r10, 0x50(r3)
    lwz     r9, 0x48(r3)
    lwz     r8, 0x40(r3)
    lwz     r7, 0x38(r3)
    lwz     r6, 0x30(r3)
    lwz     r5, 0x28(r3)
    lwz     r4, 0x20(r3)
    lwz     r2, 0x10(r3)
    lwz     r1, 0x8(r3)
    lwz     r0, 0x0(r3)
    lwz     r3, 0x18(r3)
    rfi

.sym cmdtab, local
    .4byte  0x800D5370          /* cmdNop */
    .4byte  0x800D53B8          /* cmdRecvRegs */
    .4byte  0x800D53FC          /* cmdSendRegs */
    .4byte  0x800D5450          /* cmdRecvMem */
    .4byte  0x800D54E8          /* cmdSendMem */
    .4byte  0x800D57DC          /* cmdGo */
    .4byte  0x800D5374          /* cmdStop */
    .4byte  0x800D5370          /* cmdNop */
    .4byte  0x800D55B8          /* cmdSendStatus */
    .4byte  0x800D5370          /* cmdNop */
    .4byte  0x800D5370          /* cmdNop */
    .4byte  0x800D55F8          /* cmdGetModuleList */
    .4byte  0x800D56A0          /* cmdGetOSData */

.sym restorespr, local
    lwz     r5, 0x390(r3)
    mtspr   HID2, r5
    lwz     r5, 0x388(r3)
    mtspr   HID1, r5
    lwz     r5, 0x380(r3)
    mtspr   HID0, r5
    lwz     r5, 0x450(r3)
    mtspr   DABR, r5
    lwz     r5, 0x350(r3)
    mtctr   r5
    lwz     r5, 0x348(r3)
    mtxer   r5
    lwz     r5, 0x370(r3)
    mtspr   SRR0, r5
    lwz     r5, 0x378(r3)
    mtspr   SRR1, r5
    blr

.sym restorefp, local
    mfmsr   r5
    ori     r5, r5, 0x900
    xori    r5, r5, 0x900
    ori     r5, r5, 0x2000
    mtmsr   r5
    isync
    lis     r5, 0x8000
    lwz     r3, 0xd4(r5)
    lwz     r30, 0xd8(r5)
    cmpw    r3, r30
    beq     correctosfpregs
    mflr    r29
    .4byte  0x48009401          /* bl external (__OSSaveFPUContext) */
    cmpwi   r30, 0x0
    beq     noloadfpregs
    mr      r3, r30
    lhz     r30, 0x1a2(r3)
    ori     r4, r30, 0x1
    sth     r4, 0x1a2(r3)
    .4byte  0x480093DD          /* bl external (__OSLoadFPUContext) */
    sth     r30, 0x1a2(r3)
.sym noloadfpregs, local
    mtlr    r29
.sym correctosfpregs, local
    mr      r3, r31
    lfd     f0, 0x200(r3)
    mtfsf   255, f0
    mfspr   r5, HID2
    isync
    rlwinm. r5, r5, 3, 31, 31
    beq     normfp
    li      r5, 0x0
    mtspr   GQR0, r5
    isync
    psq_l   f0, 0x248(r3), 0, 0
    psq_l   f1, 0x250(r3), 0, 0
    psq_l   f2, 0x258(r3), 0, 0
    psq_l   f3, 0x260(r3), 0, 0
    psq_l   f4, 0x268(r3), 0, 0
    psq_l   f5, 0x270(r3), 0, 0
    psq_l   f6, 0x278(r3), 0, 0
    psq_l   f7, 0x280(r3), 0, 0
    psq_l   f8, 0x288(r3), 0, 0
    psq_l   f9, 0x290(r3), 0, 0
    psq_l   f10, 0x298(r3), 0, 0
    psq_l   f11, 0x2a0(r3), 0, 0
    psq_l   f12, 0x2a8(r3), 0, 0
    psq_l   f13, 0x2b0(r3), 0, 0
    psq_l   f14, 0x2b8(r3), 0, 0
    psq_l   f15, 0x2c0(r3), 0, 0
    psq_l   f16, 0x2c8(r3), 0, 0
    psq_l   f17, 0x2d0(r3), 0, 0
    psq_l   f18, 0x2d8(r3), 0, 0
    psq_l   f19, 0x2e0(r3), 0, 0
    psq_l   f20, 0x2e8(r3), 0, 0
    psq_l   f21, 0x2f0(r3), 0, 0
    psq_l   f22, 0x2f8(r3), 0, 0
    psq_l   f23, 0x300(r3), 0, 0
    psq_l   f24, 0x308(r3), 0, 0
    psq_l   f25, 0x310(r3), 0, 0
    psq_l   f26, 0x318(r3), 0, 0
    psq_l   f27, 0x320(r3), 0, 0
    psq_l   f28, 0x328(r3), 0, 0
    psq_l   f29, 0x330(r3), 0, 0
    psq_l   f30, 0x338(r3), 0, 0
    psq_l   f31, 0x340(r3), 0, 0
    lwz     r5, 0x208(r3)
    mtspr   GQR0, r5
    lwz     r5, 0x210(r3)
    mtspr   GQR1, r5
    lwz     r5, 0x218(r3)
    mtspr   GQR2, r5
    lwz     r5, 0x220(r3)
    mtspr   GQR3, r5
    lwz     r5, 0x228(r3)
    mtspr   GQR4, r5
    lwz     r5, 0x230(r3)
    mtspr   GQR5, r5
    lwz     r5, 0x238(r3)
    mtspr   GQR6, r5
    lwz     r5, 0x240(r3)
    mtspr   GQR7, r5
.sym normfp, local
    lfd     f0, 0x100(r3)
    lfd     f1, 0x108(r3)
    lfd     f2, 0x110(r3)
    lfd     f3, 0x118(r3)
    lfd     f4, 0x120(r3)
    lfd     f5, 0x128(r3)
    lfd     f6, 0x130(r3)
    lfd     f7, 0x138(r3)
    lfd     f8, 0x140(r3)
    lfd     f9, 0x148(r3)
    lfd     f10, 0x150(r3)
    lfd     f11, 0x158(r3)
    lfd     f12, 0x160(r3)
    lfd     f13, 0x168(r3)
    lfd     f14, 0x170(r3)
    lfd     f15, 0x178(r3)
    lfd     f16, 0x180(r3)
    lfd     f17, 0x188(r3)
    lfd     f18, 0x190(r3)
    lfd     f19, 0x198(r3)
    lfd     f20, 0x1a0(r3)
    lfd     f21, 0x1a8(r3)
    lfd     f22, 0x1b0(r3)
    lfd     f23, 0x1b8(r3)
    lfd     f24, 0x1c0(r3)
    lfd     f25, 0x1c8(r3)
    lfd     f26, 0x1d0(r3)
    lfd     f27, 0x1d8(r3)
    lfd     f28, 0x1e0(r3)
    lfd     f29, 0x1e8(r3)
    lfd     f30, 0x1f0(r3)
    lfd     f31, 0x1f8(r3)
    blr

.sym stashfp, local
    mfmsr   r5
    ori     r5, r5, 0x900
    xori    r5, r5, 0x900
    ori     r5, r5, 0x2000
    mtmsr   r5
    isync
    lis     r5, 0x8000
    lwz     r30, 0xd4(r5)
    lwz     r3, 0xd8(r5)
    cmpw    r3, r30
    beq     correctfpregs
    mflr    r29
    cmpwi   r3, 0x0
    beq     nosavefpregs
    .4byte  0x48009235          /* bl external (__OSSaveFPUContext) */
.sym nosavefpregs, local
    mr      r3, r30
    lhz     r30, 0x1a2(r3)
    ori     r4, r30, 0x1
    sth     r4, 0x1a2(r3)
    .4byte  0x48009219          /* bl external (__OSLoadFPUContext) */
    sth     r30, 0x1a2(r3)
    mtlr    r29
.sym correctfpregs, local
    mr      r3, r31
    stfd    f0, 0x100(r3)
    stfd    f1, 0x108(r3)
    stfd    f2, 0x110(r3)
    stfd    f3, 0x118(r3)
    stfd    f4, 0x120(r3)
    stfd    f5, 0x128(r3)
    stfd    f6, 0x130(r3)
    stfd    f7, 0x138(r3)
    stfd    f8, 0x140(r3)
    stfd    f9, 0x148(r3)
    stfd    f10, 0x150(r3)
    stfd    f11, 0x158(r3)
    stfd    f12, 0x160(r3)
    stfd    f13, 0x168(r3)
    stfd    f14, 0x170(r3)
    stfd    f15, 0x178(r3)
    stfd    f16, 0x180(r3)
    stfd    f17, 0x188(r3)
    stfd    f18, 0x190(r3)
    stfd    f19, 0x198(r3)
    stfd    f20, 0x1a0(r3)
    stfd    f21, 0x1a8(r3)
    stfd    f22, 0x1b0(r3)
    stfd    f23, 0x1b8(r3)
    stfd    f24, 0x1c0(r3)
    stfd    f25, 0x1c8(r3)
    stfd    f26, 0x1d0(r3)
    stfd    f27, 0x1d8(r3)
    stfd    f28, 0x1e0(r3)
    stfd    f29, 0x1e8(r3)
    stfd    f30, 0x1f0(r3)
    stfd    f31, 0x1f8(r3)
    mfspr   r5, HID2
    isync
    rlwinm. r5, r5, 3, 31, 31
    beq     _skps
    mfspr   r5, GQR0
    stw     r5, 0x208(r3)
    mfspr   r5, GQR1
    stw     r5, 0x210(r3)
    mfspr   r5, GQR2
    stw     r5, 0x218(r3)
    mfspr   r5, GQR3
    stw     r5, 0x220(r3)
    mfspr   r5, GQR4
    stw     r5, 0x228(r3)
    mfspr   r5, GQR5
    stw     r5, 0x230(r3)
    mfspr   r5, GQR6
    stw     r5, 0x238(r3)
    mfspr   r5, GQR7
    stw     r5, 0x240(r3)
    li      r5, 0x0
    mtspr   GQR0, r5
    isync
    psq_st  f0, 0x248(r3), 0, 0
    psq_st  f1, 0x250(r3), 0, 0
    psq_st  f2, 0x258(r3), 0, 0
    psq_st  f3, 0x260(r3), 0, 0
    psq_st  f4, 0x268(r3), 0, 0
    psq_st  f5, 0x270(r3), 0, 0
    psq_st  f6, 0x278(r3), 0, 0
    psq_st  f7, 0x280(r3), 0, 0
    psq_st  f8, 0x288(r3), 0, 0
    psq_st  f9, 0x290(r3), 0, 0
    psq_st  f10, 0x298(r3), 0, 0
    psq_st  f11, 0x2a0(r3), 0, 0
    psq_st  f12, 0x2a8(r3), 0, 0
    psq_st  f13, 0x2b0(r3), 0, 0
    psq_st  f14, 0x2b8(r3), 0, 0
    psq_st  f15, 0x2c0(r3), 0, 0
    psq_st  f16, 0x2c8(r3), 0, 0
    psq_st  f17, 0x2d0(r3), 0, 0
    psq_st  f18, 0x2d8(r3), 0, 0
    psq_st  f19, 0x2e0(r3), 0, 0
    psq_st  f20, 0x2e8(r3), 0, 0
    psq_st  f21, 0x2f0(r3), 0, 0
    psq_st  f22, 0x2f8(r3), 0, 0
    psq_st  f23, 0x300(r3), 0, 0
    psq_st  f24, 0x308(r3), 0, 0
    psq_st  f25, 0x310(r3), 0, 0
    psq_st  f26, 0x318(r3), 0, 0
    psq_st  f27, 0x320(r3), 0, 0
    psq_st  f28, 0x328(r3), 0, 0
    psq_st  f29, 0x330(r3), 0, 0
    psq_st  f30, 0x338(r3), 0, 0
    psq_st  f31, 0x340(r3), 0, 0
.sym _skps, local
    mffs    f0
    stfd    f0, 0x200(r3)
    blr

.sym stashspr, local
    mfxer   r5
    stw     r5, 0x348(r3)
    mfctr   r5
    stw     r5, 0x350(r3)
    mfspr   r5, HID0
    stw     r5, 0x380(r3)
    mfspr   r5, HID1
    stw     r5, 0x388(r3)
    mfspr   r5, IBAT0U
    stw     r5, 0x398(r3)
    mfspr   r5, IBAT0L
    stw     r5, 0x3a0(r3)
    mfspr   r5, IBAT1U
    stw     r5, 0x3a8(r3)
    mfspr   r5, IBAT1L
    stw     r5, 0x3b0(r3)
    mfspr   r5, IBAT2U
    stw     r5, 0x3b8(r3)
    mfspr   r5, IBAT2L
    stw     r5, 0x3c0(r3)
    mfspr   r5, IBAT3U
    stw     r5, 0x3c8(r3)
    mfspr   r5, IBAT3L
    stw     r5, 0x3d0(r3)
    mfspr   r5, DBAT0U
    stw     r5, 0x3d8(r3)
    mfspr   r5, DBAT0L
    stw     r5, 0x3e0(r3)
    mfspr   r5, DBAT1U
    stw     r5, 0x3e8(r3)
    mfspr   r5, DBAT1L
    stw     r5, 0x3f0(r3)
    mfspr   r5, DBAT2U
    stw     r5, 0x3f8(r3)
    mfspr   r5, DBAT2L
    stw     r5, 0x400(r3)
    mfspr   r5, DBAT3U
    stw     r5, 0x408(r3)
    mfspr   r5, DBAT3L
    stw     r5, 0x410(r3)
    mfsr    r5, 0
    stw     r5, 0x500(r3)
    mfsr    r5, 1
    stw     r5, 0x508(r3)
    mfsr    r5, 2
    stw     r5, 0x510(r3)
    mfsr    r5, 3
    stw     r5, 0x518(r3)
    mfsr    r5, 4
    stw     r5, 0x520(r3)
    mfsr    r5, 5
    stw     r5, 0x528(r3)
    mfsr    r5, 6
    stw     r5, 0x530(r3)
    mfsr    r5, 7
    stw     r5, 0x538(r3)
    mfsr    r5, 8
    stw     r5, 0x540(r3)
    mfsr    r5, 9
    stw     r5, 0x548(r3)
    mfsr    r5, 10
    stw     r5, 0x550(r3)
    mfsr    r5, 11
    stw     r5, 0x558(r3)
    mfsr    r5, 12
    stw     r5, 0x560(r3)
    mfsr    r5, 13
    stw     r5, 0x568(r3)
    mfsr    r5, 14
    stw     r5, 0x570(r3)
    mfsr    r5, 15
    stw     r5, 0x578(r3)
    mfspr   r5, SPRG0
    stw     r5, 0x418(r3)
    mfspr   r5, SPRG1
    stw     r5, 0x420(r3)
    mfspr   r5, SPRG2
    stw     r5, 0x428(r3)
    mfspr   r5, SPRG3
    stw     r5, 0x430(r3)
    mfspr   r5, DAR
    stw     r5, 0x438(r3)
    mfspr   r5, DSISR
    stw     r5, 0x440(r3)
    mfspr   r5, EAR
    stw     r5, 0x448(r3)
    mfspr   r5, DABR
    stw     r5, 0x450(r3)
    mfspr   r5, 284
    stw     r5, 0x458(r3)
    mfspr   r5, 285
    stw     r5, 0x460(r3)
    mfspr   r5, L2CR
    stw     r5, 0x468(r3)
    mfspr   r5, DEC
    stw     r5, 0x470(r3)
    mfspr   r5, IABR
    stw     r5, 0x478(r3)
    mfspr   r5, PMC1
    stw     r5, 0x480(r3)
    mfspr   r5, PMC2
    stw     r5, 0x488(r3)
    mfspr   r5, PMC3
    stw     r5, 0x490(r3)
    mfspr   r5, PMC4
    stw     r5, 0x498(r3)
    mfspr   r5, SIA
    stw     r5, 0x4a0(r3)
    mfspr   r5, MMCR0
    stw     r5, 0x4a8(r3)
    mfspr   r5, MMCR1
    stw     r5, 0x4b0(r3)
    mfspr   r5, THRM1
    stw     r5, 0x4b8(r3)
    mfspr   r5, THRM2
    stw     r5, 0x4c0(r3)
    mfspr   r5, THRM3
    stw     r5, 0x4c8(r3)
    mfspr   r5, ICTC
    stw     r5, 0x4d0(r3)
    mfspr   r5, SDR1
    stw     r5, 0x4d8(r3)
    mfspr   r5, PVR
    stw     r5, 0x4e0(r3)
    mfspr   r5, DMA_L
    stw     r5, 0x4f0(r3)
    mfspr   r5, DMA_U
    stw     r5, 0x4e8(r3)
    mfspr   r5, WPAR
    stw     r5, 0x4f8(r3)
    mfspr   r5, HID2
    stw     r5, 0x390(r3)
    blr

.sym ExHook, local
    stw     r3, 0x7c0(0)
    mfspr   r3, SRR0
    stw     r3, 0x7c4(0)
    mfspr   r3, SRR1
    stw     r3, 0x7c8(0)
    mfmsr   r3
    ori     r3, r3, 0x30
    mtspr   SRR1, r3
    lis     r3, 0x800d
    ori     r3, r3, 0x5f14
    mtspr   SRR0, r3
    mflr    r3
    stw     r3, 0x7cc(0)
    bl      nextop
.sym nextop, local
    mflr    r3
    stw     r3, 0x7d0(0)
    rfi
    .4byte  0x00000000
    .4byte  0x00000000
    .4byte  0x00000000
    .4byte  0x00000000

.sym ExHook2, local
    lis     r3, 0x8021
    ori     r3, r3, 0x7794
    stw     r0, 0x0(r3)
    stw     r1, 0x8(r3)
    stw     r2, 0x10(r3)
    stw     r4, 0x20(r3)
    stw     r5, 0x28(r3)
    stw     r6, 0x30(r3)
    stw     r7, 0x38(r3)
    stw     r8, 0x40(r3)
    stw     r9, 0x48(r3)
    stw     r10, 0x50(r3)
    stw     r11, 0x58(r3)
    stw     r12, 0x60(r3)
    stw     r13, 0x68(r3)
    stw     r14, 0x70(r3)
    stw     r15, 0x78(r3)
    stw     r16, 0x80(r3)
    stw     r17, 0x88(r3)
    stw     r18, 0x90(r3)
    stw     r19, 0x98(r3)
    stw     r20, 0xa0(r3)
    stw     r21, 0xa8(r3)
    stw     r22, 0xb0(r3)
    stw     r23, 0xb8(r3)
    stw     r24, 0xc0(r3)
    stw     r25, 0xc8(r3)
    stw     r26, 0xd0(r3)
    stw     r27, 0xd8(r3)
    stw     r28, 0xe0(r3)
    stw     r29, 0xe8(r3)
    stw     r30, 0xf0(r3)
    stw     r31, 0xf8(r3)
    lis     r1, 0x8021
    ori     r1, r1, 0x7774
    mr      r31, r3
    mfcr    r5
    stw     r5, 0x360(r3)
    mfmsr   r5
    stw     r5, 0x368(r3)
    bl      stashspr
    bl      stashfp
    mfmsr   r9
    xori    r0, r9, 0x10
    mtmsr   r0
    sync
    lwz     r4, 0x7c0(0)
    lwz     r5, 0x7cc(0)
    lwz     r6, 0x7c4(0)
    lwz     r7, 0x7c8(0)
    lwz     r8, 0x7d0(0)
    srwi    r8, r8, 8
    mtmsr   r9
    sync

.sym excont, local
    stw     r4, 0x18(r3)
    stw     r5, 0x358(r3)
    stw     r6, 0x370(r3)
    ori     r7, r7, 0x600
    xori    r7, r7, 0x600
    stw     r7, 0x378(r3)
    stw     r8, 0x580(r3)
    lis     r4, 0xcc00
    lhz     r5, 0x4010(r4)
    stw     r5, 0x6c8(r3)
    li      r5, 0xff
    sth     r5, 0x4010(r4)
    li      r4, 0x0
    mtspr   DABR, r4
    cmpwi   r8, 0xd
    bne     ntr
    lwz     r4, 0x6c0(r3)
    cmpwi   r4, 0x0
    beq     exokay
    lwz     r4, 0x6c4(r3)
    add     r5, r3, r4
    lwz     r7, 0x0(r5)
    ori     r7, r7, 0x600
    xori    r7, r7, 0x600
    stw     r7, 0x0(r5)
    li      r4, 0x0
    stw     r4, 0x6c0(r3)
    b       exokay

.sym ntr, local
    cmpwi   r8, 0x7
    bne     nbp
    lwz     r4, 0x370(r3)
    lwz     r5, 0x0(r4)
    cmpwi   r5, 0x1
    bne     checkfs
    li      r5, 0x1e
    stw     r5, 0x580(r3)
    addi    r4, r4, 0x4
    stw     r4, 0x370(r3)
    lis     r3, 0x8016
    ori     r3, r3, 0x57e1
    .4byte  0x48009231          /* bl external (snputs) */
    mr      r3, r31
    b       nhbp

.sym checkfs, local
    cmpwi   r5, 0x10
    blt     nhbp
    cmpwi   r5, 0x16
    bgt     nhbp
    andi.   r3, r5, 0xf
    .4byte  0x4BFFED34          /* b external (fileserver dispatch) */

.sym nbp, local
    cmpwi   r8, 0x3
    bne     nhbp
    lwz     r5, 0x440(r3)
    rlwinm. r5, r5, 10, 31, 31
    beq     nhbp
    li      r5, 0x1f
    stw     r5, 0x580(r3)

.sym nhbp, local
    li      r4, 0x0
    stw     r4, 0x6bc(r3)

.sym exokay, local
    lis     r5, 0x8016
    ori     r5, r5, 0x56f0
    li      r4, 0x1
    stw     r4, 0x0(r5)
    lwz     r4, 0x6bc(r3)
    cmpwi   r4, 0x0
    bne     comms

.sym notify, local
    lis     r3, 0x8016
    ori     r3, r3, 0x5808
    li      r4, 0x8
    lwz     r0, 0x6a4(r31)
    mtlr    r0
    blrl
    b       comms

.sym hookvecs, local
    mflr    r0
    stw     r0, 0x4(r1)
    stwu    r1, -0x8(r1)
    lis     r5, 0x8000
    stw     r4, 0x44(r5)
    li      r0, 0x1
    stw     r0, 0x40(r5)
    lis     r5, 0x8000
    ori     r5, r5, 0x100
    li      r6, 0x1
.sym lpv, local
    and.    r4, r3, r6
    beq     nvec
    lis     r7, 0x800d
    ori     r7, r7, 0x5ebc
    subi    r8, r5, 0x4
    addi    r9, r7, 0x54
.sym lpwr, local
    lwzu    r4, 0x4(r7)
    stwu    r4, 0x4(r8)
    cmplw   r7, r9
    blt     lpwr
.sym nvec, local
    addi    r5, r5, 0x100
    slwi.   r6, r6, 1
    bne     lpv
    lis     r3, 0x8000
    ori     r3, r3, 0x100
    li      r4, 0x2000
    .4byte  0x48008585          /* bl external (ICInvalidateRange) */
    lis     r3, 0x8000
    ori     r3, r3, 0x100
    li      r4, 0x2000
    .4byte  0x4800860D          /* bl external (DCFlushRange) */
    lis     r3, 0x0
    ori     r3, r3, 0x0
    li      r4, 0x2000
    .4byte  0x480085FD          /* bl external (DCFlushRange) */
    addi    r1, r1, 0x8
    lwz     r0, 0x4(r1)
    mtlr    r0
    blr

.sym ISIentry, global
    mtspr   SPRG1, r3
    li      r3, 0x4
    mtspr   SPRG2, r3
    lis     r3, 0x8021
    ori     r3, r3, 0x7794
    b       gotexnum

.sym DSIentry, global
    mtspr   SPRG1, r3
    li      r3, 0x3
    mtspr   SPRG2, r3
    lis     r3, 0x8021
    ori     r3, r3, 0x7794

.sym gotexnum, local
    stw     r0, 0x0(r3)
    stw     r1, 0x8(r3)
    stw     r2, 0x10(r3)
    stw     r4, 0x20(r3)
    stw     r5, 0x28(r3)
    stw     r6, 0x30(r3)
    stw     r7, 0x38(r3)
    stw     r8, 0x40(r3)
    stw     r9, 0x48(r3)
    stw     r10, 0x50(r3)
    stw     r11, 0x58(r3)
    stw     r12, 0x60(r3)
    stw     r13, 0x68(r3)
    stw     r14, 0x70(r3)
    stw     r15, 0x78(r3)
    stw     r16, 0x80(r3)
    stw     r17, 0x88(r3)
    stw     r18, 0x90(r3)
    stw     r19, 0x98(r3)
    stw     r20, 0xa0(r3)
    stw     r21, 0xa8(r3)
    stw     r22, 0xb0(r3)
    stw     r23, 0xb8(r3)
    stw     r24, 0xc0(r3)
    stw     r25, 0xc8(r3)
    stw     r26, 0xd0(r3)
    stw     r27, 0xd8(r3)
    stw     r28, 0xe0(r3)
    stw     r29, 0xe8(r3)
    stw     r30, 0xf0(r3)
    stw     r31, 0xf8(r3)
    lis     r1, 0x8021
    ori     r1, r1, 0x7774
    mr      r31, r3
    mflr    r30
    mfcr    r5
    stw     r5, 0x360(r3)
    mfmsr   r5
    stw     r5, 0x368(r3)
    bl      stashspr
    bl      stashfp
    mfspr   r4, SPRG1
    mr      r5, r30
    mfspr   r6, SRR0
    mfspr   r7, SRR1
    mfspr   r8, SPRG2
    b       excont
.endfn pad_01_800D5020_text

# .data
.data
.balign 8

.obj indebugger, local
    .4byte 0x00000001
.endobj indebugger

.obj context, local
    .4byte 0x00000000
.endobj context

.obj exception_str, local
    .4byte 0x0A457863
    .4byte 0x65707469
    .4byte 0x6F6E0A00
    .4byte 0x00000000
.endobj exception_str

.obj cbmsg, local
    .4byte 0x0A2A4342
    .4byte 0x2A0A0000
.endobj cbmsg

.obj cbmsg2, local
    .4byte 0x0A2A5144
    .4byte 0x203D2025
    .4byte 0x782A0A00
    .4byte 0x00000000
.endobj cbmsg2

.obj intmodemsg, local
    .4byte 0x0A496E74
    .4byte 0x206D6F64
    .4byte 0x6520656E
    .4byte 0x61626C65
    .4byte 0x640A0000
.endobj intmodemsg

.obj PPCHaltPatch, local
    .4byte 0x556E6162
    .4byte 0x6C652074
    .4byte 0x6F207075
    .4byte 0x7420736E
    .4byte 0x50617573
    .4byte 0x6520696E
    .4byte 0x20505043
    .4byte 0x48616C74
    .4byte 0x0A000000
.endobj PPCHaltPatch

.obj ddhjmp, local
    .4byte EXI2_Init
    .4byte EXI2_EnableInterrupts
    .4byte EXI2_Poll
    .4byte EXI2_ReadN
    .4byte EXI2_WriteN
    .4byte EXI2_Reserve
    .4byte EXI2_Unreserve
.endobj ddhjmp

.obj gdevjmp, local
    .4byte DBInitComm
    .4byte DBInitInterrupts
    .4byte DBQueryData
    .4byte DBRead
    .4byte DBWrite
    .4byte DBOpen
    .4byte DBClose
.endobj gdevjmp

.obj PPCHaltData, local
    .4byte 0x7C0004AC
    .4byte 0x60000000
    .4byte 0x38600000
    .4byte 0x60000000
    .4byte 0x4BFFFFF4
    .4byte 0xFFFFFFFF
.endobj PPCHaltData

.obj cmderrstr, local
    .4byte 0x2A2A2A20
    .4byte 0x434D4420
    .4byte 0x52454144
    .4byte 0x20455252
    .4byte 0x4F52202A
    .4byte 0x2A2A0A00
    .byte 0x00
.endobj cmderrstr

.obj cmderrstr2, local
    .4byte 0x2A2A2A20
    .4byte 0x42414420
    .4byte 0x434F4D4D
    .4byte 0x414E4420
    .4byte 0x2A2A2A0A
    .2byte 0x0000
.endobj cmderrstr2

.obj waitstr, local
    .4byte 0x57414954
    .4byte 0x2E2E2E0A
    .2byte 0x0000
.endobj waitstr

.obj pausetext, local
    .4byte 0x736E5061
    .4byte 0x75736528
    .4byte 0x29203A20
    .4byte 0x53746F70
    .4byte 0x7065642E
    .byte 0x0A, 0x00, 0x00
.endobj pausetext

.obj commserror, local
    .4byte 0x436F6D6D
    .4byte 0x73204572
    .4byte 0x726F720A
    .4byte 0x00000000
.endobj commserror

.obj exception_notification_packet, local
    .4byte 0x07000000
    .4byte 0x00000000
.endobj exception_notification_packet

# .bss
.section .bss, "wa", @nobits
.balign 4

.obj SNstack, local
    .skip 0x20
.endobj SNstack

.obj SNworkspace, local
    .skip 0x11B7
.endobj SNworkspace

.obj lbl_8021894B, global
    .skip 0x51D
.endobj lbl_8021894B
