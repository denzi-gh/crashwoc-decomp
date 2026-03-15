.include "macros.inc"

.text
.balign 4

.sym gcc2_compiled., local
.endsym gcc2_compiled.

.fn cos, global
    stwu    r1, -0x20(r1)
    mflr    r0
    stw     r0, 0x24(r1)
    lfd     f2, lbl_803D36F8@sda21(r13)
    fmr     f0, f1
    stfd    f0, 0x18(r1)
    lwz     r9, 0x18(r1)
    lwz     r10, 0x1c(r1)
    clrlwi  r9, r9, 1
    lis     r0, 0x3fe9
    ori     r0, r0, 0x21fb
    cmpw    r9, r0
    bgt     .L_check_nan
    bl      __kernel_cos
    b       .L_return
.L_check_nan:
    lis     r0, 0x7fef
    ori     r0, r0, 0xffff
    cmpw    r9, r0
    ble     .L_rem_pio2
    fsub    f1, f1, f1
    b       .L_return
.L_rem_pio2:
    addi    r3, r1, 0x8
    bl      __ieee754_rem_pio2
    clrlwi  r3, r3, 30
    cmpwi   r3, 0x1
    beq     .L_case1
    bgt     .L_case_gt1
    cmpwi   r3, 0x0
    beq     .L_case0
    b       .L_case3
.L_case_gt1:
    cmpwi   r3, 0x2
    beq     .L_case2
    b       .L_case3
.L_case0:
    lfd     f1, 0x8(r1)
    lfd     f2, 0x10(r1)
    bl      __kernel_cos
    b       .L_return
.L_case1:
    lfd     f1, 0x8(r1)
    li      r3, 0x1
    lfd     f2, 0x10(r1)
    bl      __kernel_sin
    fneg    f1, f1
    b       .L_return
.L_case2:
    lfd     f1, 0x8(r1)
    lfd     f2, 0x10(r1)
    bl      __kernel_cos
    fneg    f1, f1
    b       .L_return
.L_case3:
    lfd     f1, 0x8(r1)
    li      r3, 0x1
    lfd     f2, 0x10(r1)
    bl      __kernel_sin
.L_return:
    lwz     r0, 0x24(r1)
    mtlr    r0
    addi    r1, r1, 0x20
    blr
.endfn cos
