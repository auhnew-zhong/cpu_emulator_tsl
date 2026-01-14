# 导入必要的Python库
import sys  # 系统相关功能，用于命令行参数

# 获取指令长度，根据操作码返回对应的字节数
# 1字节: TRIGGER, RET, TIMER_SET
# 2字节: TRIGGER_POS, JMP, BL, DISPLAY, EXEC, EDGE_DETECT, DOMAIN_SET, MOVI
# 4字节: JMPC, ARITH_OP, LOAD, BIT_SLICE
# 8字节: MOV
# 其他: 返回0，表示未知或不支持

def get_inst_size_from_bytes(data, pos):
    """根据字节数据确定指令长度, 处理MOV指令的变长情况"""
    if pos >= len(data):
        return 0
    
    first_byte = data[pos]
    opcode = (first_byte >> 4) & 0xF
    
    if opcode in [0x03, 0x08]:
        return 1
    elif opcode == 0x0F:
        return 8
    elif opcode in [0x04, 0x05, 0x09, 0x0A, 0x0B, 0x0C, 0x0E]:
        return 2
    elif opcode in [0x0, 0x1, 0x6, 0xD]:
        return 4
    elif opcode == 0x7:  # MOV指令需要特殊处理
        # 检查是否是16位MOV指令（寄存器到寄存器）
        if pos + 1 < len(data):
            second_byte = data[pos + 1]
            first_two_bytes = (first_byte << 8) | second_byte
            func_bit = (first_two_bytes >> 11) & 0x1
            
            # 检查是否所有高位字节都为0（表示16位指令）
            is_short_inst = True
            if pos + 8 <= len(data):
                for i in range(2, 8):
                    if data[pos + i] != 0:
                        is_short_inst = False
                        break
            
            if is_short_inst and func_bit == 1:
                return 2  # MOV (寄存器到寄存器)
        return 8  # MOVI (立即数到寄存器)
    else:
        return 0

def bin_to_mem(infile, outfile):
    """
    将TSL二进制文件转换为内存格式,每行一条指令
    Args:
        infile: 输入的二进制文件名
        outfile: 输出的内存格式文件名
    """
    # 打开输入文件(二进制文件)
    with open(infile, 'rb') as binfile:
        # 读取整个文件内容
        binfile_content = binfile.read()
    
    # 打开输出文件(内存格式文件)
    with open(outfile, 'w') as datafile:
        pos = 0
        while pos < len(binfile_content):
            # 使用新的变长指令长度检测函数
            inst_length = get_inst_size_from_bytes(binfile_content, pos)
            
            # 如果指令长度未知或剩余字节不足，跳过
            if inst_length == 0 or pos + inst_length > len(binfile_content):
                # 跳过未知指令，移动1字节继续
                pos += 1
                continue
                
            # 获取完整指令，打印一条指令至一行，并以大端模式打印
            instruction = binfile_content[pos:pos+inst_length]
            # 大端序：高字节在前，低字节在后
            word_hex = ''.join(f'{b:02x}' for b in instruction)
            datafile.write(word_hex + '\n')
            pos += inst_length

# 主程序入口
if __name__ == '__main__':
    # 检查命令行参数数量
    if len(sys.argv) == 3:
        # 如果参数正确，调用转换函数
        bin_to_mem(sys.argv[1], sys.argv[2])
    else:
        # 如果参数不正确，打印使用方法
        print('Usage: python BinToMem.py <input_binary> <output_memory>')
