#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// 确定指令长度（1或2字节）
uint8_t determine_instruction_length(uint8_t first_byte, uint8_t second_byte) {
    if (second_byte == 0) {  // 模拟Python中second_byte为None的情况（用0表示无）
        return 1;
    }

    // 从高字节（第二个字节）获取操作码（高4位）
    uint8_t opcode = (second_byte >> 4) & 0x0F;
    if (opcode == 0x04 || opcode == 0x05 || opcode == 0x06 || opcode == 0x09) {
        return 2;
    }

    // 检查低字节（第一个字节）的操作码（高4位）
    opcode = (first_byte >> 4) & 0x0F;
    if (opcode == 0x03 || opcode == 0x08 || opcode == 0x0A) {
        return 1;
    }

    return 1;  // 默认1字节
}

// 解码指令（返回静态缓冲区字符串）
const char* decode_instruction(const uint8_t* instruction, uint8_t length) {
    static char buffer[64] = {0};  // 静态缓冲区存储结果

    if (length == 1) {
        uint8_t opcode = (instruction[0] >> 4) & 0x0F;
        switch (opcode) {
            case 0x03: strcpy(buffer, "TRIGGER"); break;
            case 0x08: strcpy(buffer, "RET"); break;
            case 0x0A: strcpy(buffer, "NOP"); break;
            default: strcpy(buffer, "UNKNOWN"); break;
        }
    } else {
        uint16_t inst = (instruction[1] << 8) | instruction[0];  // 小端序组合双字节
        uint8_t opcode = (instruction[1] >> 4) & 0x0F;

        switch (opcode) {
            case 0x04: {  // TRIGGER_POS
                uint8_t imm = (inst >> 5) & 0x7F;
                snprintf(buffer, sizeof(buffer), "TRIGGER_POS %d", imm);
                break;
            }
            case 0x05: {  // JMP（符号扩展偏移）
                int16_t offset = (inst >> 4) & 0xFF;
                if (offset & 0x80) offset -= 0x100;  // 符号扩展
                snprintf(buffer, sizeof(buffer), "JMP %d", offset);
                break;
            }
            case 0x06: {  // JMPC（符号扩展偏移+条件）
                int16_t offset = (inst >> 4) & 0xFF;
                if (offset & 0x80) offset -= 0x100;
                uint8_t cond = (inst >> 3) & 0x01;
                snprintf(buffer, sizeof(buffer), "JMPC %d, %d", offset, cond);
                break;
            }
            case 0x09: {  // BL（符号扩展偏移）
                int16_t offset = (inst >> 2) & 0x3FF;
                if (offset & 0x200) offset -= 0x400;  // 符号扩展
                snprintf(buffer, sizeof(buffer), "BL %d", offset);
                break;
            }
            default: strcpy(buffer, "UNKNOWN"); break;
        }
    }
    return buffer;
}

// 分析二进制文件
void analyze_binary(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Error opening file");
        return;
    }

    // 获取文件大小
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // 读取文件内容到缓冲区
    uint8_t* data = (uint8_t*)malloc(file_size);
    if (!data) {
        perror("Memory allocation failed");
        fclose(f);
        return;
    }
    fread(data, 1, file_size, f);
    fclose(f);

    // 遍历分析指令
    size_t pos = 0;
    while (pos < file_size) {
        uint8_t first_byte = data[pos];
        uint8_t second_byte = (pos + 1 < file_size) ? data[pos + 1] : 0;

        uint8_t inst_length = determine_instruction_length(first_byte, second_byte);
        if (pos + inst_length > file_size) break;

        const uint8_t* instruction = &data[pos];
        const char* inst_name = decode_instruction(instruction, inst_length);

        // 格式化输出（类似Python版本）
        printf("Offset: %04zx | ", pos);
        if (inst_length == 2) {
            printf("%02x %02x | %s\n", instruction[1], instruction[0], inst_name);
        } else {
            printf("%02x | %s\n", instruction[0], inst_name);
        }

        pos += inst_length;
    }

    free(data);
}

// 主函数处理命令行参数
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }
    analyze_binary(argv[1]);
    return 0;
}