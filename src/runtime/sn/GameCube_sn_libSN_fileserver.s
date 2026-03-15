.include "macros.inc"
.file "GameCube_sn_libSN_fileserver.s"

# .text
.text
.balign 4

.sym cmdFS_STOP, local

.fn pad_01_800D4F30_text, local
    li      r4, 0x1
    lis     r3, 0x8016
    ori     r3, r3, 0x5678
    stw     r4, 0x0(r3)
    .4byte  0x4BFFFF04          /* b fscomms (external) */

.sym snInitFileserver, global
    li      r3, 0x10
    lis     r4, 0x800D
    ori     r4, r4, 0x4FE4
    stw     r3, 0x0(r4)
    addi    r3, r3, 0x1
    lis     r4, 0x800D
    ori     r4, r4, 0x4FEC
    stw     r3, 0x0(r4)
    addi    r3, r3, 0x1
    lis     r4, 0x800D
    ori     r4, r4, 0x4FF4
    stw     r3, 0x0(r4)
    addi    r3, r3, 0x1
    lis     r4, 0x800D
    ori     r4, r4, 0x4FFC
    stw     r3, 0x0(r4)
    addi    r3, r3, 0x1
    lis     r4, 0x800D
    ori     r4, r4, 0x5004
    stw     r3, 0x0(r4)
    addi    r3, r3, 0x1
    lis     r4, 0x800D
    ori     r4, r4, 0x500C
    stw     r3, 0x0(r4)
    addi    r3, r3, 0x1
    lis     r4, 0x800D
    ori     r4, r4, 0x5014
    stw     r3, 0x0(r4)
    blr

.sym snFSNotAvailable, local
    mflr    r0
    stw     r0, 0x4(r1)
    stwu    r1, -0x8(r1)
    lis     r3, 0x8016
    ori     r3, r3, 0x56B1
    .4byte  0x4800A2F1          /* bl OSReport (external) */
    li      r3, -0x1
    addi    r1, r1, 0x8
    lwz     r0, 0x4(r1)
    mtlr    r0
    blr

.sym PCinit, global
    b       snFSNotAvailable
    blr

.sym PCcreat, global
    b       snFSNotAvailable
    blr

.sym PCopen, global
    b       snFSNotAvailable
    blr

.sym PCclose, global
    b       snFSNotAvailable
    blr

.sym PCread, global
    b       snFSNotAvailable
    blr

.sym PCwrite, global
    b       snFSNotAvailable
    blr

.sym PClseek, global
    b       snFSNotAvailable
    blr
    .4byte  0x00000000
.endfn pad_01_800D4F30_text

# .data
.data
.balign 8

.obj commandtable, local
    .4byte 0x00000001
    .4byte 0x00000002
    .4byte 0x00000003
    .4byte 0x00000004
    .4byte 0x00000005
    .4byte 0x00000006
    .4byte 0x00000007
.endobj commandtable

.obj fscmdtab, local
    .4byte cmdNop
    .4byte cmdNop
    .4byte cmdNop
    .4byte cmdRecvMem
    .4byte cmdSendMem
    .4byte cmdNop
    .4byte cmdFS_STOP
    .4byte cmdNop
    .4byte cmdNop
    .4byte cmdNop
    .4byte cmdFS_ACK_800D4EDC
    .4byte cmdNop
    .4byte cmdNop
.endobj fscmdtab

.obj fscommand, local
    .4byte 0x09000000
    .4byte 0x00000000
    .4byte 0x00000000
    .4byte 0x00000000
    .4byte 0x00000000
    .4byte 0x00000000
.endobj fscommand

.obj fsstop, local
    .4byte 0x00000000
.endobj fsstop

.obj fscmderrstr, local
    .4byte 0x2A2A2A20
    .4byte 0x46532043
    .4byte 0x4D442052
    .4byte 0x45414420
    .4byte 0x4552524F
    .4byte 0x52202A2A
    .4byte 0x2A0A0000
.endobj fscmderrstr

.obj fscmderrstr2, local
    .4byte 0x2A2A2A20
    .4byte 0x46532042
    .4byte 0x41442043
    .4byte 0x4F4D4D41
    .4byte 0x4E44202A
    .4byte 0x2A2A0A00
    .byte 0x00
.endobj fscmderrstr2

.obj FSNotAvailable, local
    .4byte 0x46696C65
    .4byte 0x20736572
    .4byte 0x76657220
    .4byte 0x66756E63
    .4byte 0x74696F6E
    .4byte 0x206E6F74
    .4byte 0x20617661
    .4byte 0x696C6162
    .4byte 0x6C652069
    .4byte 0x6E206E6F
    .4byte 0x6E206465
    .4byte 0x62756720
    .4byte 0x6D6F6465
    .4byte 0x0A000000
    .4byte 0x00000000
    .byte 0x00, 0x00, 0x00
.endobj FSNotAvailable
