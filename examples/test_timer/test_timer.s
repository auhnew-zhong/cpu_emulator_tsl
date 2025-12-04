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
	timer_set 0, 1
	display	5
	exec 1
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
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
