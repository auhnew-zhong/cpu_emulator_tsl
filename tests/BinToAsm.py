# 导入必要的Python库
import sys  # 系统相关功能，用于命令行参数
import os   # 操作系统接口，用于文件操作

def determine_instruction_length(first_byte, second_byte=None):
    """
    根据指令字节确定指令长度
    Args:
        first_byte: 第一个字节（小端序中的低字节）
        second_byte: 第二个字节（小端序中的高字节，包含操作码）
    Returns:
        instruction_length: 指令长度（1或2字节）
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

def decode_instruction(instruction):
    """
    解码指令
    Args:
        instruction: 完整的指令字节
    Returns:
        inst_name: 指令助记符
    """
    if len(instruction) == 1:
        # 单字节指令
        opcode = (instruction[0] >> 4) & 0xF
        if opcode == 0x03:
            return "TRIGGER"
        elif opcode == 0x08:
            return "RET"
        elif opcode == 0x0A:
            return "NOP"
    else:
        # 双字节指令（小端序）
        opcode = (instruction[1] >> 4) & 0xF  # 操作码在高字节的高4位
        inst = (instruction[1] << 8) | instruction[0]
        
        if opcode == 0x04:  # TRIGGER_POS
            imm = (inst >> 5) & 0x7F
            return f"TRIGGER_POS {imm}"
        elif opcode == 0x05:  # JMP
            offset = (inst >> 4) & 0xFF
            if offset & 0x80:
                offset = -(256 - offset)
            return f"JMP {offset}"
        elif opcode == 0x06:  # JMPC
            offset = (inst >> 4) & 0xFF
            if offset & 0x80:
                offset = -(256 - offset)
            cond = (inst >> 3) & 0x1
            return f"JMPC {offset}, {cond}"
        elif opcode == 0x09:  # BL
            offset = (inst >> 2) & 0x3FF
            if offset & 0x200:
                offset = -(1024 - offset)
            return f"BL {offset}"
    
    return f"UNKNOWN"

def analyze_binary(filename):
    """
    分析TSL二进制文件
    Args:
        filename: 二进制文件名
    """
    with open(filename, 'rb') as f:
        data = f.read()

    pos = 0
    while pos < len(data):
        # 总是尝试读取两个字节
        first_byte = data[pos]
        second_byte = data[pos + 1] if pos + 1 < len(data) else None

        # 打印调试信息
        # print(f"First byte: {first_byte:02x}")
        # print(f"Second byte: {second_byte:02x}")

        # 根据两个字节确定指令长度
        inst_length = determine_instruction_length(first_byte, second_byte)

        # 提取完整指令
        if pos + inst_length <= len(data):
            instruction = data[pos:pos+inst_length]

            # 解码指令
            inst_name = decode_instruction(instruction)

            # 打印指令信息
            print(f"Offset: {pos:04x} | ", end='')
            if inst_length == 2:
                print(f"{instruction[1]:02x} {instruction[0]:02x}", end='')
            else:
                print(f"{instruction[0]:02x}", end='')
            print(f" | {inst_name}")

            pos += inst_length
        else:
            break

# 主程序入口
if __name__ == "__main__":
    if len(sys.argv) > 1:
        analyze_binary(sys.argv[1])
    else:
        print("Usage: python BinToAsm.py <binary_file>")