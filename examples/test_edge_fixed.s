	.file	"trigger"
	.text
	.globl	main                            # -- Begin function main
	.p2align	2
	.type	main,@function
main:                                   # @main
# %bb.0:                                # %entry
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
	load %r0, 4104
	bit_slice %r1, %r0, 5, 5
	edge_detect %r1, %r1, 1
	mov %r2, 1
	jmpc 1, %r1, %r2, @.LBB1_2
# %bb.1:                                # %then
	display	1
	ret
.LBB1_2:                                # %else
	load %r1, 4096
	load %r2, 4
	bit_op %r2, %r2, %r1, 2
	bit_slice %r2, %r2, 3, 0
	mov %r3, 13
	jmpc 3, %r2, %r3, @.LBB1_8
# %bb.3:                                # %then1
	display	2
	edge_detect %r0, %r0, 0
	mov %r2, 1
	jmpc 0, %r0, %r2, @.LBB1_4
	jmp	@.LBB1_5
.LBB1_4:                                # %then4
	trigger
	ret
.LBB1_5:                                # %else5
	load %r0, 4100
	bit_op %r0, %r1, %r0, 0
	bit_slice %r0, %r0, 1, 1
	edge_detect %r0, %r0, 1
	mov %r1, 1
	jmpc 1, %r0, %r1, @.LBB1_7
# %bb.6:                                # %then7
	trigger
	ret
.LBB1_7:                                # %else8
	display	3
.LBB1_8:                                # %else2
	ret
.Lfunc_end1:
	.size	inst1, .Lfunc_end1-inst1
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
