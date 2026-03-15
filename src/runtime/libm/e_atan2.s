.include "macros.inc"

.text
.balign 4

.sym gcc2_compiled., local
.endsym gcc2_compiled.

.fn atan2, global
    stwu    r1, -0x18(r1)
    mflr    r0
    stw     r31, 0x14(r1)
    stw     r0, 0x1c(r1)
    fmr     f0, f2
    stfd    f0, 0x8(r1)
    lwz     r9, 0x8(r1)
    lwz     r10, 0xc(r1)
    mr      r8, r10
    mr      r5, r9
    clrlwi  r6, r5, 1
    fmr     f0, f1
    stfd    f0, 0x8(r1)
    lwz     r9, 0x8(r1)
    lwz     r10, 0xc(r1)
    mr      r11, r10
    mr      r10, r9
    clrlwi  r7, r10, 1
    neg     r0, r8
    lis     r9, 0x7ff0
    or      r0, r8, r0
    srwi    r0, r0, 31
    or      r0, r6, r0
    cmplw   r0, r9
    bgt     .L_800D6750
    neg     r0, r11
    or      r0, r11, r0
    srwi    r0, r0, 31
    or      r0, r7, r0
    cmplw   r0, r9
    ble     .L_800D6758
.L_800D6750:
    fadd    f1, f2, f1
    b       .L_800D6940
.L_800D6758:
    subis   r0, r5, 0x3ff0
    or.     r9, r0, r8
    bne     .L_800D676C
    bl      atan
    b       .L_800D6940
.L_800D676C:
    or.     r0, r7, r11
    srwi    r9, r10, 31
    rlwinm  r0, r5, 2, 30, 30
    or      r31, r9, r0
    bne     .L_800D67A0
    cmpwi   r31, 0x2
    beq     .L_800D685C
    ble     .L_800D6798
    cmpwi   r31, 0x3
    beq     .L_800D6864
    b       .L_800D67A0
.L_800D6798:
    cmpwi   r31, 0x0
    bge     .L_800D6940
.L_800D67A0:
    or.     r9, r6, r8
    bne     .L_800D67C0
.L_800D67A8:
    cmpwi   r10, 0x0
    lfd     f1, lbl_803D3580@sda21(r0)
    lfd     f0, lbl_803D3578@sda21(r0)
    bge     .L_800D6940
    fmr     f1, f0
    b       .L_800D6940
.L_800D67C0:
    lis     r0, 0x7ff0
    cmpw    r6, r0
    bne     .L_800D686C
    cmpw    r7, r6
    bne     .L_800D6820
    cmpwi   r31, 0x1
    beq     .L_800D6808
    bgt     .L_800D67EC
    cmpwi   r31, 0x0
    beq     .L_800D6800
    b       .L_800D686C
.L_800D67EC:
    cmpwi   r31, 0x2
    beq     .L_800D6810
    cmpwi   r31, 0x3
    beq     .L_800D6818
    b       .L_800D686C
.L_800D6800:
    lfd     f1, lbl_803D3588@sda21(r0)
    b       .L_800D6940
.L_800D6808:
    lfd     f1, lbl_803D3590@sda21(r0)
    b       .L_800D6940
.L_800D6810:
    lfd     f1, lbl_803D3598@sda21(r0)
    b       .L_800D6940
.L_800D6818:
    lfd     f1, lbl_803D35A0@sda21(r0)
    b       .L_800D6940
.L_800D6820:
    cmpwi   r31, 0x1
    beq     .L_800D6854
    bgt     .L_800D6838
    cmpwi   r31, 0x0
    beq     .L_800D684C
    b       .L_800D686C
.L_800D6838:
    cmpwi   r31, 0x2
    beq     .L_800D685C
    cmpwi   r31, 0x3
    beq     .L_800D6864
    b       .L_800D686C
.L_800D684C:
    lfd     f1, lbl_803D35A8@sda21(r0)
    b       .L_800D6940
.L_800D6854:
    lfd     f1, lbl_803D35B0@sda21(r0)
    b       .L_800D6940
.L_800D685C:
    lfd     f1, lbl_803D3568@sda21(r0)
    b       .L_800D6940
.L_800D6864:
    lfd     f1, lbl_803D3570@sda21(r0)
    b       .L_800D6940
.L_800D686C:
    lis     r0, 0x7ff0
    cmpw    r7, r0
    beq     .L_800D67A8
    subf    r0, r6, r7
    srawi   r0, r0, 20
    cmpwi   r0, 0x3c
    ble     .L_800D6890
    lfd     f13, lbl_803D3580@sda21(r0)
    b       .L_800D68C0
.L_800D6890:
    cmpwi   cr7, r0, -0x3c
    srwi    r9, r5, 31
    mfcr    r0
    extrwi  r0, r0, 1, 28
    and.    r11, r9, r0
    beq     .L_800D68B0
    lfd     f13, lbl_803D35A8@sda21(r0)
    b       .L_800D68C0
.L_800D68B0:
    fdiv    f1, f1, f2
    bl      fabs
    bl      atan
    fmr     f13, f1
.L_800D68C0:
    cmpwi   r31, 0x1
    beq     .L_800D68E4
    bgt     .L_800D68D8
    cmpwi   r31, 0x0
    beq     .L_800D6914
    b       .L_800D6930
.L_800D68D8:
    cmpwi   r31, 0x2
    beq     .L_800D691C
    b       .L_800D6930
.L_800D68E4:
    fmr     f0, f13
    stfd    f0, 0x8(r1)
    lwz     r11, 0x8(r1)
    lwz     r12, 0xc(r1)
    stfd    f13, 0x8(r1)
    lwz     r9, 0x8(r1)
    lwz     r10, 0xc(r1)
    xoris   r9, r11, 0x8000
    stw     r9, 0x8(r1)
    stw     r10, 0xc(r1)
    lfd     f0, 0x8(r1)
    fmr     f13, f0
.L_800D6914:
    fmr     f1, f13
    b       .L_800D6940
.L_800D691C:
    lfd     f1, lbl_803D35B8@sda21(r0)
    lfd     f0, lbl_803D3568@sda21(r0)
    fsub    f1, f13, f1
    fsub    f1, f0, f1
    b       .L_800D6940
.L_800D6930:
    lfd     f1, lbl_803D35B8@sda21(r0)
    lfd     f0, lbl_803D3568@sda21(r0)
    fsub    f1, f13, f1
    fsub    f1, f1, f0
.L_800D6940:
    lwz     r0, 0x1c(r1)
    mtlr    r0
    lwz     r31, 0x14(r1)
    addi    r1, r1, 0x18
    blr
.endfn atan2

.section .data5, "wa"
.balign 8

.obj tiny, local
    .4byte 0x01A56E1F
    .4byte 0xC2F8F359
.endobj tiny

.obj zero, local
    .4byte 0x00000000
    .4byte 0x00000000
.endobj zero

.obj pi_o_4, local
    .4byte 0x3FE921FB
    .4byte 0x54442D18
.endobj pi_o_4

.obj pi_o_2, local
    .4byte 0x3FF921FB
    .4byte 0x54442D18
.endobj pi_o_2

.obj pi, local
    .4byte 0x400921FB
    .4byte 0x54442D18
.endobj pi

.obj pi_lo, local
    .4byte 0x3CA1A626
    .4byte 0x33145C07
.endobj pi_lo
