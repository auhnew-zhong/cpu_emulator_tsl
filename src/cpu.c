#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "opcodes.h"


#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[31m"
#define ANSI_RESET   "\x1b[0m"

//=====================================================================================
//   Signal table
//=====================================================================================

// 信号表结构体
struct signal_entry {
    uint32_t addr;
    uint32_t value;
};

// 示例信号表，可以根据实际需求扩展
struct signal_entry signal_table[] = {
    {0x00000004, 0x1001cccc},
    {0x00001000, 0x10001111},
    {0x00001008, 0x000060ff}, // 0x1001cccc
    {0x00001024, 0x000011f1},
    {0x00002048, 0x000022a2},
    {0x00004096, 0x000033b3},
    {0x00004104, 0x10011ccc},
    {0x00008192, 0x000044c4},
};

// 信号表大小
const int signal_table_size = sizeof(signal_table) / sizeof(signal_table[0]);

// 根据addr查找信号值，找不到返回0
uint32_t get_signal_value(uint32_t addr) {
    for (int i = 0; i < signal_table_size; ++i) {
        if (signal_table[i].addr == addr)
            return signal_table[i].value;
    }
    fprintf(stderr, "[-] ERROR-> No signal found for address:0x%x\n", addr);
    assert(0);
}

// print operation for DEBUG
void print_op(char* s) {
    printf("%s%s%s", ANSI_BLUE, s, ANSI_RESET);
}

//=====================================================================================
//   Display info table
//=====================================================================================

// display信息表结构体
struct display_info_entry {
    uint32_t id;
    char* format;
    // 由于变量参数是动态的，这里不包含实际值，只在运行时解析
};

// 读取display_info.db文件并解析为display_info表
struct display_info_entry* display_info_table = NULL;
int display_info_table_size = 0;

// 初始化display信息表
void init_display_info_table() {
    FILE* file = fopen("tests/display_info.db", "r");
    if (!file) {
        fprintf(stderr, "[-] ERROR-> Failed to open display_info.db\n");
        return;
    }

    // 先计算行数，为表分配内存
    int lines = 0;
    char ch;
    while(!feof(file)) {
        ch = fgetc(file);
        if(ch == '\n') {
            lines++;
        }
    }
    rewind(file);

    // 分配内存
    display_info_table = (struct display_info_entry*)malloc(lines * sizeof(struct display_info_entry));
    if (!display_info_table) {
        fprintf(stderr, "[-] ERROR-> Failed to allocate memory for display_info_table\n");
        fclose(file);
        return;
    }

    // 读取每一行并解析
    char line[512];
    int i = 0;
    while (fgets(line, sizeof(line), file) && i < lines) {
        // 检查行是否有效
        if (line[0] != '{') continue;
        
        // 提取ID
        uint32_t id;
        if (sscanf(line, "{0x%x,", &id) != 1) continue;

        // 提取格式字符串
        char* format_start = strchr(line, '"');
        if (!format_start) continue;
        
        char* format_end = strchr(format_start + 1, '"');
        if (!format_end) continue;
        
        int format_len = format_end - format_start - 1;
        char* format = (char*)malloc(format_len + 1);
        if (!format) continue;
        
        strncpy(format, format_start + 1, format_len);
        format[format_len] = '\0';
        
        // 保存到表中
        display_info_table[i].id = id;
        display_info_table[i].format = format;
        i++;
    }
    
    display_info_table_size = i;
    fclose(file);
    printf("[+] Loaded %d display info entries\n", display_info_table_size);
}

// 释放display信息表
void free_display_info_table() {
    if (display_info_table) {
        for (int i = 0; i < display_info_table_size; i++) {
            if (display_info_table[i].format) {
                free(display_info_table[i].format);
            }
        }
        free(display_info_table);
        display_info_table = NULL;
        display_info_table_size = 0;
    }
}

// 根据ID获取display信息格式字符串
char* get_display_format(uint32_t id) {
    for (int i = 0; i < display_info_table_size; i++) {
        if (display_info_table[i].id == id) {
            return display_info_table[i].format;
        }
    }
    return NULL;
}

// 从display_info表条目中提取完整字符串，包括格式和变量
char* get_complete_display_string(uint32_t id) {
    static char result[1024];
    result[0] = '\0';
    
    // 读取文件并查找指定ID的行
    FILE* file = fopen("tests/display_info.db", "r");
    if (!file) {
        return NULL;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        uint32_t line_id;
        if (sscanf(line, "{0x%x,", &line_id) == 1 && line_id == id) {
            // 找到匹配的ID
            fclose(file);
            
            // 移除行末尾的逗号和换行符
            char* end = strrchr(line, '}');
            if (end) *end = '\0';
            
            // 移除ID部分，只保留格式和变量
            char* content_start = strchr(line, ',');
            if (content_start) {
                // 跳过逗号和空格
                content_start++;
                while (*content_start == ' ') content_start++;
                
                // 复制剩余部分
                strcpy(result, content_start);
                return result;
            }
            
            return NULL;
        }
    }
    
    fclose(file);
    return NULL;
}

//=====================================================================================
//   CPU Initialization
//=====================================================================================

void cpu_init(CPU *cpu) {
    cpu->regs[0] = 0x00;                    // register R0 hardwired to 0
    cpu->regs[15] = 0;                      // R15 返回地址寄存器
    cpu->pc      = DRAM_BASE;               // Set program counter to the base address
    
    // 初始化display信息表
    init_display_info_table();
}

// 获取指令长度，根据操作码返回对应的字节数
// 1字节: TRIGGER, RET, NOP
// 2字节: TRIGGER_POS, JMP, BL, DISPLAY
// 4字节: JMPC(包含边沿检测), BIT_OP, LOAD, BIT_SLICE
// 8字节: MOV
// 其他: 返回0，表示未知或不支持
uint8_t get_inst_size(uint8_t opcode) {
    if (opcode == trigger || opcode == ret || opcode == nop)
        return 1;
    else if (opcode == trigger_pos || opcode == jmp || opcode == bl || opcode == display)
        return 2;
    else if (opcode == jmpc || opcode == bit_op || opcode == load || opcode == bit_slice)
        return 4;
    else if (opcode == mov)
        return 8;
    else {
        assert(0);
        return 0;
    }
}

uint8_t getInstLength(CPU *cpu) {
    uint8_t opcode_byte = bus_load(&(cpu->bus), cpu->pc, 8);
    return get_inst_size(opcode_byte >> 4 & 0xF);
}

//=====================================================================================
// Instruction Fetch
//=====================================================================================

uint64_t cpu_fetch(CPU *cpu, uint8_t *inst_length) {
    // 新增：检查指针有效性，避免解引用空指针
    if (inst_length == NULL) {
        fprintf(stderr, "[-] ERROR: cpu_fetch called with NULL inst_length pointer!\n");
        assert(0);
        return 0;
    }
    *inst_length = getInstLength(cpu);
    return bus_load(&(cpu->bus), cpu->pc, *inst_length * 8);
}

//=====================================================================================
// Assess Memory
//=====================================================================================

uint64_t cpu_load(CPU* cpu, uint64_t addr, uint64_t size) {
    return bus_load(&(cpu->bus), addr, size);
}

void cpu_store(CPU* cpu, uint64_t addr, uint64_t size, uint64_t value) {
    bus_store(&(cpu->bus), addr, size, value);
}

//=====================================================================================
// Instruction Decoder Functions
//=====================================================================================

uint64_t rd(uint32_t inst) {
    return 0;
}

uint64_t rs1(uint32_t inst) {
    return 0;
}

uint64_t rs2(uint32_t inst) {
    return 0;
}

//=====================================================================================
//   8BYTE Instruction Execution Functions
//=====================================================================================

void exec_MOV(CPU* cpu, uint64_t inst) {
    uint8_t dst_reg = (inst >> 56) & 0xF;
    uint32_t imm = (inst >> 24) & 0xFFFFFFFF;
    printf("%smov r%u %u%s\n", ANSI_BLUE, dst_reg, imm, ANSI_RESET);
    // 实际MOV操作可在此实现
    cpu->regs[dst_reg] = imm;
}

int decode_eight_byte_inst(CPU* cpu, uint64_t inst) {
    uint64_t inst_64 = inst;
    uint8_t opcode = (inst_64 >> 60) & 0xF;
    switch (opcode) {
        case mov: {  // MOV立即数
            exec_MOV(cpu, inst_64);
            break;
        }
        default: {
            fprintf(stderr, "[-] ERROR-> 8-byte opcode:0x%x\n", opcode);
            assert(0);
            return 0;
        }
    }
    return 1;
}

//=====================================================================================
//   4BYTE Instruction Execution Functions
//=====================================================================================

void exec_JMPC(CPU* cpu, uint32_t inst) {
    uint32_t func = (inst >> 24) & 0xF;
    uint32_t src1_reg = (inst >> 20) & 0xF;
    uint32_t src2_reg = (inst >> 16) & 0xF;
    uint32_t addr = (inst >> 8) & 0xFF;

    // 打印
    const char* func_symbols[] = {"==", "!=", ">", "<", ">=", "<="};
    if (func <= 0x5) {
        printf("%sjmpc r%u %s r%u addr=%u%s\n", ANSI_BLUE, src1_reg, func_symbols[func], src2_reg, addr, ANSI_RESET);
    } else {
        if (func == 0x6) {
            printf("%sjmpc r%u == 'bP addr=%u%s\n", ANSI_BLUE, src1_reg, addr, ANSI_RESET);
        } else {
            printf("%sjmpc r%u == 'bN addr=%u%s\n", ANSI_BLUE, src1_reg, addr, ANSI_RESET);
        }
    }
    
    // 获取源操作数值
    uint32_t src1 = cpu->regs[src1_reg];
    uint32_t src2 = cpu->regs[src2_reg];
    
    // 使用查找表优化条件判断
    bool should_jump = false;
    
    // 根据func值确定比较结果
    switch (func) {
        case 0x0: should_jump = (src1 == src2); break;
        case 0x1: should_jump = (src1 != src2); break;
        case 0x2: should_jump = (src1 > src2); break;
        case 0x3: should_jump = (src1 < src2); break;
        case 0x4: should_jump = (src1 >= src2); break;
        case 0x5: should_jump = (src1 <= src2); break;
        case 0x6: should_jump = true; break;
        case 0x7: should_jump = true; break;
        default:
            fprintf(stderr, "[-] ERROR-> exec_JMPC error!\n");
            assert(0);
            return;
    }
    
    // 如果条件满足，执行跳转
    if (should_jump) {
        cpu->pc += addr;
    }
}

void exec_BIT_OP(CPU* cpu, uint32_t inst) {
    uint8_t func = (inst >> 26) & 0x3;
    uint8_t dst_reg = (inst >> 22) & 0xF;
    uint8_t src1_reg = (inst >> 18) & 0xF;
    uint8_t src2_reg = (inst >> 14) & 0xF;

    // 打印
    const char* func_symbols[] = {"&", "|", "^"};
    if (func <= 0x2) {
        printf("%sbit_op r%u = r%u %s r%u%s\n", ANSI_BLUE, dst_reg, src1_reg, func_symbols[func], src2_reg, ANSI_RESET);
    }

    // 实际BIT_OP操作可在此实现
    uint32_t src1 = cpu->regs[src1_reg];
    uint32_t src2 = cpu->regs[src2_reg];
    switch (func) {
        case 0x0 :
            cpu->regs[dst_reg] = src1 & src2;
            break;
        case 0x1 :
            cpu->regs[dst_reg] = src1 | src2;
            break;
        case 0x2 :
            cpu->regs[dst_reg] = src1 ^ src2;
            break;
        default:
            fprintf(stderr, "[-] ERROR-> exec_BIT_OP error!\n");
            assert(0);
            break;
    }
}

void exec_BIT_SLICE(CPU* cpu, uint32_t inst) {
    uint8_t Dst = (inst >> 24) & 0xF;      // [27-24]
    uint8_t Src = (inst >> 20) & 0xF;      // [23-20]
    uint8_t Start = (inst >> 15) & 0x1F;   // [19-15]
    uint8_t End = (inst >> 10) & 0x1F;     // [14-10]
    printf("%sbit_slice r%u r%u[%u:%u]%s\n", ANSI_BLUE, Dst, Src, Start, End, ANSI_RESET);
    
    // 实际BIT_SLICE操作
    uint32_t src1 = cpu->regs[Src];
    
    // 确保Start <= End
    if (Start > End) {
        uint8_t temp = Start;
        Start = End;
        End = temp;
    }

    // 计算掩码: 创建一个长度为(End-Start+1)的全1位掩码
    uint32_t mask = ((1U << (End - Start + 1)) - 1);
    
    // 右移提取指定位段，然后通过掩码保留需要的位
    uint32_t res = (src1 >> Start) & mask;
    
    // 存储结果到目标寄存器
    cpu->regs[Dst] = res;
}

void exec_LOAD(CPU* cpu, uint32_t inst) {
    uint32_t dst = (inst >> 24) & 0xF;
    uint32_t addr = inst & 0xFFFFFF;
    printf("%sload r%u %u%s\n", ANSI_BLUE, dst, addr, ANSI_RESET);
    // 实际LOAD操作可在此实现，获取信号变量值（可能拆分汇聚）
    cpu->regs[dst] = get_signal_value(addr);
    printf("Get signal var from addr[%u] = 0x%x\n", addr, cpu->regs[dst]);
}

int decode_four_byte_inst(CPU* cpu, uint64_t inst) {
    uint32_t inst_32 = inst & 0xFFFFFFFF;
    uint8_t opcode = (inst_32 >> 28) & 0xF;
    switch (opcode) {
        case 0x0: // JMPC
            exec_JMPC(cpu, inst_32);
            break;
        case 0x1: // BIT_OP
            exec_BIT_OP(cpu, inst_32);
            break;
        case 0x6: // BIT_SLICE
            exec_BIT_SLICE(cpu, inst_32);
            break;
        case 0xD: // LOAD
            exec_LOAD(cpu, inst_32);
            break;
        default:
            fprintf(stderr, "[-] ERROR-> 4-byte opcode:0x%x\n", opcode);
            assert(0);
            return 0;
    }
    return 1;
}

//=====================================================================================
//   2BYTE Instruction Execution Functions
//=====================================================================================

void exec_TRIGGER_POS(CPU* cpu, uint16_t inst) {
    // 立即数imm为[11:5]位
    uint8_t imm = (inst >> 5) & 0x7F;
    printf("%strigger_pos %u%s\n", ANSI_BLUE, imm, ANSI_RESET);
    // 实际TRIGGER_POS操作可在此实现
    printf("Trigger sample pos set %u!\n", imm);
}

void exec_BL(CPU* cpu, uint16_t inst) {
    // offset为[11:2]，10位有符号
    int16_t offset = (inst >> 2) & 0x3FF;
    if (offset & 0x200) offset = -(1024 - offset);
    printf("%sbl %d%s\n", ANSI_BLUE, offset, ANSI_RESET);
    // 实际BL操作可在此实现
    cpu->regs[15] = cpu->pc;  // 保存返回地址到R15
    cpu->pc += offset;
}

void exec_JMP(CPU* cpu, uint16_t inst) {
    // offset为[11:4]，8位有符号
    int16_t offset = (inst >> 4) & 0xFF;
    if (offset & 0x80) offset = -(256 - offset);
    printf("%sjmp %d%s\n", ANSI_BLUE, offset, ANSI_RESET);
    // 实际JMP操作可在此实现
    cpu->pc += offset;
}

void exec_DISPLAY(CPU* cpu, uint16_t inst) {
    // 根据指令格式 [4bit op][10bit id][2bit rsv]
    uint16_t id = (inst >> 2) & 0x3FF;
    printf("%sdisplay %u%s\n", ANSI_BLUE, id, ANSI_RESET);
    
    // 获取完整的display字符串（格式+变量）
    char* complete_string = get_complete_display_string(id);
    if (complete_string) {
        printf("display(%s)\n", complete_string);
    } else {
        printf("Display: No format string found for ID %u\n", id);
        assert(0);
    }
}

int decode_two_byte_inst(CPU* cpu, uint64_t inst) {
    uint16_t inst_16 = inst & 0xFFFF;
    uint8_t opcode = (inst_16 >> 12) & 0xF;
    switch (opcode) {
        case 0x04: // trigger_pos
            exec_TRIGGER_POS(cpu, inst_16);
            break;
        case 0x05: // jmp
            exec_JMP(cpu, inst_16);
            break;
        case 0x09: // bl
            exec_BL(cpu, inst_16);
            break;
        case 0x0B: // display
            exec_DISPLAY(cpu, inst_16);
            break;
        default: {
            fprintf(stderr, "[-] ERROR-> 2-byte opcode:0x%x\n", opcode);
            assert(0);
            return 0;
        }
    }
    return 1;
}

//=====================================================================================
//   1BYTE Instruction Execution Functions
//=====================================================================================

void exec_TRIGGER(CPU* cpu, uint8_t inst) {
    printf("%strigger%s\n", ANSI_BLUE, ANSI_RESET);
    // 实际TRIGGER操作可在此实现
    printf("Time stop! Start trigger signal sample!\n");
}

void exec_RET(CPU* cpu, uint8_t inst) {
    printf("%sret%s\n", ANSI_BLUE, ANSI_RESET);
    // 实际RET操作可在此实现
    cpu->pc = cpu->regs[15];
    cpu->regs[15] = 0;
}

void exec_NOP(CPU* cpu, uint8_t inst) {
    printf("%snop%s\n", ANSI_BLUE, ANSI_RESET);
}

int decode_one_byte_inst(CPU* cpu, uint64_t inst) {
    uint8_t inst_8 = inst & 0xFF;
    uint8_t opcode = (inst_8 >> 4) & 0xF;
    switch (opcode) {
        case 0x03: // trigger
            exec_TRIGGER(cpu, inst_8);
            break;
        case 0x08: // ret
            exec_RET(cpu, inst_8);
            break;
        case 0x0A: // nop
            exec_NOP(cpu, inst_8);
            break;
        default: {
            fprintf(stderr, "[-] ERROR-> 1-byte opcode:0x%x\n", opcode);
            assert(0);
            return 0;
        }
    }
    return 1;
}

//=====================================================================================
//   Dump Register Info
//=====================================================================================

void dump_registers(CPU *cpu) {
    char* abi[] = { // Application Binary Interface registers
        "R0", "R1",  "R2",  "R3",
        "R4", "R5",  "R6",  "R7",
        "R8", "R9",  "R10",  "R11",
        "R12", "R13",  "R14",  "R15",
    };

    int N = 16;
    for (int i = 0; i < N; i++) {
        printf("   %4s: %#-13.2x  ", abi[i], cpu->regs[i]);
        if (i % 4 == 3)
            printf("\n");
    }
}

//=====================================================================================
//   Cpu Execution root function
//=====================================================================================

int cpu_execute(CPU *cpu, uint64_t inst, uint8_t inst_length) {
    // 打印当前指令地址
    printf("%s\n%#.8x -> %s", ANSI_YELLOW, cpu->pc, ANSI_RESET); // DEBUG

    cpu->pc += inst_length; // update pc for next cpu cycle

    if (inst_length == 1) {
        return decode_one_byte_inst(cpu, inst);
    } else if (inst_length == 2) {
        return decode_two_byte_inst(cpu, inst);
    } else if (inst_length == 4) {
        return decode_four_byte_inst(cpu, inst);
    } else if (inst_length == 8) {
        return decode_eight_byte_inst(cpu, inst);
    } else {
        fprintf(stderr, "[-] ERROR-> inst_length:0x%x\n", inst_length);
        assert(0);
        return 0;
    }
    return 1;  // 明确返回执行状态（1表示正常，0表示异常）
}

// 在程序结束时释放资源
void cpu_cleanup(CPU *cpu) {
    // 无需释放资源
}
