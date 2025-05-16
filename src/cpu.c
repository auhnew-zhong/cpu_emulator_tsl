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
    cpu->regs[2] = DRAM_BASE + DRAM_SIZE;   // Set stack pointer
    cpu->pc      = DRAM_BASE;               // Set program counter to the base address
}

uint32_t cpu_fetch(CPU *cpu) {
    uint32_t inst = bus_load(&(cpu->bus), cpu->pc, 32);
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
int cpu_execute(CPU *cpu, uint32_t inst) {
    int opcode = inst & 0x7f;           // opcode in bits 6..0
    int funct3 = (inst >> 12) & 0x7;    // funct3 in bits 14..12
    int funct7 = (inst >> 25) & 0x7f;   // funct7 in bits 31..25

    cpu->regs[0] = 0;                   // x0 hardwired to 0 at each cycle

    /*printf("%s\n%#.8x -> Inst: %#.8x <OpCode: %#.2x, funct3:%#x, funct7:%#x> %s",*/
            /*ANSI_YELLOW, cpu->pc-4, inst, opcode, funct3, funct7, ANSI_RESET); // DEBUG*/
    printf("%s\n%#.8x -> %s", ANSI_YELLOW, cpu->pc-4, ANSI_RESET); // DEBUG

    switch (opcode) {
        case R_TYPE:  
            switch (funct3) {
                case OR:   exec_OR(cpu, inst); break;
                case AND:  exec_AND(cpu, inst); break;
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct7:0x%x\n"
                            , opcode, funct3, funct7);
                    return 0;
            } break;

        case 0x00:
            return 0;

        default:
            fprintf(stderr, 
                    "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                    , opcode, funct3, funct7);
            return 0;
            /*exit(1);*/
    }
    return 1;
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

