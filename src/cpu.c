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

    fprintf(stderr, "[-] ERROR-> opcode error!\n");

    return 1;  // 默认1字节
}

uint8_t getInstLength(CPU *cpu) {
    uint16_t opcode_byte = bus_load(&(cpu->bus), cpu->pc, 16);
    return determine_instruction_length(opcode_byte & 0xFF, opcode_byte >> 8 & 0xFF);
}

uint64_t cpu_fetch(CPU *cpu, uint8_t *inst_length) {
    *inst_length = getInstLength(cpu);
    uint64_t inst = bus_load(&(cpu->bus), cpu->pc, *inst_length * 8);
    return inst;
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

uint64_t imm_I(uint32_t inst) {
    // imm[11:0] = inst[31:20]
    return ((int64_t)(int32_t) (inst & 0xfff00000)) >> 20; // right shift as signed?
}

uint64_t imm_S(uint32_t inst) {
    // imm[11:5] = inst[31:25], imm[4:0] = inst[11:7]
    return ((int64_t)(int32_t)(inst & 0xfe000000) >> 20)
        | ((inst >> 7) & 0x1f); 
}

uint64_t imm_B(uint32_t inst) {
    // imm[12|10:5|4:1|11] = inst[31|30:25|11:8|7]
    return ((int64_t)(int32_t)(inst & 0x80000000) >> 19)
        | ((inst & 0x80) << 4) // imm[11]
        | ((inst >> 20) & 0x7e0) // imm[10:5]
        | ((inst >> 7) & 0x1e); // imm[4:1]
}

uint64_t imm_U(uint32_t inst) {
    // imm[31:12] = inst[31:12]
    return (int64_t)(int32_t)(inst & 0xfffff999);
}

uint64_t imm_J(uint32_t inst) {
    // imm[20|10:1|11|19:12] = inst[31|30:21|20|19:12]
    return (uint64_t)((int64_t)(int32_t)(inst & 0x80000000) >> 11)
        | (inst & 0xff000) // imm[19:12]
        | ((inst >> 9) & 0x800) // imm[11]
        | ((inst >> 20) & 0x7fe); // imm[10:1]
}

uint32_t shamt(uint32_t inst) {
    // shamt(shift amount) only required for immediate shift instructions
    // shamt[4:5] = imm[5:0]
    return (uint32_t) (imm_I(inst) & 0x1f); // TODO: 0x1f / 0x3f ?
}

uint64_t csr(uint32_t inst) {
    // csr[11:0] = inst[31:20]
    return ((inst & 0xfff00000) >> 20);
}

//=====================================================================================
//   Instruction Execution Functions
//=====================================================================================

void exec_OR(CPU* cpu, uint32_t inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] | cpu->regs[rs2(inst)];
    print_op("or\n");
}

void exec_AND(CPU* cpu, uint32_t inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] & cpu->regs[rs2(inst)];
    print_op("and\n");
}

//=====================================================================================
//   Cpu Execution root function
//=====================================================================================
int cpu_execute(CPU *cpu, uint64_t inst, uint8_t inst_length) {

    cpu->regs[0] = 0; // x0 hardwired to 0 at each cycle
    printf("%s\n%#.8x -> %s", ANSI_YELLOW, cpu->pc-4, ANSI_RESET); // DEBUG

    if (inst_length == 1) {
        uint16_t inst_8 = inst & 0xFF;  // 小端序：低8位为单字节指令
        uint8_t opcode = (inst >> 4) & 0x0F;
        switch (opcode) {
            case trigger:
                // TRIGGER指令逻辑（待补充）
                break;
            case ret:
                // RET指令逻辑（待补充）
                break;
            case nop:
                // NOP指令：无操作，保持寄存器和PC不变
                break;
            default: {
                fprintf(stderr, "[-] ERROR-> opcode:0x%x\n", opcode);
                break;
            }
        }
    } else if (inst_length == 2) {
        uint16_t inst_16 = inst & 0xFFFF;  // 小端序：低16位为双字节指令
        uint8_t opcode = (inst_16 >> 12) & 0x0F;  // 高字节的高4位（小端序中高字节是inst_16的高8位）
        switch (opcode) {
            case 0x04: {  // TRIGGER_POS
                uint8_t imm = (inst_16 >> 5) & 0x7F;
                // 执行TRIGGER_POS逻辑（待补充）
                break;
            }
            case 0x05: {  // JMP
                int16_t offset = (inst_16 >> 4) & 0xFF;
                if (offset & 0x80) offset -= 0x100;  // 符号扩展
                cpu->pc += offset * inst_length;  // 示例：更新PC（需根据实际指令语义调整）
                break;
            }
            case 0x06: {  // JMPC（符号扩展偏移+条件）
                int16_t offset = (inst >> 4) & 0xFF;
                if (offset & 0x80) offset -= 0x100;
                uint8_t cond = (inst >> 3) & 0x01;
                break;
            }
            case 0x09: {  // BL（符号扩展偏移）
                int16_t offset = (inst >> 2) & 0x3FF;
                if (offset & 0x200) offset -= 0x400;  // 符号扩展
                break;
            }
            default: {
                fprintf(stderr, "[-] ERROR-> opcode:0x%x\n", opcode);
                break;
            }
        }
    } else if (inst_length == 4) {
        uint32_t inst_32 = inst & 0xFFFFFFFF;  // 小端序：低32位为四字节指令
        uint8_t opcode = (inst_32 >> 20) & 0x1F;  // 从32位指令中提取操作码（根据实际指令集调整位移）
        switch (opcode) {
            // TODO: 根据实际指令集补充具体操作码处理逻辑（如ADD/SUB等）
            default: {
                fprintf(stderr, "[-] ERROR-> 4-byte opcode:0x%x\n", opcode);
                break;
            }
        }
    } else if (inst_length == 8) {
        uint64_t inst_64 = inst;  // 小端序：8字节指令已完整存储在inst中
        uint8_t opcode = (inst_64 >> 32) & 0x1F;  // 从64位指令中提取操作码（根据实际指令集调整位移）
        switch (opcode) {
            // TODO: 根据实际指令集补充具体操作码处理逻辑（如LDB/STB等）
            default: {
                fprintf(stderr, "[-] ERROR-> 8-byte opcode:0x%x\n", opcode);
                break;
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
        "R16", "R17",  "R18",  "R19",
        "R20", "R21",  "R22",  "R23"
    };

    /*for (int i=0; i<8; i++) {*/
        /*printf("%4s| x%02d: %#-8.2lx\t", abi[i],    i,    cpu->regs[i]);*/
        /*printf("%4s| x%02d: %#-8.2lx\t", abi[i+8],  i+8,  cpu->regs[i+8]);*/
        /*printf("%4s| x%02d: %#-8.2lx\t", abi[i+16], i+16, cpu->regs[i+16]);*/
    /*}*/

    for (int i=0; i<8; i++) {
        printf("   %4s: %#-13.2x  ", abi[i],    cpu->regs[i]);
        printf("   %2s: %#-13.2x  ", abi[i+8],  cpu->regs[i+8]);
        printf("   %2s: %#-13.2x  ", abi[i+16], cpu->regs[i+16]);
    }
}

