.include "macros.inc"

.text
.balign 4

.sym gcc2_compiled., local
.endsym gcc2_compiled.

.fn __ashrdi3, global
    mr.     r5, r5
    beqlr
    subfic  r0, r5, 0x20
    cmpwi   r0, 0x0
    bgt     .L_ashrdi3_gt
    neg     r0, r0
    srawi   r11, r3, 31
    sraw    r12, r3, r0
    b       .L_ashrdi3_end
.L_ashrdi3_gt:
    srw     r9, r4, r5
    slw     r0, r3, r0
    sraw    r11, r3, r5
    or      r12, r9, r0
.L_ashrdi3_end:
    mr      r3, r11
    mr      r4, r12
    blr
.endfn __ashrdi3
