	.file	"trigger"
	.text
	.globl	main                            # -- Begin function main
	.p2align	2
	.type	main,@function
main:                                   # @main
# %bb.0:                                # %entry
	trigger_pos	60
	bl	@inst1
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        # -- End function
	.p2align	2                               # -- Begin function inst1
	.type	inst1,@function
inst1:                                  # @inst1
# %bb.0:                                # %entry
	mov %r0, 13
	load %r1, 1024
	bit_op %r0, %r1, %r0, 0
	mov %r1, 9
	jcmp 3, %r0, %r1, @.LBB1_2
	jmp	@.LBB1_1
.LBB1_1:                                # %then
	trigger
.LBB1_2:                                # %merge
	ret
.Lfunc_end1:
	.size	inst1, .Lfunc_end1-inst1
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
