.file	"display_test"
.text
.globl	main                            # -- Begin function main
.p2align	2
.type	main,@function
main:                                   # @main
# %bb.0:                                # %entry
	trigger_pos	60
	mov %r0, 0x12345678
	display	1                            # 显示信号ID为1，应显示"top.op[5:5] : %s"
	display	2                            # 显示信号ID为2，应显示多个信号
	display	3                            # 显示信号ID为3，应显示"signal2[1:1] : %s"
	display	10                           # 显示信号ID为10，应显示未找到格式字符串
	trigger
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        # -- End function
	.section	".note.GNU-stack","",@progbits 