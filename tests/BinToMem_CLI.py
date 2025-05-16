# 导入必要的Python库
import sys  # 系统相关功能，用于命令行参数
import os   # 操作系统接口，用于文件操作

def determine_instruction_length(first_byte, second_byte=None):
    """
    根据指令字节确定TSL指令长度
    Args:
        first_byte: 第一个字节（小端序中的低字节）
        second_byte: 第二个字节（小端序中的高字节，包含操作码）
    Returns:
        instruction_length: 指令长度(1或2字节)
    """
    if second_byte is None:
        return 1  # 如果没有第二个字节，只能是单字节指令
        
    # 从高字节（第二个字节）获取操作码
    opcode = (second_byte >> 4) & 0xF
    
    # 2字节指令：TRIGGER_POS(0x04), JMP(0x05), JMPC(0x06), BL(0x09)
    if opcode in [0x04, 0x05, 0x06, 0x09]:
        return 2
        
    # 否则是单字节指令：TRIGGER(0x03), RET(0x08), NOP(0x0A)
    # 检查第一个字节的操作码
    opcode = (first_byte >> 4) & 0xF
    if opcode in [0x03, 0x08, 0x0A]:
        return 1
        
    return 1  # 默认返回1

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
            # 读取两个字节来判断指令长度
            first_byte = binfile_content[pos]
            second_byte = binfile_content[pos + 1] if pos + 1 < len(binfile_content) else None
            
            # 确定指令长度
            inst_length = determine_instruction_length(first_byte, second_byte)
            
            # 获取完整指令
            if pos + inst_length <= len(binfile_content):
                instruction = binfile_content[pos:pos+inst_length]
                
                # 将指令填充到32位（4字节）
                padded_instruction = list(instruction)
                while len(padded_instruction) < 4:
                    padded_instruction.append(0)
                
                # 写入一行（一条指令）
                # 小端序：低字节在前，高字节在后
                word_hex = ''.join(f'{b:02x}' for b in padded_instruction)
                datafile.write(word_hex + '\n')
                
                pos += inst_length
            else:
                break

def print_debug_info(first_byte, second_byte, inst_length, instruction):
    """
    打印调试信息（可选）
    """
    print(f"First byte: {first_byte:02x}")
    if second_byte is not None:
        print(f"Second byte: {second_byte:02x}")
    print(f"Instruction length: {inst_length}")
    print(f"Instruction bytes: {' '.join(f'{b:02x}' for b in instruction)}")
    print(f"Padded instruction: {' '.join(f'{b:02x}' for b in instruction + [0] * (4 - len(instruction)))}")
    print("-" * 50)

# 主程序入口
if __name__ == '__main__':
    # 检查命令行参数数量
    if len(sys.argv) == 3:
        # 如果参数正确，调用转换函数
        bin_to_mem(sys.argv[1], sys.argv[2])
    else:
        # 如果参数不正确，打印使用方法
        print('Usage: python BinToMem_CLI.py <input_binary> <output_memory>')