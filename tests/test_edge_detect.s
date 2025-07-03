.file	"edge_detect_test"
.text
.globl	main                            # -- Begin function main
.p2align	2
.type	main,@function
main:                                   # @main
# %bb.0:                                # %entry
	trigger_pos	60
	
	# 初始化寄存器值
	mov %r1, 0                          # 初始值为0
	mov %r2, 1                          # 初始值为1
	
	# 测试上升沿检测，正极性
	edge_detect %r3, %r1, 0, 1          # 应该返回0，因为没有变化
	mov %r1, 1                          # 改变值为1
	edge_detect %r3, %r1, 0, 1          # 应该返回1，检测到上升沿
	edge_detect %r3, %r1, 0, 1          # 应该返回0，因为没有变化
	
	# 测试下降沿检测，正极性
	edge_detect %r4, %r2, 1, 1          # 应该返回0，因为没有变化
	mov %r2, 0                          # 改变值为0
	edge_detect %r4, %r2, 1, 1          # 应该返回1，检测到下降沿
	edge_detect %r4, %r2, 1, 1          # 应该返回0，因为没有变化
	
	# 测试任意边沿检测
	mov %r5, 0                          # 初始值为0
	edge_detect %r6, %r5, 2, 0          # 应该返回0，因为没有变化
	mov %r5, 1                          # 改变值为1
	edge_detect %r6, %r5, 2, 0          # 应该返回1，检测到变化
	edge_detect %r6, %r5, 2, 0          # 应该返回0，因为没有变化
	mov %r5, 0                          # 改变值为0
	edge_detect %r6, %r5, 2, 0          # 应该返回1，检测到变化
	
	# 测试电平检测
	mov %r7, 0                          # 值为0
	edge_detect %r8, %r7, 3, 0          # 应该返回0，电平为0
	mov %r7, 1                          # 值为1
	edge_detect %r8, %r7, 3, 0          # 应该返回1，电平为1
	
	trigger
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        # -- End function
	.section	".note.GNU-stack","",@progbits 