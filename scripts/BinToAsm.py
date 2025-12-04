import sys

# 获取指令长度，根据操作码返回对应的字节数
# 1字节: TRIGGER, RET, TIMER_SET
# 2字节: TRIGGER_POS, JMP, BL, DISPLAY, EXEC, EDGE_DETECT, DOMAIN_SET, MOVI
# 4字节: JMPC, ARITH_OP, LOAD, BIT_SLICE
# 8字节: MOV
# 其他: 返回0，表示未知或不支持

def get_inst_size_from_bytes(data, pos):
    if pos >= len(data):
        return 0
    first_byte = data[pos]
    opcode = (first_byte >> 4) & 0xF
    if opcode in [0x03, 0x08, 0x0F]:
        return 1
    if opcode in [0x04, 0x05, 0x09, 0x0A, 0x0B, 0x0C, 0x0E]:
        return 2
    if opcode in [0x0, 0x1, 0x6, 0xD]:
        return 4
    if opcode == 0x7:
        if pos + 1 >= len(data):
            return 0
        first_two_bytes = (data[pos] << 8) | data[pos + 1]
        func_bit = (first_two_bytes >> 11) & 0x1
        return 2 if func_bit == 1 else 8
    return 0

def decode_timer_set(inst):
    if (inst & 0x1) != 0:
        return "UNKNOWN"
    func = (inst >> 1) & 0x3
    id = (inst >> 3) & 0x1
    return f"timer_set {id}, {func}"

# 解码1字节指令
# 只支持TRIGGER、RET、TIMER_SET，其他返回UNKNOWN

def decode_one_byte_inst(byte):
    opcode = (byte >> 4) & 0xF
    if opcode == 0x03:
        if (byte & 0xF) == 0:
            return "trigger"
        return "UNKNOWN"
    elif opcode == 0x08:
        if (byte & 0xF) == 0:
            return "ret"
        return "UNKNOWN"
    elif opcode == 0x0F:
        return decode_timer_set(byte)
    else:
        return "UNKNOWN"

# 解码TRIGGER_POS指令（2字节）
# 低5位必须为0，否则非法
# 立即数imm为[11:5]位

def decode_trigger_pos(inst):
    if (inst & 0x1F) != 0:
        return "UNKNOWN"
    imm = (inst >> 5) & 0x7F
    return f"trigger_pos {imm}"

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
    return f"jmp {offset}"

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
    return f"bl {offset}"

# 解码DISPLAY指令（2字节）
# 低2位必须为0，否则非法
# offset为[11:2]，10位有符号

def decode_display(inst):
    if (inst & 0x3) != 0:
        return "UNKNOWN"
    id_ = (inst >> 2) & 0x3FF
    return f"display\t{id_}"

# 解码DOMAIN_SET指令（2字节）
# 低1位必须为0，否则非法
# offset为[11:4]，8位有符号

def decode_domain_set(inst):
    if (inst & 0x1) != 0:
        return "UNKNOWN"
    offset = (inst >> 4) & 0xFF
    return f"domain_set {offset}"

# 解码EDGE_DETECT指令（2字节）
# dst: [11-8], src: [7-4], func: [3-1], polarity: [0]

def decode_edge_detect(inst):
    dst = (inst >> 8) & 0xF
    src = (inst >> 4) & 0xF
    func = (inst >> 1) & 0x7
    return f"edge_detect %r{dst}, %r{src}, {func}"

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
    elif opcode == 0x0C:
        return decode_exec(inst)
    elif opcode == 0x07:
        return decode_mov(inst)
    else:
        return "UNKNOWN"

# 解码JMPC指令（4字节）
# func: [27-24], src1: [23-20], src2: [19-16], addr: [15-8]

def decode_jmpc(inst):
    func = (inst >> 24) & 0xF
    src1 = (inst >> 20) & 0xF
    src2 = (inst >> 16) & 0xF
    addr = (inst >> 8) & 0xFF
    if addr & 0x80:
        addr = -(256 - addr)
    return f"jmpc {func}, %r{src1}, %r{src2}, {addr}"

# 解码BIT_OP指令（4字节）
# func: [27-26], dst: [25-22], src1: [21-18], src2: [17-14]

def decode_arith_op(inst):
    func = (inst >> 24) & 0xF
    dst = (inst >> 20) & 0xF
    src1 = (inst >> 16) & 0xF
    src2 = (inst >> 12) & 0xF
    return f"arith_op %r{dst}, %r{src1}, %r{src2}, {func}"

# 解码LOAD指令（4字节）
# dst: [27-24], addr: [23-0]

def decode_load(inst):
    dst = (inst >> 24) & 0xF
    addr = inst & 0xFFFFFF
    return f"load %r{dst}, {addr}"

# 解码BIT_SLICE指令（4字节）
# dst: [27-24], src: [23-20], start: [19-15], end: [14-10]

def decode_bit_slice(inst):
    dst = (inst >> 24) & 0xF
    src = (inst >> 20) & 0xF
    start = (inst >> 15) & 0x1F
    end = (inst >> 10) & 0x1F
    return f"bit_slice %r{dst}, %r{src}, {end}, {start}"

# 解码4字节指令，根据高4位opcode分发

def decode_four_byte_inst(bytes_):
    inst = (bytes_[0] << 24) | (bytes_[1] << 16) | (bytes_[2] << 8) | bytes_[3]  # 大端序拼接
    opcode = (bytes_[0] >> 4) & 0xF
    if opcode == 0x0:
        return decode_jmpc(inst)
    elif opcode == 0x1:
        return decode_arith_op(inst)
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
        return f"mov %r{dst}, %r{src}"
    else:
        # MOVI: mov $dst, $imm - 64位指令
        # [4bit op][1bit func][4bit dest][32bit imm][23bit rsv]
        func_bit = (inst >> 59) & 0x1  # [59]
        if func_bit != 0:
            return "INVALID_MOVI"  # func位应该为0表示MOVI
        
        dst = (inst >> 55) & 0xF           # [58-55]
        imm = (inst >> 23) & 0xFFFFFFFF    # [54-23]
        return f"mov %r{dst}, {imm}"

def decode_exec(inst):
    if (inst & 0x3) != 0:
        return "UNKNOWN"
    id_ = (inst >> 2) & 0x3FF
    return f"exec {id_}"

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
        inst_size = get_inst_size_from_bytes(data, pos)
        # 如果指令长度未知或剩余字节不足，按未知处理，跳过1字节
        if inst_size == 0 or pos + inst_size > len(data):
            print(f"Offset: {pos:04x} | {data[pos]:02x} | UNKNOWN")
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
        print(f"{pos:x}: {hex_str}\t{asm}")
        pos += inst_size

# 程序入口，命令行参数为二进制文件名
if __name__ == "__main__":
    if len(sys.argv) > 1:
        analyze_binary(sys.argv[1])
    else:
        print("Usage: python BinToAsm.py <binary_file>")
