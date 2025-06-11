	.file	"trigger"
	.text
	.globl	main                            # -- Begin function main
	.p2align	2
	.type	main,@function
main:                                   # @main
# %bb.0:                                # %entry
	trigger_pos	50
	bl	@inst1
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        # -- End function
	.p2align	2                               # -- Begin function inst1
	.type	inst1,@function
inst1:                                  # @inst1
# %bb.0:                                # %entry
	mov %r0, 9
	load %r1, 4096
	bit_op %r0, %r1, %r0, 2
	mov %r1, 13
	jmpc 3, %r0, %r1, @.LBB1_3
# %bb.1:                                # %then
	mov %r0, -1431655766
	load %r1, 4100
	jmpc 0, %r1, %r0, @.LBB1_2
	jmp	@.LBB1_3
.LBB1_2:                                # %then3
	trigger
.LBB1_3:                                # %else
	ret
.Lfunc_end1:
	.size	inst1, .Lfunc_end1-inst1
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
