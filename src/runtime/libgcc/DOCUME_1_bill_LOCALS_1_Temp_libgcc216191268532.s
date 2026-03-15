.include "macros.inc"

.text
.balign 4

.sym gcc2_compiled., local
.endsym gcc2_compiled.

.fn __lshrdi3, global
    mr.     r5, r5
    beqlr
    subfic  r0, r5, 0x20
    cmpwi   r0, 0x0
    bgt     .L_lshrdi3_gt
    neg     r0, r0
    li      r11, 0x0
    srw     r12, r3, r0
    b       .L_lshrdi3_end
.L_lshrdi3_gt:
    srw     r9, r4, r5
    slw     r0, r3, r0
    srw     r11, r3, r5
    or      r12, r9, r0
.L_lshrdi3_end:
    mr      r3, r11
    mr      r4, r12
    blr
.endfn __lshrdi3
