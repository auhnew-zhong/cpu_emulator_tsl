import sys

# 获取指令长度，根据操作码返回对应的字节数
# 1字节: TRIGGER, RET, TIMER_SET
# 2字节: TRIGGER_POS, JMP, BL, DISPLAY, EDGE_DETECT, DOMAIN_SET
# 4字节: JMPC, BIT_OP, LOAD, BIT_SLICE
# 8字节: MOV
# 其他: 返回0，表示未知或不支持

def get_inst_size_from_bytes(data, pos):
    """根据字节数据确定指令长度，处理MOV指令的变长情况"""
    if pos >= len(data):
        return 0
    
    first_byte = data[pos]
    opcode = (first_byte >> 4) & 0xF
    
    if opcode in [0x03, 0x08, 0x0F]:
        return 1
    elif opcode in [0x04, 0x05, 0x09, 0xA, 0x0B, 0x0E]:
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

def get_inst_size(opcode):
    """保持向后兼容的简单版本，用于不需要检查具体字节的场合"""
    if opcode in [0x03, 0x08, 0x0F]:
        return 1
    elif opcode in [0x04, 0x05, 0x09, 0xA, 0x0B, 0x0E]:
        return 2
    elif opcode in [0x0, 0x1, 0x6, 0xD]:
        return 4
    elif opcode == 0x7:
        return 8  # 默认返回8，实际使用时应该用get_inst_size_from_bytes
    else:
        return 0

def decode_timer_set(inst):
    if (inst & 0x1) != 0:
        return "UNKNOWN"
    id = (inst >> 1) & 0x1
    func = (inst >> 2) & 0x3
    return f"TIMER_SET {id} {func}"

# 解码1字节指令
# 只支持TRIGGER、RET、TIMER_SET，其他返回UNKNOWN

def decode_one_byte_inst(byte):
    if (byte & 0xF) != 0:
        return "UNKNOWN"
    opcode = (byte >> 4) & 0xF
    if opcode == 0x03:
        return "TRIGGER"
    elif opcode == 0x08:
        return "RET"
    elif opcode == 0x0F:
        return "TIMER_SET"
    else:
        return "UNKNOWN"

# 解码TRIGGER_POS指令（2字节）
# 低5位必须为0，否则非法
# 立即数imm为[11:5]位

def decode_trigger_pos(inst):
    if (inst & 0x1F) != 0:
        return "UNKNOWN"
    imm = (inst >> 5) & 0x7F
    return f"TRIGGER_POS {imm}"

# 解码JMP指令（2字节）
# 低4位必须为0，否则非法
# offset为[11:4]，8位有符号

def decode_jmp(inst):
    if (inst & 0xF) != 0:
        return "UNKNOWN"
    offset = (inst >> 4) & 0xFF
    # 处理有符号数
    if offset & 0x80:
        offset = -(256 - offset)
    return f"JMP {offset}"

# 解码BL指令（2字节）
# 低2位必须为0，否则非法
# offset为[11:2]，10位有符号

def decode_bl(inst):
    if (inst & 0x3) != 0:
        return "UNKNOWN"
    offset = (inst >> 2) & 0x3FF
    # 处理有符号数
    if offset & 0x200:
        offset = -(1024 - offset)
    return f"BL {offset}"

# 解码DISPLAY指令（2字节）
# 低2位必须为0，否则非法
# offset为[11:2]，10位有符号

def decode_display(inst):
    if (inst & 0x3) != 0:
        return "UNKNOWN"
    offset = (inst >> 2) & 0x3FF
    # 处理有符号数
    if offset & 0x200:
        offset = -(1024 - offset)
    return f"DISPLAY {offset}"

# 解码DOMAIN_SET指令（2字节）
# 低1位必须为0，否则非法
# offset为[11:4]，8位有符号

def decode_domain_set(inst):
    if (inst & 0x1) != 0:
        return "UNKNOWN"
    offset = (inst >> 4) & 0xFF
    return f"DOMAIN_SET {offset}"

# 解码EDGE_DETECT指令（2字节）
# dst: [11-8], src: [7-4], func: [3-1], polarity: [0]

def decode_edge_detect(inst):
    dst = (inst >> 8) & 0xF
    src = (inst >> 4) & 0xF
    func = (inst >> 1) & 0x3
    # 功能名称
    func_names = ["上升沿", "下降沿"]

    if func <= 1:
        return f"EDGE_DETECT r{dst} r{src} {func_names[func]}"
    else:
        return f"EDGE_DETECT r{dst} r{src} func={func}"

# 解码2字节指令，根据高4位opcode分发

def decode_two_byte_inst(bytes_):
    inst = (bytes_[0] << 8) | bytes_[1]  # 大端序拼接
    opcode = (bytes_[0] >> 4) & 0xF
    if opcode == 0x04:
        return decode_trigger_pos(inst)
    elif opcode == 0x05:
        return decode_jmp(inst)
    elif opcode == 0x09:
        return decode_bl(inst)
    elif opcode == 0x0B:
        return decode_display(inst)
    elif opcode == 0xA:
        return decode_domain_set(inst)
    elif opcode == 0x0E:
        return decode_edge_detect(inst)
    else:
        return "UNKNOWN"

# 解码JMPC指令（4字节）
# func: [27-24], src1: [23-20], src2: [19-16], addr: [15-8]

def decode_jmpc(inst):
    func = (inst >> 24) & 0xF
    src1 = (inst >> 20) & 0xF
    src2 = (inst >> 16) & 0xF
    addr = (inst >> 8) & 0xFF
    return f"JMPC func={func} r{src1} r{src2} addr={addr}"

# 解码BIT_OP指令（4字节）
# func: [27-26], dst: [25-22], src1: [21-18], src2: [17-14]

def decode_bit_op(inst):
    func = (inst >> 26) & 0x3
    dst = (inst >> 22) & 0xF
    src1 = (inst >> 18) & 0xF
    src2 = (inst >> 14) & 0xF
    return f"BIT_OP r{dst} r{src1} r{src2} func={func}"

# 解码LOAD指令（4字节）
# dst: [27-24], addr: [23-0]

def decode_load(inst):
    dst = (inst >> 24) & 0xF
    addr = inst & 0xFFFFFF
    return f"LOAD r{dst} {addr}"

# 解码BIT_SLICE指令（4字节）
# dst: [27-24], src: [23-20], start: [19-15], end: [14-10]

def decode_bit_slice(inst):
    dst = (inst >> 24) & 0xF
    src = (inst >> 20) & 0xF
    start = (inst >> 15) & 0x1F
    end = (inst >> 10) & 0x1F
    return f"BIT_SLICE r{dst} r{src} {start} {end}"

# 解码4字节指令，根据高4位opcode分发

def decode_four_byte_inst(bytes_):
    inst = (bytes_[0] << 24) | (bytes_[1] << 16) | (bytes_[2] << 8) | bytes_[3]  # 大端序拼接
    opcode = (bytes_[0] >> 4) & 0xF
    if opcode == 0x0:
        return decode_jmpc(inst)
    elif opcode == 0x1:
        return decode_bit_op(inst)
    elif opcode == 0x6:
        return decode_bit_slice(inst)
    elif opcode == 0xD:
        return decode_load(inst)
    else:
        return "UNKNOWN"

# 解码MOV指令，支持两种变体：
# MOVI（64位）: [4bit op][1bit func][4bit dest][32bit imm][23bit rsv], func=0
# MOV（16位）: [4bit op][1bit func][4bit dest][4bit src][3bit rsv], func=1

def decode_mov(inst):
    # 检查func位来区分MOVI和MOV指令
    # 首先检查是否是16位MOV指令（寄存器到寄存器）
    # 16位指令的高48位应该为0，且func位[11]=1
    if (inst >> 16) == 0 and ((inst >> 11) & 0x1) == 1:
        # MOV: mov $dst, $src - 16位指令
        # [4bit op][1bit func][4bit dest][4bit src][3bit rsv]
        dst = (inst >> 7) & 0xF      # [10-7]
        src = (inst >> 3) & 0xF      # [6-3]
        return f"MOV r{dst}, r{src}"
    else:
        # MOVI: mov $dst, $imm - 64位指令
        # [4bit op][1bit func][4bit dest][32bit imm][23bit rsv]
        func_bit = (inst >> 59) & 0x1  # [59]
        if func_bit != 0:
            return "INVALID_MOVI"  # func位应该为0表示MOVI
        
        dst = (inst >> 55) & 0xF           # [58-55]
        imm = (inst >> 23) & 0xFFFFFFFF    # [54-23]
        return f"MOV r{dst}, {imm}"

# 解码8字节指令，目前只支持MOV

def decode_eight_byte_inst(bytes_):
    inst = 0
    for i in range(8):
        inst = (inst << 8) | bytes_[i]  # 大端序拼接
    opcode = (bytes_[0] >> 4) & 0xF
    if opcode == 0x7:
        return decode_mov(inst)
    else:
        return "UNKNOWN"

# 主分析函数，逐条读取二进制文件并反汇编
# 输出格式：偏移 | 原始字节 | 助记符

def analyze_binary(filename):
    with open(filename, 'rb') as f:
        data = f.read()
    pos = 0
    while pos < len(data):
        first_byte = data[pos]
        opcode = (first_byte >> 4) & 0xF
        inst_size = get_inst_size(opcode)
        # 如果指令长度未知或剩余字节不足，按未知处理，跳过1字节
        if inst_size == 0 or pos + inst_size > len(data):
            print(f"Offset: {pos:04x} | {first_byte:02x} | UNKNOWN")
            pos += 1
            continue
        inst_bytes = data[pos:pos+inst_size]
        # 根据指令长度调用对应解码函数
        if inst_size == 1:
            asm = decode_one_byte_inst(inst_bytes[0])
            hex_str = f"{inst_bytes[0]:02x}"
        elif inst_size == 2:
            asm = decode_two_byte_inst(inst_bytes)
            hex_str = f"{inst_bytes[0]:02x} {inst_bytes[1]:02x}"
        elif inst_size == 4:
            asm = decode_four_byte_inst(inst_bytes)
            hex_str = " ".join(f"{b:02x}" for b in inst_bytes)
        elif inst_size == 8:
            asm = decode_eight_byte_inst(inst_bytes)
            hex_str = " ".join(f"{b:02x}" for b in inst_bytes)
        else:
            asm = "UNKNOWN"
            hex_str = " ".join(f"{b:02x}" for b in inst_bytes)
        print(f"Offset: {pos:04x} | {hex_str} | {asm}")
        pos += inst_size

# 程序入口，命令行参数为二进制文件名
if __name__ == "__main__":
    if len(sys.argv) > 1:
        analyze_binary(sys.argv[1])
    else:
        print("Usage: python BinToAsm.py <binary_file>")