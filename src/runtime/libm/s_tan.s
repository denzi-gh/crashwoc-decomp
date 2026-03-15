.include "macros.inc"

.text
.balign 4

.sym gcc2_compiled., local
.endsym gcc2_compiled.

.fn tan, global
    stwu    r1, -0x20(r1)
    mflr    r0
    stw     r0, 0x24(r1)
    lfd     f2, lbl_803D3718@sda21(r13)
    fmr     f0, f1
    stfd    f0, 0x18(r1)
    lwz     r9, 0x18(r1)
    lwz     r10, 0x1c(r1)
    clrlwi  r9, r9, 1
    lis     r0, 0x3fe9
    ori     r0, r0, 0x21fb
    cmpw    r9, r0
    bgt     .L_check_large
    li      r3, 0x1
    bl      __kernel_tan
    b       .L_return
.L_check_large:
    lis     r0, 0x7fef
    ori     r0, r0, 0xffff
    cmpw    r9, r0
    bgt     .L_nan
    addi    r3, r1, 0x8
    bl      __ieee754_rem_pio2
    clrlslwi r3, r3, 31, 1
    lfd     f1, 0x8(r1)
    lfd     f2, 0x10(r1)
    subfic  r3, r3, 0x1
    bl      __kernel_tan
    b       .L_return
.L_nan:
    fsub    f1, f1, f1
.L_return:
    lwz     r0, 0x24(r1)
    mtlr    r0
    addi    r1, r1, 0x20
    blr
.endfn tan
