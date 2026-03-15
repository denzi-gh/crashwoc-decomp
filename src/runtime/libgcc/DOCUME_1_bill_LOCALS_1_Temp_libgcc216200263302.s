.include "macros.inc"

.text
.balign 4

.sym gcc2_compiled., local
.endsym gcc2_compiled.

.fn __ashldi3, global
    mr.     r5, r5
    beqlr
    subfic  r0, r5, 0x20
    cmpwi   r0, 0x0
    bgt     .L_ashldi3_gt
    neg     r0, r0
    li      r12, 0x0
    slw     r11, r4, r0
    b       .L_ashldi3_end
.L_ashldi3_gt:
    slw     r9, r3, r5
    srw     r0, r4, r0
    slw     r12, r4, r5
    or      r11, r9, r0
.L_ashldi3_end:
    mr      r3, r11
    mr      r4, r12
    blr
.endfn __ashldi3
