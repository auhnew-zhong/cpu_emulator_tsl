#include <stdio.h>
#include "../includes/cpu.h"
#include "../includes/opcodes.h"

#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[31m"
#define ANSI_RESET   "\x1b[0m"

#define ADDR_MISALIGNED(addr) (addr & 0x3)


// print operation for DEBUG
void print_op(char* s) {
    printf("%s%s%s", ANSI_BLUE, s, ANSI_RESET);
}

void cpu_init(CPU *cpu) {
    cpu->regs[0] = 0x00;                    // register R0 hardwired to 0
    cpu->regs[15] = 0;                      // R15 返回地址寄存器
    cpu->pc      = DRAM_BASE;               // Set program counter to the base address
}

uint8_t determine_instruction_length(uint8_t first_byte, uint8_t second_byte) {
    // 从高字节（第二个字节）获取操作码（高4位）
    uint8_t opcode = (second_byte >> 4) & 0x0F;
    if (opcode == trigger_pos || opcode == jmp || opcode == jmpc || opcode == bl) {
        return 2;
    }

    // 检查低字节（第一个字节）的操作码（高4位）
    opcode = (first_byte >> 4) & 0x0F;
    if (opcode == trigger || opcode == ret || opcode == nop) {
        return 1;
    }

    return 1;  // 默认1字节
}

uint8_t getInstLength(CPU *cpu) {
    uint16_t opcode_byte = bus_load(&(cpu->bus), cpu->pc, 16);
    return determine_instruction_length(opcode_byte & 0xFF, opcode_byte >> 8 & 0xFF);
}

uint64_t cpu_fetch(CPU *cpu, uint8_t *inst_length) {
    // 新增：检查指针有效性，避免解引用空指针
    if (inst_length == NULL) {
        fprintf(stderr, "[-] ERROR: cpu_fetch called with NULL inst_length pointer!\n");
        return 0;
    }
    *inst_length = getInstLength(cpu);
    return bus_load(&(cpu->bus), cpu->pc, *inst_length * 8);
}

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
    return (inst >> 7) & 0x1f;    // rd in bits 11..7
}

uint64_t rs1(uint32_t inst) {
    return (inst >> 15) & 0x1f;   // rs1 in bits 19..15
}

uint64_t rs2(uint32_t inst) {
    return (inst >> 20) & 0x1f;   // rs2 in bits 24..20
}

//=====================================================================================
//   Instruction Execution Functions
//=====================================================================================

void exec_TRIGGER(CPU* cpu, uint8_t inst) {
    print_op("trigger_pos");
}

void exec_RET(CPU* cpu, uint8_t inst) {
    cpu->pc = cpu->regs[15];
    cpu->regs[15] = 0;
    print_op("ret\n");
}

void exec_NOP(CPU* cpu, uint8_t inst) {
    print_op("nop\n");
}

void exec_TRIGGER_POS(CPU* cpu, uint16_t inst) {
    uint8_t imm = (inst >> 5) & 0x7F;
    printf("%s%s%d%s\n", ANSI_BLUE, "trigger_pos ", imm ,ANSI_RESET);
}

void exec_BL(CPU* cpu, uint16_t inst, uint8_t inst_length) {
    int16_t offset = (inst >> 2) & 0x3FF;
    cpu->regs[15] = cpu->pc;  // 保存返回地址到x15
    cpu->pc += offset;  // 示例：更新PC（需根据实际指令语义调整）
    printf("%s%s%d%s\n", ANSI_BLUE, "bl ", offset ,ANSI_RESET);
}

void exec_JMPC(CPU* cpu, uint16_t inst) {
    int16_t offset = (inst >> 4) & 0xFF;
    if (offset & 0x80) offset -= 0x100;  // 符号扩展
    uint8_t cond = (inst >> 3) & 0x01;
    if (cond) {  // 根据条件标志决定是否跳转
        cpu->pc += offset;
    }
    printf("%s%s%d, %d%s\n", ANSI_BLUE, "jmpc ", offset, cond, ANSI_RESET);
}

void exec_JMP(CPU* cpu, uint16_t inst) {
    int16_t offset = (inst >> 4) & 0xFF;
    if (offset & 0x80) offset -= 0x100;  // 符号扩展
    cpu->pc += offset;  // 示例：更新PC（需根据实际指令语义调整）
    printf("%s%s%d%s\n", ANSI_BLUE, "jmp ", offset, ANSI_RESET);
}

//=====================================================================================
//   Cpu Execution root function
//=====================================================================================
int cpu_execute(CPU *cpu, uint64_t inst, uint8_t inst_length) {

    // 打印当前指令地址
    printf("%s\n%#.8x -> %s", ANSI_YELLOW, cpu->pc, ANSI_RESET); // DEBUG

    cpu->pc += inst_length; // update pc for next cpu cycle
    cpu->regs[0] = 0; // R0 hardwired to 0 at each cycle

    if (inst_length == 1) {
        uint8_t inst_8 = inst & 0xFF;  // 小端序：低8位为单字节指令
        uint8_t opcode = (inst >> 4) & 0x0F;
        switch (opcode) {
            case trigger: {
                // TRIGGER指令：执行TRIGGER指令逻辑
                exec_TRIGGER(cpu, inst_8);
                break;
            } 
            case ret: {
                // RET指令：从R15中读取返回地址并跳转
                exec_RET(cpu, inst_8);
                break;
            }
            case nop: {
                // NOP指令：无操作，保持寄存器和PC不变
                exec_NOP(cpu, inst_8);
                break;
            }
            default: {
                fprintf(stderr, "[-] ERROR-> 1-byte opcode:0x%x\n", opcode);
                return 0;
            }
        }
    } else if (inst_length == 2) {
        uint16_t inst_16 = inst & 0xFFFF;  // 小端序：低16位为双字节指令
        uint8_t opcode = (inst_16 >> 12) & 0x0F;  // 高字节的高4位（小端序中高字节是inst_16的高8位）
        switch (opcode) {
            case trigger_pos: {  // TRIGGER_POS（立即数偏移）
                exec_TRIGGER_POS(cpu, inst_16);
                break;
            }
            case jmp: {  // JMP（符号扩展偏移）
                exec_JMP(cpu, inst_16);
                break;
            }
            case jmpc: {  // JMPC（符号扩展偏移+条件）
                exec_JMPC(cpu, inst_16);
                break;
            }
            case bl: {  // BL（符号扩展偏移）
                exec_BL(cpu, inst_16, inst_length);
                break;
            }
            default: {
                fprintf(stderr, "[-] ERROR-> 2-byte opcode:0x%x\n", opcode);
                return 0;
            }
        }
    } else if (inst_length == 4) {
        uint32_t inst_32 = inst & 0xFFFFFFFF;  // 小端序：低32位为四字节指令
        uint8_t opcode = (inst_32 >> 20) & 0x1F;  // 从32位指令中提取操作码（根据实际指令集调整位移）
        switch (opcode) {
            // TODO: 根据实际指令集补充具体操作码处理逻辑（如ADD/SUB等）
            default: {
                fprintf(stderr, "[-] ERROR-> 4-byte opcode:0x%x\n", opcode);
                return 0;
            }
        }
    } else if (inst_length == 8) {
        uint64_t inst_64 = inst;  // 小端序：8字节指令已完整存储在inst中
        uint8_t opcode = (inst_64 >> 32) & 0x1F;  // 从64位指令中提取操作码（根据实际指令集调整位移）
        switch (opcode) {
            // TODO: 根据实际指令集补充具体操作码处理逻辑（如LDB/STB等）
            default: {
                fprintf(stderr, "[-] ERROR-> 8-byte opcode:0x%x\n", opcode);
                return 0;
            }
        }
    }
    return 1;  // 明确返回执行状态（1表示正常，0表示异常）
}

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

