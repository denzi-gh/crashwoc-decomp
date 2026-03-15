.include "macros.inc"

.text
.balign 4

.sym gcc2_compiled., local
.endsym gcc2_compiled.

.fn sqrtf, global
    stwu    r1, -0x10(r1)
    fmr     f0, f1
    stfs    f0, 0x8(r1)
    lwz     r10, 0x8(r1)
    mr      r11, r10
    rlwinm  r9, r11, 0, 1, 8
    lis     r0, 0x7f80
    cmpw    r9, r0
    bne     .L_800D7DB4
    fmadds  f1, f1, f1, f1
    b       .L_800D7E8C
.L_800D7DB4:
    cmpwi   cr7, r11, 0x0
    bgt     cr7, .L_800D7DD4
    clrlwi. r0, r11, 1
    beq     .L_800D7E8C
    bge     cr7, .L_800D7DD4
    fsubs   f1, f1, f1
    fdivs   f1, f1, f1
    b       .L_800D7E8C
.L_800D7DD4:
    srawi.  r8, r10, 23
    bne     .L_800D7DFC
    andis.  r9, r11, 0x80
    li      r9, 0x0
    bne     .L_800D7DF8
.L_800D7DE8:
    slwi    r11, r11, 1
    addi    r9, r9, 0x1
    andis.  r0, r11, 0x80
    beq     .L_800D7DE8
.L_800D7DF8:
    subfic  r8, r9, 0x1
.L_800D7DFC:
    subi    r8, r8, 0x7f
    clrlwi  r0, r11, 9
    andi.   r9, r8, 0x1
    oris    r11, r0, 0x80
    add     r9, r11, r11
    srawi   r8, r8, 1
    li      r6, 0x0
    li      r7, 0x0
    mfcr    r0
    extrwi  r0, r0, 1, 2
    lis     r10, 0x100
    neg     r0, r0
    andc    r9, r9, r0
    and     r0, r11, r0
    or      r11, r0, r9
    add     r11, r11, r11
.L_800D7E3C:
    add     r0, r6, r10
    cmpw    r0, r11
    bgt     .L_800D7E54
    subf    r11, r0, r11
    add     r6, r0, r10
    add     r7, r7, r10
.L_800D7E54:
    srwi.   r10, r10, 1
    add     r11, r11, r11
    bne     .L_800D7E3C
    cmpwi   r11, 0x0
    beq     .L_800D7E70
    clrlwi  r0, r7, 31
    add     r7, r7, r0
.L_800D7E70:
    srawi   r11, r7, 1
    slwi    r9, r8, 23
    addis   r11, r11, 0x3f00
    add     r0, r11, r9
    stw     r0, 0x8(r1)
    lfs     f0, 0x8(r1)
    fmr     f1, f0
.L_800D7E8C:
    addi    r1, r1, 0x10
    blr
.endfn sqrtf

.section .data5
.balign 8

.obj one, local
    .4byte 0x3F800000
.endobj one

.obj tiny, local
    .4byte 0x0DA24260
.endobj tiny
