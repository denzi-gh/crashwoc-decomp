.include "macros.inc"

.text
.balign 4

.sym l0000004C, local

.fn pad_01_800D63B0_text, local
	cmplwi r0, 0x3
	bne l0000006C
	lbz r6, 0x1(r3)
	addi r7, r3, 0x1
	li r8, 0x8
	extsb r6, r6
	li r11, 0x20
	li r12, 0x8
l0000006C:
	clrlwi r0, r4, 24
	cmplwi r0, 0x2
	bne l00000090
	clrlwi. r0, r6, 31
	li r8, 0x8
	li r5, 0x7
	beq l0000008C
	li r10, 0x1
l0000008C:
	li r9, 0x2
l00000090:
	cmpw r6, r5
	bge l000000B8
	add r6, r6, r10
	lwz r5, 0x8(r3)
	mullw r3, r6, r12
	add r0, r6, r9
	add r6, r11, r3
	stb r0, 0x0(r7)
	add r6, r5, r6
	b l000000E0
l000000B8:
	li r0, 0x8
	stb r0, 0x0(r7)
	subi r0, r8, 0x1
	nor r6, r0, r0
	lwz r0, 0x4(r3)
	add r5, r8, r0
	subi r0, r5, 0x1
	and r6, r6, r0
	add r0, r6, r8
	stw r0, 0x4(r3)
l000000E0:
	clrlwi. r0, r4, 24
	bne l000000EC
	lwz r6, 0x0(r6)
l000000EC:
	mr r3, r6
	blr
.endfn pad_01_800D63B0_text
