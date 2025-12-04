	.file	"trigger"
	.text
	.globl	main                            # -- Begin function main
	.p2align	2
	.type	main,@function
main:                                   # @main
# %bb.0:                                # %entry
	mov %counter0, 10
	mov %counter1, 12
	trigger_pos	90
	mov %r0, 268553420
	load %r1, 4
	jmpc 1, %r1, %r0, @.LBB0_2
# %bb.1:                                # %then
	trigger
.LBB0_2:                                # %merge
	bl	@inst1
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        # -- End function
	.p2align	2                               # -- Begin function inst1
	.type	inst1,@function
inst1:                                  # @inst1
# %bb.0:                                # %entry
	domain_set 0
	load %r4, 4104
	bit_slice %r0, %r4, 5, 5
	edge_detect %r0, %r0, 1
	mov %r2, 1
	jmpc 1, %r0, %r2, @.LBB1_2
# %bb.1:                                # %then
	arith_op %counter0, %counter0, %r2, 8
	timer_set 0, 0
	display	0
	exec 0
	ret
.LBB1_2:                                # %else
	domain_set 1
	mov %r0, -1
	arith_op %counter0, %counter0, %r0, 8
	load %r3, 4096
	load %r0, 4
	bit_slice %r1, %r0, 3, 0
	bit_slice %r5, %r3, 3, 0
	arith_op %r5, %r1, %r5, 6
	display	1
	mov %r1, 0
	arith_op %r6, %r0, %r1, 7
	arith_op %r6, %r0, %r3, 2
	arith_op %r7, %r6, %r1, 7
	arith_op %r5, %r5, %r1, 7
	display	2
	display	3
	display	4
	bit_slice %r6, %r6, 3, 0
	mov %r7, 13
	mov %r5, 15
	jmpc 3, %r6, %r7, @.LBB1_5
# %bb.3:                                # %else
	jmpc 0, %counter0, %r5, @.LBB1_5
# %bb.4:                                # %then5
	timer_set 0, 1
	display	5
	exec 1
	ret
.LBB1_5:                                # %else6
	jmpc 0, %counter1, %r5, @.LBB1_13
# %bb.6:                                # %else6
	jmpc 1, %counter0, %r5, @.LBB1_7
	jmp	@.LBB1_13
.LBB1_7:                                # %else12
	domain_set 2
	mov %counter1, %r1
	mov %counter0, %r1
	bit_slice %r6, %r4, 1, 0
	bit_slice %r7, %r3, 1, 0
	load %r5, 4100
	bit_slice %r8, %r5, 1, 0
	arith_op %r7, %r7, %r8, 6
	arith_op %r6, %r6, %r7, 6
	display	6
	arith_op %r7, %r4, %r1, 7
	arith_op %r6, %r6, %r1, 7
	display	7
	display	8
	bit_slice %r6, %r4, 3, 0
	bit_slice %r7, %r6, 3, 3
	edge_detect %r8, %r7, 0
	bit_slice %r7, %r6, 1, 1
	edge_detect %r7, %r7, 2
	bit_slice %r6, %r6, 0, 0
	edge_detect %r6, %r6, 4
	jmpc 1, %r8, %r2, @.LBB1_11
# %bb.8:                                # %else12
	jmpc 0, %r1, %r2, @.LBB1_11
# %bb.9:                                # %else12
	jmpc 1, %r7, %r2, @.LBB1_11
# %bb.10:                               # %else12
	jmpc 0, %r6, %r2, @.LBB1_13
.LBB1_11:                               # %else23
	arith_op %r6, %r3, %r5, 0
	bit_slice %r7, %r6, 0, 0
	bit_slice %r4, %r4, 0, 0
	arith_op %r4, %r7, %r4, 6
	arith_op %r4, %r4, %r1, 7
	display	9
	display	10
	bit_slice %r4, %r6, 1, 1
	edge_detect %r4, %r4, 1
	jmpc 1, %r4, %r2, @.LBB1_14
.LBB1_13:                               # %then11
	trigger
	ret
.LBB1_14:                               # %else27
	display	11
	domain_set 0
	bit_slice %r2, %r0, 7, 0
	arith_op %r4, %r2, %r1, 5
	display	12
	bit_slice %r2, %r3, 3, 0
	arith_op %r2, %r2, %r1, 3
	display	13
	bit_slice %r3, %r5, 7, 0
	arith_op %r3, %r3, %r1, 4
	display	14
	bit_slice %r0, %r0, 7, 0
	display	15
	jmpc 1, %r4, %r1, @.LBB1_15
	jmp	@.LBB1_16
.LBB1_15:                               # %then31
	display	16
	exec 2
	ret
.LBB1_16:                               # %else32
	jmpc 1, %r2, %r1, @.LBB1_17
	jmp	@.LBB1_18
.LBB1_17:                               # %else36
	display	18
	trigger
	ret
.LBB1_18:                               # %then35
	display	17
	trigger
	ret
.Lfunc_end1:
	.size	inst1, .Lfunc_end1-inst1
                                        # -- End function
	.type	counter0,@object                # @counter0
	.data
	.globl	counter0
	.p2align	2, 0x0
counter0:
	.word	10                              # 0xa
	.size	counter0, 4

	.type	counter1,@object                # @counter1
	.globl	counter1
	.p2align	2, 0x0
counter1:
	.word	12                              # 0xc
	.size	counter1, 4

	.section	".note.GNU-stack","",@progbits
