.include "macros.inc"

.text
.balign 4

.sym gcc2_compiled., local
.endsym gcc2_compiled.

.fn fabs, global
    stwu    r1, -0x10(r1)
    fmr     f0, f1
    stfd    f0, 0x8(r1)
    lwz     r11, 0x8(r1)
    lwz     r12, 0xc(r1)
    stfd    f1, 0x8(r1)
    lwz     r9, 0x8(r1)
    lwz     r10, 0xc(r1)
    clrlwi  r9, r11, 1
    stw     r9, 0x8(r1)
    stw     r10, 0xc(r1)
    lfd     f0, 0x8(r1)
    fmr     f1, f0
    addi    r1, r1, 0x10
    blr
.endfn fabs
