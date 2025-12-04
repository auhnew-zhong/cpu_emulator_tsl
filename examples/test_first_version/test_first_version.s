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
	load %r1, 4104
	bit_slice %r0, %r1, 5, 5
	edge_detect %r2, %r0, 1
	mov %r0, 1
	jmpc 1, %r2, %r0, @.LBB1_2
# %bb.1:                                # %then
	arith_op %r0, %counter0, %r0, 3
	timer_set 0, 0
	display	0
	ret
.LBB1_2:                                # %else
	domain_set 0
	mov %r2, -1
	arith_op %counter0, %counter0, %r2, 3
	load %r2, 4096
	load %r3, 4
	arith_op %r3, %r3, %r2, 2
	bit_slice %r3, %r3, 3, 0
	mov %r4, 13
	jmpc 3, %r3, %r4, @.LBB1_5
# %bb.3:                                # %then2
	timer_set 0, 1
	display	1
	mov %counter0, 0
                                        # kill: def $counter1 killed $counter0
	edge_detect %r1, %r1, 0
	jmpc 0, %r1, %r0, @.LBB1_4
	jmp	@.LBB1_8
.LBB1_4:                                # %then10
	trigger
	ret
.LBB1_5:                                # %else3
	mov %r0, 15
	jmpc 1, %counter1, %r0, @.LBB1_7
# %bb.6:                                # %then6
	trigger
.LBB1_7:                                # %merge8
	ret
.LBB1_8:                                # %else11
	load %r1, 4100
	arith_op %r1, %r2, %r1, 0
	bit_slice %r1, %r1, 1, 1
	edge_detect %r1, %r1, 1
	jmpc 1, %r1, %r0, @.LBB1_10
# %bb.9:                                # %then13
	trigger
	ret
.LBB1_10:                               # %else14
	display	2
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
