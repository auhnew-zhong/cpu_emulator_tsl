#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// 获取指令长度，根据操作码返回对应的字节数
// 1字节: TRIGGER, RET, NOP
// 2字节: TRIGGER_POS, JMP, BL, DISPLAY
// 4字节: JMPC, BIT_OP, LOAD, BIT_SLICE
// 8字节: MOV
// 其他: 返回0，表示未知或不支持
int get_inst_size(uint8_t opcode) {
    if (opcode == 0x03 || opcode == 0x08 || opcode == 0x0A)
        return 1;
    else if (opcode == 0x04 || opcode == 0x05 || opcode == 0x09 || opcode == 0x0B)
        return 2;
    else if (opcode == 0x0 || opcode == 0x1 || opcode == 0x6 || opcode == 0xD)
        return 4;
    else if (opcode == 0x7)
        return 8;
    else
        return 0;
}

// 解码1字节指令
const char* decode_one_byte_inst(uint8_t byte, char* buf, size_t buflen) {
    if ((byte & 0xF) != 0)
        return "UNKNOWN";
    uint8_t opcode = (byte >> 4) & 0xF;
    switch (opcode) {
        case 0x03: return "TRIGGER";
        case 0x08: return "RET";
        case 0x0A: return "NOP";
        default:   return "UNKNOWN";
    }
}

// 解码TRIGGER_POS指令（2字节）
const char* decode_trigger_pos(uint16_t inst, char* buf, size_t buflen) {
    if ((inst & 0x1F) != 0)
        return "UNKNOWN";
    uint8_t imm = (inst >> 5) & 0x7F;
    snprintf(buf, buflen, "TRIGGER_POS %u", imm);
    return buf;
}

// 解码JMP指令（2字节）
const char* decode_jmp(uint16_t inst, char* buf, size_t buflen) {
    if ((inst & 0xF) != 0)
        return "UNKNOWN";
    int8_t offset = (inst >> 4) & 0xFF;
    // 处理有符号数
    if (offset & 0x80)
        offset = -(256 - offset);
    snprintf(buf, buflen, "JMP %d", offset);
    return buf;
}

// 解码BL指令（2字节）
const char* decode_bl(uint16_t inst, char* buf, size_t buflen) {
    if ((inst & 0x3) != 0)
        return "UNKNOWN";
    int16_t offset = (inst >> 2) & 0x3FF;
    if (offset & 0x200)
        offset = -(1024 - offset);
    snprintf(buf, buflen, "BL %d", offset);
    return buf;
}

// 解码DISPLAY指令（2字节）
const char* decode_display(uint16_t inst, char* buf, size_t buflen) {
    if ((inst & 0x3) != 0)
        return "UNKNOWN";
    int16_t offset = (inst >> 2) & 0x3FF;
    if (offset & 0x200)
        offset = -(1024 - offset);
    snprintf(buf, buflen, "DISPLAY %d", offset);
    return buf;
}

// 解码2字节指令
const char* decode_two_byte_inst(const uint8_t* bytes, char* buf, size_t buflen) {
    uint16_t inst = (bytes[0] << 8) | bytes[1]; // 大端序
    uint8_t opcode = (bytes[0] >> 4) & 0xF;
    switch (opcode) {
        case 0x04: return decode_trigger_pos(inst, buf, buflen);
        case 0x05: return decode_jmp(inst, buf, buflen);
        case 0x09: return decode_bl(inst, buf, buflen);
        case 0x0B: return decode_display(inst, buf, buflen);
        default:   return "UNKNOWN";
    }
}

// 解码JMPC指令（4字节）
const char* decode_jmpc(uint32_t inst, char* buf, size_t buflen) {
    uint32_t func = (inst >> 24) & 0xF;
    uint32_t src1 = (inst >> 20) & 0xF;
    uint32_t src2 = (inst >> 16) & 0xF;
    uint32_t addr = (inst >> 8) & 0xFF;
    snprintf(buf, buflen, "JMPC func=%u r%u r%u addr=%u", func, src1, src2, addr);
    return buf;
}

// 解码BIT_OP指令（4字节）
const char* decode_bit_op(uint32_t inst, char* buf, size_t buflen) {
    uint32_t func = (inst >> 26) & 0x3;
    uint32_t dst = (inst >> 22) & 0xF;
    uint32_t src1 = (inst >> 18) & 0xF;
    uint32_t src2 = (inst >> 14) & 0xF;
    snprintf(buf, buflen, "BIT_OP r%u r%u r%u func=%u", dst, src1, src2, func);
    return buf;
}

// 解码LOAD指令（4字节）
const char* decode_load(uint32_t inst, char* buf, size_t buflen) {
    uint32_t dst = (inst >> 24) & 0xF;
    uint32_t addr = inst & 0xFFFFFF;
    snprintf(buf, buflen, "LOAD r%u %u", dst, addr);
    return buf;
}

// 解码BIT_SLICE指令（4字节）
const char* decode_bit_slice(uint32_t inst, char* buf, size_t buflen) {
    uint32_t dst = (inst >> 24) & 0xF;
    uint32_t src = (inst >> 20) & 0xF;
    uint32_t start = (inst >> 15) & 0x1F;
    uint32_t end = (inst >> 10) & 0x1F;
    snprintf(buf, buflen, "BIT_SLICE r%u r%u %u %u", dst, src, start, end);
    return buf;
}

// 解码4字节指令
const char* decode_four_byte_inst(const uint8_t* bytes, char* buf, size_t buflen) {
    uint32_t inst = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    uint8_t opcode = (bytes[0] >> 4) & 0xF;
    switch (opcode) {
        case 0x0: return decode_jmpc(inst, buf, buflen);
        case 0x1: return decode_bit_op(inst, buf, buflen);
        case 0xD: return decode_load(inst, buf, buflen);
        case 0x6: return decode_bit_slice(inst, buf, buflen);
        default:  return "UNKNOWN";
    }
}

// 解码MOV指令（8字节）
const char* decode_mov(uint64_t inst, char* buf, size_t buflen) {
    uint32_t dst = (inst >> 56) & 0xF;
    uint32_t imm = (inst >> 24) & 0xFFFFFFFF;
    snprintf(buf, buflen, "MOV r%u %u", dst, imm);
    return buf;
}

// 解码8字节指令
const char* decode_eight_byte_inst(const uint8_t* bytes, char* buf, size_t buflen) {
    uint64_t inst = 0;
    for (int i = 0; i < 8; ++i)
        inst = (inst << 8) | bytes[i];
    uint8_t opcode = (bytes[0] >> 4) & 0xF;
    if (opcode == 0x7)
        return decode_mov(inst, buf, buflen);
    else
        return "UNKNOWN";
}

// 主分析函数，逐条读取二进制文件并反汇编
void analyze_binary(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "无法打开文件: %s\n", filename);
        return;
    }
    // 读取文件到内存
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t* data = (uint8_t*)malloc(filesize);
    if (!data) {
        fprintf(stderr, "内存分配失败\n");
        fclose(f);
        return;
    }
    fread(data, 1, filesize, f);
    fclose(f);

    long pos = 0;
    char buf[64];
    while (pos < filesize) {
        uint8_t first_byte = data[pos];
        uint8_t opcode = (first_byte >> 4) & 0xF;
        int inst_size = get_inst_size(opcode);
        // 如果指令长度未知或剩余字节不足，按未知处理，跳过1字节
        if (inst_size == 0 || pos + inst_size > filesize) {
            printf("Offset: %04lx | %02x | UNKNOWN\n", pos, first_byte);
            pos += 1;
            continue;
        }
        const uint8_t* inst_bytes = data + pos;
        const char* asm_str = NULL;
        // 根据指令长度调用对应解码函数
        if (inst_size == 1) {
            asm_str = decode_one_byte_inst(inst_bytes[0], buf, sizeof(buf));
            printf("Offset: %04lx | %02x | %s\n", pos, inst_bytes[0], asm_str);
        } else if (inst_size == 2) {
            asm_str = decode_two_byte_inst(inst_bytes, buf, sizeof(buf));
            printf("Offset: %04lx | %02x %02x | %s\n", pos, inst_bytes[0], inst_bytes[1], asm_str);
        } else if (inst_size == 4) {
            asm_str = decode_four_byte_inst(inst_bytes, buf, sizeof(buf));
            printf("Offset: %04lx | %02x %02x %02x %02x | %s\n", pos, inst_bytes[0], inst_bytes[1], inst_bytes[2], inst_bytes[3], asm_str);
        } else if (inst_size == 8) {
            asm_str = decode_eight_byte_inst(inst_bytes, buf, sizeof(buf));
            printf("Offset: %04lx | %02x %02x %02x %02x %02x %02x %02x %02x | %s\n", pos,
                inst_bytes[0], inst_bytes[1], inst_bytes[2], inst_bytes[3],
                inst_bytes[4], inst_bytes[5], inst_bytes[6], inst_bytes[7], asm_str);
        }
        pos += inst_size;
    }
    free(data);
}

// 程序入口，命令行参数为二进制文件名
int main(int argc, char* argv[]) {
    if (argc > 1) {
        analyze_binary(argv[1]);
    } else {
        printf("Usage: %s <binary_file>\n", argv[0]);
    }
    return 0;
}