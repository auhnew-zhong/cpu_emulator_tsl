	.file	"trigger"
	.text
	.globl	main                            # -- Begin function main
	.p2align	2
	.type	main,@function
main:                                   # @main
# %bb.0:                                # %entry
	mov %counter0, 10
	mov %counter1, 12
	timer_set 0, 3, -294967296, @.Ltmp0
	trigger_pos	10
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
	load %r0, 4104
	bit_slice %r0, %r0, 5, 5
	edge_detect %r1, %r0, 1
	mov %r0, 1
	jmpc 0, %r1, %r0, @.LBB1_1
	jmp	@.LBB1_2
.LBB1_1:                                # %then
	arith_op %counter0, %counter0, %r0, 8
	timer_set 0, 1, -294967296, @.Ltmp0
	send 0, 0, 0
	send 1, 1, 0
	ret
.LBB1_2:                                # %else
	send 4, 2, 0
	send 3, 3, 0
	send 8, 4, 0
	send 6, 5, 0
	send 0, 6, 0
.Ltmp0:                                 # Block address taken
# %bb.3:                                # %s2
	domain_set 1
	load %r1, 4
	jmpc 0, %r1, %r0, @.LBB1_4
	jmp	@.LBB1_5
.LBB1_4:                                # %then1
	send 5, 7, 0
	trigger
	ret
.LBB1_5:                                # %else2
	send 2, 8, 0
	send 7, 9, 0
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
