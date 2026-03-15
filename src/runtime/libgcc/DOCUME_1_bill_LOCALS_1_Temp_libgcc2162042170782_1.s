.include "macros.inc"

.text
.balign 4

.sym gcc2_compiled., local
.endsym gcc2_compiled.

.fn __divdi3, global
    stwu    r1, -0x28(r1)
    stmw    r25, 0xc(r1)
    cmpwi   r3, 0x0
    li      r25, 0x0
    bge     .L_pos_u
    neg     r8, r4
    neg     r9, r3
    subic   r10, r8, 0x1
    subfe   r0, r10, r8
    li      r25, -0x1
    subf    r7, r0, r9
    mr      r3, r7
    mr      r4, r8
.L_pos_u:
    cmpwi   r5, 0x0
    bge     .L_pos_v
    neg     r12, r6
    neg     r9, r5
    subic   r10, r12, 0x1
    subfe   r0, r10, r12
    nor     r25, r25, r25
    subf    r11, r0, r9
    mr      r5, r11
    mr      r6, r12
.L_pos_v:
    mr.     r10, r5
    mr      r31, r3
    mr      r7, r6
    mr      r30, r4
    bne     .L_den_hi_nonzero
    cmplw   r7, r31
    ble     .L_den_ge_num_hi
    # d0 == 0 && d1 > n1 => single precision
    li      r11, 0x0
    mr      r9, r6
    ori     r11, r11, 0xffff
    cmplw   r7, r11
    bgt     .L_count_shift_sp1
    subfic  r0, r7, 0xff
    subfe   r0, r0, r0
    rlwinm  r5, r0, 0, 28, 28
    b       .L_shift_done_sp1
.L_count_shift_sp1:
    lis     r0, 0xff
    ori     r0, r0, 0xffff
    subfc   r0, r7, r0
    subfe   r0, r0, r0
    rlwinm  r0, r0, 0, 27, 28
    ori     r5, r0, 0x10
.L_shift_done_sp1:
    srw     r11, r9, r5
    li      r9, __clz_tab@sda21
    lbzx    r0, r9, r11
    add     r0, r0, r5
    subfic  r5, r0, 0x20
    cmpwi   r5, 0x0
    beq     .L_bm_zero_sp
    subfic  r0, r5, 0x20
    slw     r9, r31, r5
    srw     r0, r30, r0
    slw     r7, r7, r5
    or      r31, r9, r0
    slw     r30, r30, r5
.L_bm_zero_sp:
    srwi    r8, r7, 16
    clrlwi  r12, r7, 16
    divwu   r0, r31, r8
    srwi    r11, r30, 16
    mullw   r9, r0, r8
    mr      r5, r0
    mullw   r6, r5, r12
    subf    r10, r9, r31
    slwi    r0, r10, 16
    or      r10, r0, r11
    cmplw   r10, r6
    bge     .L_sp1_q1_ok
    add     r10, r10, r7
    subi    r5, r5, 0x1
    cmplw   r10, r7
    blt     .L_sp1_q1_ok
    cmplw   r10, r6
    bge     .L_sp1_q1_ok
    subi    r5, r5, 0x1
    add     r10, r10, r7
.L_sp1_q1_ok:
    subf    r10, r6, r10
    clrlwi  r11, r30, 16
    divwu   r0, r10, r8
    mullw   r9, r0, r8
    mr      r8, r0
    mullw   r6, r8, r12
    subf    r0, r9, r10
    slwi    r0, r0, 16
    or      r0, r0, r11
    cmplw   r0, r6
    bge     .L_sp1_q0_ok
    add     r0, r0, r7
    subi    r8, r8, 0x1
    cmplw   r0, r7
    blt     .L_sp1_q0_ok
    cmplw   r0, r6
    bge     .L_sp1_q0_ok
    subi    r8, r8, 0x1
.L_sp1_q0_ok:
    slwi    r0, r5, 16
    or      r6, r0, r8
    b       .L_done_q0_zero
.L_den_ge_num_hi:
    # d0 == 0 && d1 <= n1
    cmpwi   r7, 0x0
    bne     .L_den_nonzero
    li      r0, 0x1
    divwu   r7, r0, r10
.L_den_nonzero:
    li      r0, 0x0
    ori     r0, r0, 0xffff
    cmplw   r7, r0
    bgt     .L_count_shift_dp1
    subfic  r0, r7, 0xff
    subfe   r0, r0, r0
    rlwinm  r10, r0, 0, 28, 28
    b       .L_shift_done_dp1
.L_count_shift_dp1:
    lis     r0, 0xff
    ori     r0, r0, 0xffff
    subfc   r0, r7, r0
    subfe   r0, r0, r0
    rlwinm  r0, r0, 0, 27, 28
    ori     r10, r0, 0x10
.L_shift_done_dp1:
    srw     r11, r7, r10
    li      r9, __clz_tab@sda21
    lbzx    r0, r9, r11
    add     r0, r0, r10
    subfic  r5, r0, 0x20
    cmpwi   r5, 0x0
    bne     .L_bm_nonzero_dp
    subf    r31, r7, r31
    li      r4, 0x1
    b       .L_dp_second_half
.L_bm_nonzero_dp:
    subfic  r8, r5, 0x20
    slw     r9, r31, r5
    srw     r0, r30, r8
    srw     r12, r31, r8
    or      r31, r9, r0
    slw     r30, r30, r5
    slw     r7, r7, r5
    srwi    r8, r7, 16
    divwu   r0, r12, r8
    clrlwi  r4, r7, 16
    srwi    r11, r31, 16
    mullw   r9, r0, r8
    mr      r5, r0
    mullw   r6, r5, r4
    subf    r10, r9, r12
    slwi    r0, r10, 16
    or      r10, r0, r11
    cmplw   r10, r6
    bge     .L_dp1_q1_ok
    add     r10, r10, r7
    subi    r5, r5, 0x1
    cmplw   r10, r7
    blt     .L_dp1_q1_ok
    cmplw   r10, r6
    bge     .L_dp1_q1_ok
    subi    r5, r5, 0x1
    add     r10, r10, r7
.L_dp1_q1_ok:
    subf    r10, r6, r10
    clrlwi  r11, r31, 16
    divwu   r0, r10, r8
    mullw   r9, r0, r8
    mr      r8, r0
    mullw   r6, r8, r4
    subf    r9, r9, r10
    slwi    r0, r9, 16
    or      r9, r0, r11
    cmplw   r9, r6
    bge     .L_dp1_q0_ok
    add     r9, r9, r7
    subi    r8, r8, 0x1
    cmplw   r9, r7
    blt     .L_dp1_q0_ok
    cmplw   r9, r6
    bge     .L_dp1_q0_ok
    subi    r8, r8, 0x1
    add     r9, r9, r7
.L_dp1_q0_ok:
    slwi    r0, r5, 16
    subf    r31, r6, r9
    or      r4, r0, r8
.L_dp_second_half:
    srwi    r8, r7, 16
    clrlwi  r12, r7, 16
    divwu   r0, r31, r8
    srwi    r11, r30, 16
    mullw   r9, r0, r8
    mr      r5, r0
    mullw   r6, r5, r12
    subf    r10, r9, r31
    slwi    r0, r10, 16
    or      r10, r0, r11
    cmplw   r10, r6
    bge     .L_dp2_q1_ok
    add     r10, r10, r7
    subi    r5, r5, 0x1
    cmplw   r10, r7
    blt     .L_dp2_q1_ok
    cmplw   r10, r6
    bge     .L_dp2_q1_ok
    subi    r5, r5, 0x1
    add     r10, r10, r7
.L_dp2_q1_ok:
    subf    r10, r6, r10
    clrlwi  r11, r30, 16
    divwu   r0, r10, r8
    mullw   r9, r0, r8
    mr      r8, r0
    mullw   r6, r8, r12
    subf    r0, r9, r10
    slwi    r0, r0, 16
    or      r0, r0, r11
    cmplw   r0, r6
    bge     .L_dp2_q0_ok
    add     r0, r0, r7
    subi    r8, r8, 0x1
    cmplw   r0, r7
    blt     .L_dp2_q0_ok
    cmplw   r0, r6
    bge     .L_dp2_q0_ok
    subi    r8, r8, 0x1
.L_dp2_q0_ok:
    slwi    r0, r5, 16
    or      r6, r0, r8
    b       .L_done_q1
.L_den_hi_nonzero:
    # d0 != 0
    cmplw   r10, r31
    ble     .L_den_le_num
    li      r6, 0x0
    b       .L_done_q0_zero
.L_den_le_num:
    li      r11, 0x0
    ori     r11, r11, 0xffff
    cmplw   r10, r11
    bgt     .L_count_shift_2w1
    subfic  r0, r10, 0xff
    subfe   r0, r0, r0
    rlwinm  r8, r0, 0, 28, 28
    b       .L_shift_done_2w1
.L_count_shift_2w1:
    lis     r0, 0xff
    ori     r0, r0, 0xffff
    subfc   r0, r10, r0
    subfe   r0, r0, r0
    rlwinm  r0, r0, 0, 27, 28
    ori     r8, r0, 0x10
.L_shift_done_2w1:
    srw     r11, r10, r8
    li      r9, __clz_tab@sda21
    lbzx    r0, r9, r11
    add     r0, r0, r8
    subfic  r5, r0, 0x20
    cmpwi   r5, 0x0
    bne     .L_bm_nonzero_2w
    # bm == 0
    subfc   r9, r31, r10
    subfe   r9, r9, r9
    neg     r9, r9
    subfc   r0, r7, r30
    li      r0, 0x0
    adde    r0, r0, r0
    or.     r10, r9, r0
    li      r6, 0x0
    beq     .L_done_q0_zero
    li      r6, 0x1
    b       .L_done_q0_zero
.L_bm_nonzero_2w:
    subfic  r8, r5, 0x20
    slw     r10, r10, r5
    srw     r11, r30, r8
    srw     r9, r7, r8
    srw     r12, r31, r8
    slw     r0, r31, r5
    or      r31, r0, r11
    or      r10, r10, r9
    slw     r30, r30, r5
    slw     r7, r7, r5
    srwi    r6, r10, 16
    divwu   r0, r12, r6
    clrlwi  r3, r10, 16
    srwi    r11, r31, 16
    mullw   r9, r0, r6
    mr      r4, r0
    mullw   r5, r4, r3
    subf    r8, r9, r12
    slwi    r0, r8, 16
    or      r8, r0, r11
    cmplw   r8, r5
    bge     .L_2w_q1_ok
    add     r8, r8, r10
    subi    r4, r4, 0x1
    cmplw   r8, r10
    blt     .L_2w_q1_ok
    cmplw   r8, r5
    bge     .L_2w_q1_ok
    subi    r4, r4, 0x1
    add     r8, r8, r10
.L_2w_q1_ok:
    subf    r8, r5, r8
    clrlwi  r11, r31, 16
    divwu   r0, r8, r6
    mullw   r9, r0, r6
    mr      r6, r0
    mullw   r5, r6, r3
    subf    r3, r9, r8
    slwi    r0, r3, 16
    or      r3, r0, r11
    cmplw   r3, r5
    bge     .L_2w_q0_ok
    add     r3, r3, r10
    subi    r6, r6, 0x1
    cmplw   r3, r10
    blt     .L_2w_q0_ok
    cmplw   r3, r5
    bge     .L_2w_q0_ok
    add     r3, r3, r10
    subi    r6, r6, 0x1
.L_2w_q0_ok:
    slwi    r9, r4, 16
    subf    r3, r5, r3
    or      r6, r9, r6
    # Compute q * d1 and check
    clrlwi  r0, r6, 16
    clrlwi  r9, r7, 16
    srwi    r11, r6, 16
    mullw   r8, r0, r9
    srwi    r7, r7, 16
    mullw   r0, r0, r7
    mullw   r9, r11, r9
    srwi    r10, r8, 16
    clrlwi  r8, r8, 16
    mullw   r11, r11, r7
    add     r0, r0, r10
    add     r0, r0, r9
    subfc   r9, r9, r0
    subfe   r9, r9, r9
    nand    r9, r9, r9
    slwi    r7, r0, 16
    addis   r10, r11, 0x1
    srwi    r0, r0, 16
    and     r11, r11, r9
    add     r7, r7, r8
    andc    r9, r10, r9
    or      r11, r11, r9
    add     r0, r11, r0
    cmplw   r0, r3
    bgt     .L_q_too_large
    subfc   r9, r7, r30
    subfe   r9, r9, r9
    neg     r9, r9
    xor     r0, r0, r3
    subfic  r11, r0, 0x0
    adde    r0, r11, r0
    and.    r10, r0, r9
    beq     .L_done_q0_zero
.L_q_too_large:
    subi    r6, r6, 0x1
.L_done_q0_zero:
    li      r4, 0x0
.L_done_q1:
    mr      r27, r6
    cmpwi   r25, 0x0
    mr      r26, r4
    mr      r3, r26
    mr      r4, r27
    beq     .L_no_negate
    neg     r29, r4
    neg     r9, r3
    subic   r11, r29, 0x1
    subfe   r0, r11, r29
    subf    r28, r0, r9
    mr      r3, r28
    mr      r4, r29
.L_no_negate:
    lmw     r25, 0xc(r1)
    addi    r1, r1, 0x28
    blr
.endfn __divdi3

.section .data5
.balign 8

.obj __clz_tab, local
    .4byte 0x00010202
    .4byte 0x03030303
    .4byte 0x04040404
    .4byte 0x04040404
    .4byte 0x05050505
    .4byte 0x05050505
    .4byte 0x05050505
    .4byte 0x05050505
    .4byte 0x06060606
    .4byte 0x06060606
    .4byte 0x06060606
    .4byte 0x06060606
    .4byte 0x06060606
    .4byte 0x06060606
    .4byte 0x06060606
    .4byte 0x06060606
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x07070707
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
    .4byte 0x08080808
.endobj __clz_tab
