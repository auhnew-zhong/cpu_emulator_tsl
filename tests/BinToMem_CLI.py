# 导入必要的Python库
import sys  # 系统相关功能，用于命令行参数

# 获取指令长度，根据操作码返回对应的字节数
# 1字节: TRIGGER, RET, NOP
# 2字节: TRIGGER_POS, JMP, BL
# 4字节: JMPC, BIT_OP, LOAD
# 8字节: MOV
# 其他: 返回0，表示未知或不支持

def get_inst_size(opcode):
    if opcode in [0x03, 0x08, 0x0A]:
        return 1
    elif opcode in [0x04, 0x05, 0x09]:
        return 2
    elif opcode in [0x0, 0x1, 0xD]:
        return 4
    elif opcode == 0x7:
        return 8
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
            # 读取一个字节来判断指令长度
            first_byte = binfile_content[pos]
            # 获取操作码
            opcode = (first_byte >> 4) & 0xF
            # 确定指令长度
            inst_length = get_inst_size(opcode)
            # 获取完整指令，打印一条指令至一行，并以大端模式打印
            if pos + inst_length <= len(binfile_content):
                instruction = binfile_content[pos:pos+inst_length]
                # 大端序：高字节在前，低字节在后
                word_hex = ''.join(f'{b:02x}' for b in instruction)
                datafile.write(word_hex + '\n')
                pos += inst_length
            else:
                break

# 主程序入口
if __name__ == '__main__':
    # 检查命令行参数数量
    if len(sys.argv) == 3:
        # 如果参数正确，调用转换函数
        bin_to_mem(sys.argv[1], sys.argv[2])
    else:
        # 如果参数不正确，打印使用方法
        print('Usage: python BinToMem_CLI.py <input_binary> <output_memory>')