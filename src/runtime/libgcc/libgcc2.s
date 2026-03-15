.include "macros.inc"

.text
.balign 4

.sym gcc2_compiled., local
.endsym gcc2_compiled.

.fn _exit, global
    .4byte 0x00000000
    blr
.endfn _exit
