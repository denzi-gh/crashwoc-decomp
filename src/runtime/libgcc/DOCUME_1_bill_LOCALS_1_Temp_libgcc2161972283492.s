.include "macros.inc"
.file "DOCUME_1_bill_LOCALS_1_Temp_libgcc2161972283492.s"

# 0x800DC3F0..0x800DC908 | size: 0x518
.text
.balign 4
# .text:0x0 | 0x800DC3F0 | size: 0x0
.sym gcc2_compiled., local

# .text:0x0 | 0x800DC3F0 | size: 0x518
.fn __udivdi3, global
	stwu r1, -0x18(r1)
	stmw r28, 0x8(r1)
	mr. r10, r5
	mr r31, r3
	mr r7, r6
	mr r30, r4
	bne .L_800DC700
	cmplw r7, r31
	ble .L_800DC528
	li r0, 0x0
	mr r11, r6
	ori r0, r0, 0xffff
	cmplw r7, r0
	bgt .L_800DC438
	subfic r0, r7, 0xff
	subfe r0, r0, r0
	rlwinm r5, r0, 0, 28, 28
	b .L_800DC450
.L_800DC438:
	lis r0, 0xff
	ori r0, r0, 0xffff
	subfc r0, r7, r0
	subfe r0, r0, r0
	rlwinm r0, r0, 0, 27, 28
	ori r5, r0, 0x10
.L_800DC450:
	srw r11, r11, r5
	li r9, __clz_tab@sda21
	lbzx r0, r9, r11
	add r0, r0, r5
	subfic r5, r0, 0x20
	cmpwi r5, 0x0
	beq .L_800DC484
	subfic r0, r5, 0x20
	slw r9, r31, r5
	srw r0, r30, r0
	slw r7, r7, r5
	or r31, r9, r0
	slw r30, r30, r5
.L_800DC484:
	srwi r10, r7, 16
	clrlwi r5, r7, 16
	divwu r0, r31, r10
	srwi r11, r30, 16
	mullw r9, r0, r10
	mr r6, r0
	mullw r8, r6, r5
	subf r3, r9, r31
	slwi r0, r3, 16
	or r3, r0, r11
	cmplw r3, r8
	bge .L_800DC4D4
	add r3, r3, r7
	subi r6, r6, 0x1
	cmplw r3, r7
	blt .L_800DC4D4
	cmplw r3, r8
	bge .L_800DC4D4
	subi r6, r6, 0x1
	add r3, r3, r7
.L_800DC4D4:
	subf r3, r8, r3
	clrlwi r11, r30, 16
	divwu r0, r3, r10
	mullw r9, r0, r10
	mr r10, r0
	mullw r8, r10, r5
	subf r0, r9, r3
	slwi r0, r0, 16
	or r0, r0, r11
	cmplw r0, r8
	bge .L_800DC51C
	add r0, r0, r7
	subi r10, r10, 0x1
	cmplw r0, r7
	blt .L_800DC51C
	cmplw r0, r8
	bge .L_800DC51C
	subi r10, r10, 0x1
.L_800DC51C:
	slwi r0, r6, 16
	or r6, r0, r10
	b .L_800DC8E8
.L_800DC528:
	cmpwi r7, 0x0
	bne .L_800DC538
	li r0, 0x1
	divwu r7, r0, r10
.L_800DC538:
	li r11, 0x0
	ori r11, r11, 0xffff
	cmplw r7, r11
	bgt .L_800DC558
	subfic r0, r7, 0xff
	subfe r0, r0, r0
	rlwinm r10, r0, 0, 28, 28
	b .L_800DC570
.L_800DC558:
	lis r0, 0xff
	ori r0, r0, 0xffff
	subfc r0, r7, r0
	subfe r0, r0, r0
	rlwinm r0, r0, 0, 27, 28
	ori r10, r0, 0x10
.L_800DC570:
	srw r11, r7, r10
	li r9, __clz_tab@sda21
	lbzx r0, r9, r11
	add r0, r0, r10
	subfic r5, r0, 0x20
	cmpwi r5, 0x0
	bne .L_800DC598
	subf r31, r7, r31
	li r4, 0x1
	b .L_800DC65C
.L_800DC598:
	subfic r8, r5, 0x20
	slw r9, r31, r5
	srw r0, r30, r8
	srw r12, r31, r8
	or r31, r9, r0
	slw r30, r30, r5
	slw r7, r7, r5
	srwi r8, r7, 16
	divwu r0, r12, r8
	clrlwi r4, r7, 16
	srwi r11, r31, 16
	mullw r9, r0, r8
	mr r5, r0
	mullw r6, r5, r4
	subf r10, r9, r12
	slwi r0, r10, 16
	or r10, r0, r11
	cmplw r10, r6
	bge .L_800DC604
	add r10, r10, r7
	subi r5, r5, 0x1
	cmplw r10, r7
	blt .L_800DC604
	cmplw r10, r6
	bge .L_800DC604
	subi r5, r5, 0x1
	add r10, r10, r7
.L_800DC604:
	subf r10, r6, r10
	clrlwi r11, r31, 16
	divwu r0, r10, r8
	mullw r9, r0, r8
	mr r8, r0
	mullw r6, r8, r4
	subf r3, r9, r10
	slwi r0, r3, 16
	or r3, r0, r11
	cmplw r3, r6
	bge .L_800DC650
	add r3, r3, r7
	subi r8, r8, 0x1
	cmplw r3, r7
	blt .L_800DC650
	cmplw r3, r6
	bge .L_800DC650
	subi r8, r8, 0x1
	add r3, r3, r7
.L_800DC650:
	slwi r0, r5, 16
	subf r31, r6, r3
	or r4, r0, r8
.L_800DC65C:
	srwi r10, r7, 16
	clrlwi r5, r7, 16
	divwu r0, r31, r10
	srwi r11, r30, 16
	mullw r9, r0, r10
	mr r6, r0
	mullw r8, r6, r5
	subf r3, r9, r31
	slwi r0, r3, 16
	or r3, r0, r11
	cmplw r3, r8
	bge .L_800DC6AC
	add r3, r3, r7
	subi r6, r6, 0x1
	cmplw r3, r7
	blt .L_800DC6AC
	cmplw r3, r8
	bge .L_800DC6AC
	subi r6, r6, 0x1
	add r3, r3, r7
.L_800DC6AC:
	subf r3, r8, r3
	clrlwi r11, r30, 16
	divwu r0, r3, r10
	mullw r9, r0, r10
	mr r10, r0
	mullw r8, r10, r5
	subf r0, r9, r3
	slwi r0, r0, 16
	or r0, r0, r11
	cmplw r0, r8
	bge .L_800DC6F4
	add r0, r0, r7
	subi r10, r10, 0x1
	cmplw r0, r7
	blt .L_800DC6F4
	cmplw r0, r8
	bge .L_800DC6F4
	subi r10, r10, 0x1
.L_800DC6F4:
	slwi r0, r6, 16
	or r6, r0, r10
	b .L_800DC8EC
.L_800DC700:
	cmplw r10, r31
	ble .L_800DC710
	li r6, 0x0
	b .L_800DC8E8
.L_800DC710:
	li r0, 0x0
	ori r0, r0, 0xffff
	cmplw r10, r0
	bgt .L_800DC730
	subfic r0, r10, 0xff
	subfe r0, r0, r0
	rlwinm r8, r0, 0, 28, 28
	b .L_800DC748
.L_800DC730:
	lis r0, 0xff
	ori r0, r0, 0xffff
	subfc r0, r10, r0
	subfe r0, r0, r0
	rlwinm r0, r0, 0, 27, 28
	ori r8, r0, 0x10
.L_800DC748:
	srw r11, r10, r8
	li r9, __clz_tab@sda21
	lbzx r0, r9, r11
	add r0, r0, r8
	subfic r5, r0, 0x20
	cmpwi r5, 0x0
	bne .L_800DC790
	subfc r9, r31, r10
	subfe r9, r9, r9
	neg r9, r9
	subfc r0, r7, r30
	li r0, 0x0
	adde r0, r0, r0
	or. r11, r9, r0
	li r6, 0x0
	beq .L_800DC8E8
	li r6, 0x1
	b .L_800DC8E8
.L_800DC790:
	subfic r8, r5, 0x20
	slw r10, r10, r5
	srw r11, r30, r8
	srw r9, r7, r8
	srw r12, r31, r8
	slw r0, r31, r5
	or r31, r0, r11
	or r10, r10, r9
	slw r30, r30, r5
	slw r7, r7, r5
	srwi r6, r10, 16
	divwu r0, r12, r6
	clrlwi r3, r10, 16
	srwi r11, r31, 16
	mullw r9, r0, r6
	mr r4, r0
	mullw r5, r4, r3
	subf r8, r9, r12
	slwi r0, r8, 16
	or r8, r0, r11
	cmplw r8, r5
	bge .L_800DC808
	add r8, r8, r10
	subi r4, r4, 0x1
	cmplw r8, r10
	blt .L_800DC808
	cmplw r8, r5
	bge .L_800DC808
	subi r4, r4, 0x1
	add r8, r8, r10
.L_800DC808:
	subf r8, r5, r8
	clrlwi r11, r31, 16
	divwu r0, r8, r6
	mullw r9, r0, r6
	mr r6, r0
	mullw r5, r6, r3
	subf r3, r9, r8
	slwi r0, r3, 16
	or r3, r0, r11
	cmplw r3, r5
	bge .L_800DC854
	add r3, r3, r10
	subi r6, r6, 0x1
	cmplw r3, r10
	blt .L_800DC854
	cmplw r3, r5
	bge .L_800DC854
	add r3, r3, r10
	subi r6, r6, 0x1
.L_800DC854:
	slwi r9, r4, 16
	subf r3, r5, r3
	or r6, r9, r6
	clrlwi r0, r6, 16
	clrlwi r9, r7, 16
	srwi r11, r6, 16
	mullw r8, r0, r9
	srwi r7, r7, 16
	mullw r0, r0, r7
	mullw r9, r11, r9
	srwi r10, r8, 16
	clrlwi r8, r8, 16
	mullw r11, r11, r7
	add r0, r0, r10
	add r0, r0, r9
	subfc r9, r9, r0
	subfe r9, r9, r9
	nand r9, r9, r9
	slwi r7, r0, 16
	addis r10, r11, 0x1
	srwi r0, r0, 16
	and r11, r11, r9
	add r7, r7, r8
	andc r9, r10, r9
	or r11, r11, r9
	add r0, r11, r0
	cmplw r0, r3
	bgt .L_800DC8E4
	subfc r9, r7, r30
	subfe r9, r9, r9
	neg r9, r9
	xor r0, r0, r3
	subfic r11, r0, 0x0
	adde r0, r11, r0
	and. r11, r0, r9
	beq .L_800DC8E8
.L_800DC8E4:
	subi r6, r6, 0x1
.L_800DC8E8:
	li r4, 0x0
.L_800DC8EC:
	mr r29, r6
	mr r28, r4
	mr r3, r28
	mr r4, r29
	lmw r28, 0x8(r1)
	addi r1, r1, 0x18
	blr
.endfn __udivdi3

# 0x803D53D8..0x803D54D8 | size: 0x100
.section .data5
.balign 8

# .data5:0x0 | 0x803D53D8 | size: 0x100
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
