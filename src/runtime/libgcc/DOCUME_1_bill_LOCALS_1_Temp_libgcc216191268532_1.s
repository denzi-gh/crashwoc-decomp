.include "macros.inc"

.text
.balign 4

.sym gcc2_compiled., local
.endsym gcc2_compiled.

.fn __moddi3, global
    stwu    r1, -0x38(r1)
    stmw    r23, 0x14(r1)
    cmpwi   r3, 0x0
    li      r23, 0x0
    bge     .L_pos_u
    neg     r12, r4
    neg     r9, r3
    subic   r10, r12, 0x1
    subfe   r0, r10, r12
    li      r23, -0x1
    subf    r11, r0, r9
    mr      r3, r11
    mr      r4, r12
.L_pos_u:
    cmpwi   r5, 0x0
    bge     .L_pos_v
    neg     r8, r6
    neg     r9, r5
    subic   r11, r8, 0x1
    subfe   r0, r11, r8
    subf    r7, r0, r9
    mr      r5, r7
    mr      r6, r8
.L_pos_v:
    mr.     r31, r5
    addi    r24, r1, 0x8
    mr      r12, r6
    mr      r29, r4
    mr      r30, r3
    bne     .L_den_hi_nonzero
    cmplw   r12, r30
    ble     .L_den_ge_num_hi
    # d0 == 0 && d1 > n1 => single precision
    li      r0, 0x0
    mr      r9, r6
    ori     r0, r0, 0xffff
    cmplw   r12, r0
    bgt     .L_count_shift_sp1
    subfic  r0, r12, 0xff
    subfe   r0, r0, r0
    rlwinm  r5, r0, 0, 28, 28
    b       .L_shift_done_sp1
.L_count_shift_sp1:
    lis     r0, 0xff
    ori     r0, r0, 0xffff
    subfc   r0, r12, r0
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
    slw     r9, r30, r5
    srw     r0, r29, r0
    slw     r12, r12, r5
    or      r30, r9, r0
    slw     r29, r29, r5
    b       .L_bm_zero_sp
.L_den_ge_num_hi:
    # d0 == 0 && d1 <= n1
    cmpwi   r12, 0x0
    bne     .L_den_nonzero
    li      r0, 0x1
    divwu   r12, r0, r31
.L_den_nonzero:
    li      r10, 0x0
    ori     r10, r10, 0xffff
    cmplw   r12, r10
    bgt     .L_count_shift_dp1
    subfic  r0, r12, 0xff
    subfe   r0, r0, r0
    rlwinm  r10, r0, 0, 28, 28
    b       .L_shift_done_dp1
.L_count_shift_dp1:
    lis     r0, 0xff
    ori     r0, r0, 0xffff
    subfc   r0, r12, r0
    subfe   r0, r0, r0
    rlwinm  r0, r0, 0, 27, 28
    ori     r10, r0, 0x10
.L_shift_done_dp1:
    srw     r11, r12, r10
    li      r9, __clz_tab@sda21
    lbzx    r0, r9, r11
    add     r0, r0, r10
    subfic  r5, r0, 0x20
    cmpwi   r5, 0x0
    bne     .L_bm_nonzero_dp
    subf    r30, r12, r30
    b       .L_bm_zero_sp
.L_bm_nonzero_dp:
    subfic  r6, r5, 0x20
    slw     r9, r30, r5
    srw     r0, r29, r6
    srw     r8, r30, r6
    or      r30, r9, r0
    slw     r12, r12, r5
    slw     r29, r29, r5
    srwi    r4, r12, 16
    divwu   r0, r8, r4
    clrlwi  r10, r12, 16
    srwi    r11, r30, 16
    mullw   r9, r0, r4
    mullw   r7, r0, r10
    subf    r8, r9, r8
    slwi    r0, r8, 16
    or      r8, r0, r11
    cmplw   r8, r7
    bge     .L_dp1_q1_ok
    add     r8, r8, r12
    cmplw   r8, r12
    blt     .L_dp1_q1_ok
    cmplw   r8, r7
    bge     .L_dp1_q1_ok
    add     r8, r8, r12
.L_dp1_q1_ok:
    subf    r8, r7, r8
    clrlwi  r11, r30, 16
    divwu   r0, r8, r4
    mullw   r9, r0, r4
    mullw   r7, r0, r10
    subf    r0, r9, r8
    slwi    r0, r0, 16
    or      r0, r0, r11
    cmplw   r0, r7
    bge     .L_dp1_q0_ok
    add     r0, r0, r12
    cmplw   r0, r12
    blt     .L_dp1_q0_ok
    cmplw   r0, r7
    bge     .L_dp1_q0_ok
    add     r0, r0, r12
.L_dp1_q0_ok:
    subf    r30, r7, r0
.L_bm_zero_sp:
    srwi    r7, r12, 16
    clrlwi  r4, r12, 16
    divwu   r0, r30, r7
    srwi    r11, r29, 16
    mullw   r9, r0, r7
    mullw   r8, r0, r4
    subf    r10, r9, r30
    slwi    r0, r10, 16
    or      r10, r0, r11
    cmplw   r10, r8
    bge     .L_sp2_q1_ok
    add     r10, r10, r12
    cmplw   r10, r12
    blt     .L_sp2_q1_ok
    cmplw   r10, r8
    bge     .L_sp2_q1_ok
    add     r10, r10, r12
.L_sp2_q1_ok:
    subf    r10, r8, r10
    clrlwi  r11, r29, 16
    divwu   r0, r10, r7
    mullw   r9, r0, r7
    mullw   r8, r0, r4
    subf    r0, r9, r10
    slwi    r0, r0, 16
    or      r0, r0, r11
    cmplw   r0, r8
    bge     .L_sp2_q0_ok
    add     r0, r0, r12
    cmplw   r0, r12
    blt     .L_sp2_q0_ok
    cmplw   r0, r8
    bge     .L_sp2_q0_ok
    add     r0, r0, r12
.L_sp2_q0_ok:
    subf    r29, r8, r0
    cmpwi   r24, 0x0
    beq     .L_done
    srw     r28, r29, r5
    li      r27, 0x0
    b       .L_store_rem
.L_den_hi_nonzero:
    # d0 != 0
    cmplw   r31, r30
    ble     .L_den_le_num
    mr      r28, r4
    mr      r27, r30
    stw     r27, 0x8(r1)
    stw     r28, 0xc(r1)
    b       .L_done
.L_den_le_num:
    li      r11, 0x0
    ori     r11, r11, 0xffff
    cmplw   r31, r11
    bgt     .L_count_shift_2w1
    subfic  r0, r31, 0xff
    subfe   r0, r0, r0
    rlwinm  r10, r0, 0, 28, 28
    b       .L_shift_done_2w1
.L_count_shift_2w1:
    lis     r0, 0xff
    ori     r0, r0, 0xffff
    subfc   r0, r31, r0
    subfe   r0, r0, r0
    rlwinm  r0, r0, 0, 27, 28
    ori     r10, r0, 0x10
.L_shift_done_2w1:
    srw     r11, r31, r10
    li      r9, __clz_tab@sda21
    lbzx    r0, r9, r11
    add     r0, r0, r10
    subfic  r5, r0, 0x20
    cmpwi   r5, 0x0
    bne     .L_bm_nonzero_2w
    # bm == 0
    subfc   r9, r30, r31
    subfe   r9, r9, r9
    neg     r9, r9
    subfc   r0, r12, r29
    li      r0, 0x0
    adde    r0, r0, r0
    or.     r10, r9, r0
    beq     .L_2w_bm0_no_sub
    subf    r11, r12, r29
    subf    r9, r31, r30
    subfc   r0, r11, r29
    subfe   r0, r0, r0
    neg     r0, r0
    subf    r30, r0, r9
    mr      r29, r11
.L_2w_bm0_no_sub:
    cmpwi   r24, 0x0
    beq     .L_done
    mr      r28, r29
    mr      r27, r30
    b       .L_store_rem
.L_bm_nonzero_2w:
    subfic  r6, r5, 0x20
    slw     r10, r31, r5
    srw     r11, r12, r6
    srw     r8, r30, r6
    srw     r0, r29, r6
    slw     r9, r30, r5
    or      r30, r9, r0
    or      r31, r10, r11
    slw     r12, r12, r5
    slw     r29, r29, r5
    srwi    r7, r31, 16
    divwu   r0, r8, r7
    clrlwi  r3, r31, 16
    srwi    r11, r30, 16
    mullw   r9, r0, r7
    mr      r10, r0
    mullw   r4, r10, r3
    subf    r8, r9, r8
    slwi    r0, r8, 16
    or      r8, r0, r11
    cmplw   r8, r4
    bge     .L_2w_q1_ok
    add     r8, r8, r31
    subi    r10, r10, 0x1
    cmplw   r8, r31
    blt     .L_2w_q1_ok
    cmplw   r8, r4
    bge     .L_2w_q1_ok
    subi    r10, r10, 0x1
    add     r8, r8, r31
.L_2w_q1_ok:
    subf    r8, r4, r8
    clrlwi  r11, r30, 16
    divwu   r0, r8, r7
    mullw   r9, r0, r7
    mr      r7, r0
    mullw   r4, r7, r3
    subf    r3, r9, r8
    slwi    r0, r3, 16
    or      r3, r0, r11
    cmplw   r3, r4
    bge     .L_2w_q0_ok
    add     r3, r3, r31
    subi    r7, r7, 0x1
    cmplw   r3, r31
    blt     .L_2w_q0_ok
    cmplw   r3, r4
    bge     .L_2w_q0_ok
    subi    r7, r7, 0x1
    add     r3, r3, r31
.L_2w_q0_ok:
    subf    r3, r4, r3
    slwi    r9, r10, 16
    mr      r30, r3
    or      r9, r9, r7
    clrlwi  r0, r9, 16
    clrlwi  r11, r12, 16
    srwi    r7, r12, 16
    mullw   r8, r0, r11
    srwi    r9, r9, 16
    mullw   r0, r0, r7
    mullw   r11, r9, r11
    srwi    r10, r8, 16
    clrlwi  r8, r8, 16
    mullw   r9, r9, r7
    add     r0, r0, r10
    add     r0, r0, r11
    subfc   r11, r11, r0
    subfe   r11, r11, r11
    nand    r11, r11, r11
    slwi    r7, r0, 16
    addis   r10, r9, 0x1
    srwi    r0, r0, 16
    and     r9, r9, r11
    add     r7, r7, r8
    andc    r11, r10, r11
    or      r9, r9, r11
    add     r10, r9, r0
    cmplw   r10, r30
    bgt     .L_q_too_large
    xor     r9, r10, r30
    subfic  r11, r9, 0x0
    adde    r9, r11, r9
    subfc   r0, r7, r29
    subfe   r0, r0, r0
    neg     r0, r0
    and.    r11, r9, r0
    beq     .L_q_ok
.L_q_too_large:
    subf    r11, r12, r7
    subf    r9, r31, r10
    subfc   r0, r11, r7
    subfe   r0, r0, r0
    neg     r0, r0
    subf    r10, r0, r9
    mr      r7, r11
.L_q_ok:
    cmpwi   r24, 0x0
    beq     .L_done
    subf    r11, r7, r29
    subf    r9, r10, r3
    subfc   r0, r11, r29
    subfe   r0, r0, r0
    neg     r0, r0
    subf    r30, r0, r9
    slw     r10, r30, r6
    srw     r11, r11, r5
    or      r28, r10, r11
    srw     r27, r30, r5
.L_store_rem:
    stw     r27, 0x0(r24)
    stw     r28, 0x4(r24)
.L_done:
    cmpwi   r23, 0x0
    beq     .L_no_negate
    lwz     r9, 0x8(r1)
    lwz     r10, 0xc(r1)
    neg     r26, r10
    neg     r9, r9
    subic   r10, r26, 0x1
    subfe   r0, r10, r26
    subf    r25, r0, r9
    stw     r25, 0x8(r1)
    stw     r26, 0xc(r1)
.L_no_negate:
    lwz     r3, 0x8(r1)
    lwz     r4, 0xc(r1)
    lmw     r23, 0x14(r1)
    addi    r1, r1, 0x38
    blr
.endfn __moddi3

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
