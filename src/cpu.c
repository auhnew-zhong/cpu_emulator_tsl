#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cpu.h"
#include "../include/opcodes.h"
#include "../include/dram.h"
#include "../include/info_db.h"
#include "../include/color.h"

//=====================================================================================
//   CPU Initialization
//=====================================================================================

void cpu_init(CPU *cpu) {
    // 初始化通用寄存器
    for (int i = 0; i < 16; i++) {
        cpu->regs[i] = 0;
    }
    cpu->regs[0] = 0x00;        // register R0 hardwired to 0
    cpu->regs[16] = 0;          // R16 返回地址寄存器
    cpu->pc = DRAM_BASE;        // Set program counter to the base address

    // 初始化前一个周期的寄存器值
    memset(cpu->prev_regs, 0, sizeof(cpu->prev_regs));

    // 初始化域相关寄存器
    cpu->domain = 0;

    // 初始化计数器相关寄存器
    cpu->counters[0] = 0;
    cpu->counters[1] = 0;

    // 初始化定时器相关寄存器
    for (int i = 0; i < 2; i++) {
        cpu->timer[i] = 0;
        cpu->timer_enabled[i] = 0;
        cpu->timer_threshold[i] = 0;  // 定时器i的阈值
        cpu->timer_target_pc[i] = 0;  // 定时器i的目标PC
    }   

    // 初始化所有信息表
    info_db_init_all(cpu);
}

/*
 * get_inst_size
 * 作用：根据操作码获取指令的字节数。
 * 行为：
 *   - 根据操作码判断指令的字节数；
 *   - 返回指令的字节数。
 */
uint8_t get_inst_size(uint8_t opcode, CPU *cpu) {
    if (opcode == trigger || opcode == ret || opcode == timer_set)
        return 1;
    else if (opcode == trigger_pos || opcode == jmp || opcode == bl || opcode == display || opcode == edge_detect || opcode == domain_set || opcode == exec)
        return 2;
    else if (opcode == jmpc || opcode == arith_op || opcode == load || opcode == bit_slice)
        return 4;
    else if (opcode == mov) {
        // 需要读取更多字节来判断是2字节MOV还是8字节MOVI
        uint64_t inst = bus_load(&(cpu->bus), cpu->pc, 64); // 读取8字节
        // 检查是否是2字节MOV（寄存器到寄存器）
        // 条件：高48位全为0，且func[11]=1
        if ((inst >> 16) == 0 && ((inst >> 11) & 0x1) == 1) {
            return 2;  // 2字节MOV
        } else {
            return 8;  // 8字节MOVI
        }
    }
    else {
        fprintf(stderr, "%s[cpu][inst_size] unknown opcode 0x%x%s\n", ANSI_RED, opcode, ANSI_RESET);
        return 0;
    }
}

/*
 * getInstLength
 * 作用：根据当前PC值获取指令的字节数。
 * 行为：
 *   - 从当前PC位置读取8字节指令；
 *   - 根据操作码判断指令的字节数；
 *   - 返回指令的字节数。
 */
uint8_t getInstLength(CPU *cpu) {
    uint8_t opcode_byte = bus_load(&(cpu->bus), cpu->pc, 8);
    uint8_t opcode = (opcode_byte >> 4) & 0xF;
    return get_inst_size(opcode, cpu);
}

//=====================================================================================
// Instruction Fetch
//=====================================================================================

/*
 * cpu_fetch
 * 作用：从CPU总线获取指令。
 * 行为：
 *   - 根据当前PC值，从 DRAM 中读取指令；
 *   - 更新PC值为下一条指令的地址；
 *   - 返回读取到的指令。
 */
uint64_t cpu_fetch(CPU *cpu, uint8_t *inst_length) {
    // 检查指针有效性
    if (inst_length == NULL) {
        fprintf(stderr, "%s[cpu][fetch] NULL inst_length ptr!%s\n", ANSI_RED, ANSI_RESET);
        return 0;
    }
    *inst_length = getInstLength(cpu);
    if (*inst_length == 0) {
        fprintf(stderr, "%s[cpu][fetch] invalid inst length at pc %#.8x!%s\n", ANSI_RED, cpu->pc, ANSI_RESET);
        return 0;
    }
    if (cpu->pc + *inst_length > DRAM_SIZE) {
        fprintf(stderr, "%s[cpu][fetch] pc out of range: %#.8x!%s\n", ANSI_RED, cpu->pc, ANSI_RESET);
        return 0;
    }
    return bus_load(&(cpu->bus), cpu->pc, *inst_length * 8);
}

//=====================================================================================
// Assess Memory
//=====================================================================================

/*
 * cpu_load
 * 作用：从CPU总线加载数据。
 * 行为：
 *   - 调用总线加载函数，从 DRAM 中读取数据；
 *   - 返回读取到的数据。
 */
uint64_t cpu_load(CPU* cpu, uint64_t addr, uint64_t size) {
    return bus_load(&(cpu->bus), addr, size);
}

/*
 * cpu_store
 * 作用：向CPU总线存储数据。
 * 行为：
 *   - 调用总线存储函数，将数据写入 DRAM；
 *   - 无返回值。
 */
void cpu_store(CPU* cpu, uint64_t addr, uint64_t size, uint64_t value) {
    bus_store(&(cpu->bus), addr, size, value);
}

//=====================================================================================
// Instruction Decoder Functions
//=====================================================================================

//=====================================================================================
//   8BYTE Instruction Execution Functions
//=====================================================================================

/*
 * exec_MOVI
 * 作用：执行8字节MOVI指令。
 * 行为：
 *   - 根据操作码和立即数将立即数写入目标寄存器；
 *   - 更新CPU状态。
 */
void exec_MOVI(CPU* cpu, uint64_t inst) {
    // 8字节MOVI指令（立即数到寄存器）
    // 格式: [4bit op][1bit func][4bit dest][32bit imm][23bit rsv]
    // 条件：func[59] = 0
    uint8_t func_bit = (inst >> 59) & 0x1; // [59]
    if (func_bit != 0) {
        fprintf(stderr, "%s[cpu][decode] invalid MOVI func bit!%s\n", ANSI_RED, ANSI_RESET);
        assert(0);
    }

    uint8_t dst_reg = (inst >> 55) & 0xF;           // [58-55]
    uint32_t imm = (inst >> 23) & 0xFFFFFFFF;       // [54-23]

    printf("%smov r%u, 0x%x%s\n", ANSI_BOLD_BLUE, dst_reg, imm, ANSI_RESET);
    // 执行MOVI操作：立即数到寄存器
    cpu->regs[dst_reg] = imm;
}

/*
 * decode_eight_byte_inst
 * 作用：解码8字节指令。
 * 行为：
 *   - 根据操作码执行对应的8字节指令；
 *   - 返回1表示成功，返回0表示失败。
 */
int decode_eight_byte_inst(CPU* cpu, uint64_t inst) {
    uint8_t opcode = (inst >> 60) & 0xF;
    switch (opcode) {
        case 0x7: // MOVI (8-byte immediate)
            exec_MOVI(cpu, inst);
            break;
        default:
            fprintf(stderr, "%s[cpu][decode] 8-byte opcode:0x%x%s\n", ANSI_RED, opcode, ANSI_RESET);
            return 0;
    }
    return 1;
}

//=====================================================================================
//   4BYTE Instruction Execution Functions
//=====================================================================================

/*
 * exec_JMPC
 * 作用：执行跳转条件指令。
 * 行为：
 *   - 根据操作码和源寄存器值执行跳转条件判断；
 *   - 如果条件满足，更新PC寄存器。
 */
void exec_JMPC(CPU* cpu, uint32_t inst) {
    uint32_t func = (inst >> 24) & 0xF;
    uint32_t src1_reg = (inst >> 20) & 0xF;
    uint32_t src2_reg = (inst >> 16) & 0xF;
    int8_t addr = (int8_t)((inst >> 8) & 0xFF);

    // 打印跳转条件指令
    const char* func_symbols[] = {"==", "!=", ">", "<", ">=", "<="};
    if (func <= 0x5) {
        printf("%sjmpc r%u %s r%u addr=0x%x%s\n", ANSI_BOLD_BLUE, src1_reg, func_symbols[func], src2_reg, addr, ANSI_RESET);
    } else {
        if (func == 0x6) {
            printf("%sjmpc r%u == 'bP addr=0x%x%s\n", ANSI_BOLD_BLUE, src1_reg, addr, ANSI_RESET);
        } else {
            printf("%sjmpc r%u == 'bN addr=0x%x%s\n", ANSI_BOLD_BLUE, src1_reg, addr, ANSI_RESET);
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
        case 0x6: { // 上升沿
            uint8_t prev = cpu->prev_regs[src1_reg] & 1;
            uint8_t curr = cpu->regs[src1_reg] & 1;
            should_jump = (prev == 0 && curr == 1);
            break;
        }
        case 0x7: { // 下降沿
            uint8_t prev = cpu->prev_regs[src1_reg] & 1;
            uint8_t curr = cpu->regs[src1_reg] & 1;
            should_jump = (prev == 1 && curr == 0);
            break;
        }
        default:
            fprintf(stderr, "%s[cpu][decode] exec_JMPC error!%s\n", ANSI_RED, ANSI_RESET);
            assert(0);
            return;
    }
    
    // 如果条件满足，执行跳转
    if (should_jump) {
        cpu->pc += addr;
    }
}

/*
 * exec_ARITH_OP
 * 作用：执行算术操作指令。
 * 行为：
 *   - 根据操作码执行对应的算术操作；
 *   - 更新目标寄存器的值。
 */
void exec_ARITH_OP(CPU* cpu, uint32_t inst) {
    uint8_t func = (inst >> 24) & 0xF;
    uint8_t dst_reg = (inst >> 20) & 0xF;
    uint8_t src1_reg = (inst >> 16) & 0xF;
    uint8_t src2_reg = (inst >> 12) & 0xF;
    uint32_t src1 = cpu->regs[src1_reg];
    uint32_t src2 = cpu->regs[src2_reg];
    switch (func) {
        case 0x0:
            printf("%sbit_op r%u = r%u & r%u%s\n", ANSI_BOLD_BLUE, dst_reg, src1_reg, src2_reg, ANSI_RESET);
            cpu->regs[dst_reg] = src1 & src2;
            break;
        case 0x1:
            printf("%sbit_op r%u = r%u | r%u%s\n", ANSI_BOLD_BLUE, dst_reg, src1_reg, src2_reg, ANSI_RESET);
            cpu->regs[dst_reg] = src1 | src2;
            break;
        case 0x2:
            printf("%sbit_op r%u = r%u ^ r%u%s\n", ANSI_BOLD_BLUE, dst_reg, src1_reg, src2_reg, ANSI_RESET);
            cpu->regs[dst_reg] = src1 ^ src2;
            break;
        case 0x3: {
            printf("%sredu_and r%u = &r%u%s\n", ANSI_BOLD_BLUE, dst_reg, src1_reg, ANSI_RESET);
            uint32_t x = src1;
            uint32_t r = 1;
            for (int i = 0; i < 32; i++) r &= ((x >> i) & 1); // 与操作，判断是否所有位都是1
            cpu->regs[dst_reg] = r;
            break;
        }
        case 0x4: {
            printf("%sredu_or r%u = |r%u%s\n", ANSI_BOLD_BLUE, dst_reg, src1_reg, ANSI_RESET);
            uint32_t x = src1;
            uint32_t r = 0;
            for (int i = 0; i < 32; i++) r |= ((x >> i) & 1); // 或操作，判断是否有任意位是1
            cpu->regs[dst_reg] = r;
            break;
        }
        case 0x5: {
            printf("%sredu_xor r%u = ^r%u%s\n", ANSI_BOLD_BLUE, dst_reg, src1_reg, ANSI_RESET);
            uint32_t x = src1;
            uint32_t r = 0;
            for (int i = 0; i < 32; i++) r ^= ((x >> i) & 1); // 异或操作，判断是否有奇数个1
            cpu->regs[dst_reg] = r;
            break;
        }
        case 0x6:
            printf("%sconcat r%u = {r%u,r%u}%s\n", ANSI_BOLD_BLUE, dst_reg, src1_reg, src2_reg, ANSI_RESET);
            cpu->regs[dst_reg] = ((src1 & 0xFFFF) << 16) | (src2 & 0xFFFF); // 拼接操作，将src1的高16位和src2的低16位拼接起来，暂不考虑溢出
            break;
        case 0x7:
            printf("%sisunknow r%u = isunknow(r%u)%s\n", ANSI_BOLD_BLUE, dst_reg, src1_reg, ANSI_RESET);
            cpu->regs[dst_reg] = 1; // 暂不考虑isunknow操作，默认返回1
            break;
        case 0x8:
            printf("%sadd r%u = r%u + r%u%s\n", ANSI_BOLD_BLUE, dst_reg, src1_reg, src2_reg, ANSI_RESET);
            cpu->regs[dst_reg] = src1 + src2;
            break;
        case 0x9:
            printf("%ssub r%u = r%u - r%u%s\n", ANSI_BOLD_BLUE, dst_reg, src1_reg, src2_reg, ANSI_RESET);
            cpu->regs[dst_reg] = src1 - src2;
            break;
        default:
            fprintf(stderr, "%s[cpu][decode] exec_ARITH_OP error!\n%s", ANSI_RED, ANSI_RESET);
            assert(0);
            break;
    }
}

/*
 * exec_BIT_SLICE
 * 作用：执行位切片操作指令。
 * 行为：
 *   - 根据操作码执行对应的位切片操作；
 *   - 更新目标寄存器的值。
 */
void exec_BIT_SLICE(CPU* cpu, uint32_t inst) {
    uint8_t Dst = (inst >> 24) & 0xF;      // [27-24]
    uint8_t Src = (inst >> 20) & 0xF;      // [23-20]
    uint8_t End = (inst >> 15) & 0x1F;   // [19-15]
    uint8_t Start  = (inst >> 10) & 0x1F;     // [14-10]
    printf("%sbit_slice r%u r%u[%u:%u]%s\n", ANSI_BOLD_BLUE, Dst, Src, End, Start, ANSI_RESET);
    // 实际BIT_SLICE操作
    uint32_t src1 = cpu->regs[Src];    
    // 确保Start <= End
    if (Start > End) {
        fprintf(stderr, "%s[cpu][decode] bit_slice error: Start > End%s\n", ANSI_RED, ANSI_RESET);
        assert(0);
    }
    // 计算掩码: 创建一个长度为(End-Start+1)的全1位掩码
    uint32_t mask = ((1U << (End - Start + 1)) - 1);
    // 右移提取指定位段，然后通过掩码保留需要的位
    uint32_t res = (src1 >> Start) & mask;
    // 存储结果到目标寄存器
    cpu->regs[Dst] = res;
}

/*
 * exec_LOAD
 * 作用：执行加载指令。
 * 行为：
 *   - 根据操作码执行对应的加载操作；
 *   - 更新目标寄存器的值。
 */
void exec_LOAD(CPU* cpu, uint32_t inst) {
    uint32_t dst = (inst >> 24) & 0xF;
    uint32_t addr = inst & 0xFFFFFF;
    printf("%sload r%u 0x%x%s\n", ANSI_BOLD_BLUE, dst, addr, ANSI_RESET);
    // 实际LOAD操作可在此实现，获取信号变量值（拆分汇聚处理后）
    uint32_t val = get_signal_value(addr);
    cpu->regs[dst] = val;
    printf("%sGet signal var from addr[0x%x] = 0x%x%s\n", ANSI_BOLD_GREEN, addr, val, ANSI_RESET);
}

/*
 * decode_four_byte_inst
 * 作用：解码并执行4字节指令。
 * 行为：
 *   - 根据操作码执行对应的4字节指令操作；
 *   - 更新CPU状态。
 */
int decode_four_byte_inst(CPU* cpu, uint64_t inst) {
    uint32_t inst_32 = inst & 0xFFFFFFFF;
    uint8_t opcode = (inst_32 >> 28) & 0xF;
    switch (opcode) {
        case 0x0: // JMPC
            exec_JMPC(cpu, inst_32);
            break;
        case 0x1: // ARITH_OP
            exec_ARITH_OP(cpu, inst_32);
            break;
        case 0x6: // BIT_SLICE
            exec_BIT_SLICE(cpu, inst_32);
            break;
        case 0xD: // LOAD
            exec_LOAD(cpu, inst_32);
            break;
        default:
            fprintf(stderr, "%s[cpu][decode] 4-byte opcode:0x%x!%s\n", ANSI_RED, opcode, ANSI_RESET);
            assert(0);
            return 0;
    }
    return 1;
}

//=====================================================================================
//   2BYTE Instruction Execution Functions
//=====================================================================================

/*
 * exec_TRIGGER_POS
 * 作用：执行触发位置指令。
 * 行为：
 *   - 根据操作码执行对应的触发位置操作；
 *   - 更新触发位置。
 */
void exec_TRIGGER_POS(CPU* cpu, uint16_t inst) {
    // 立即数imm为[11:5]位
    uint8_t imm = (inst >> 5) & 0x7F;
    printf("%strigger_pos %u%s\n", ANSI_BOLD_BLUE, imm, ANSI_RESET);
    // 实际TRIGGER_POS操作可在此实现
    printf("%sTrigger sample pos set %u%%!%s\n", ANSI_BOLD_GREEN, imm, ANSI_RESET);
}

/*
 * exec_JMP
 * 作用：执行基本快跳转指令，无需返回。
 * 行为：
 *   - 根据操作码执行对应的跳转操作；
 *   - 更新PC寄存器。
 */
void exec_JMP(CPU* cpu, uint16_t inst) {
    // offset为[11:4]，8位有符号
    int16_t offset = (inst >> 4) & 0xFF;
    // 处理有符号扩展
    if (offset & 0x80) offset = -(256 - offset);
    printf("%sjmp %d%s\n", ANSI_BOLD_BLUE, offset, ANSI_RESET);
    // 实际JMP操作可在此实现
    cpu->pc += offset;
}

/*
 * exec_MOV
 * 作用：执行2字节MOV指令。
 * 行为：
 *   - 根据操作码执行对应的2字节MOV操作；
 *   - 更新目标寄存器的值。
 */
void exec_MOV(CPU* cpu, uint16_t inst) {
    // 2字节MOV指令（寄存器到寄存器）
    // 格式: [4bit op][1bit func][4bit dest][4bit src][3bit rsv]
    uint8_t dst_reg = (inst >> 7) & 0xF;      // bits [10:7]
    uint8_t src_reg = (inst >> 3) & 0xF;      // bits [6:3]
    
    printf("%smov r%u, r%u%s\n", ANSI_BOLD_BLUE, dst_reg, src_reg, ANSI_RESET);
    
    // 执行MOV操作：寄存器到寄存器
    cpu->regs[dst_reg] = cpu->regs[src_reg];
}

/*
 * exec_BL
 * 作用：执行函数跳转指令，需返回。
 * 行为：
 *   - 根据操作码执行对应的跳转操作；
 *   - 更新PC寄存器。
 */
void exec_BL(CPU* cpu, uint16_t inst) {
    // offset为[11:2]，10位有符号fmovi
    int16_t offset = (inst >> 2) & 0x3FF;
    // 处理有符号扩展
    if (offset & 0x200) offset = -(1024 - offset);
    printf("%sbl %d%s\n", ANSI_BOLD_BLUE, offset, ANSI_RESET);
    // 实际BL操作可在此实现
    cpu->regs[16] = cpu->pc;  // 保存返回地址到R16
    cpu->pc += offset;
}

/*
 * exec_DOMAIN_SET
 * 作用：执行域设置指令。
 * 行为：
 *   - 根据操作码执行对应的域设置操作；
 *   - 更新当前域。
 */
void exec_DOMAIN_SET(CPU* cpu, uint16_t inst) {
    // offset为[11:4]，8位无符号
    uint8_t offset = (inst >> 4) & 0xFF;
    printf("%sdomain %d%s\n", ANSI_BOLD_BLUE, offset, ANSI_RESET);
    cpu->domain = offset;
    char* info = get_domain_info(offset);
    if (info) {
        printf("%sdomain(%s)%s\n", ANSI_BOLD_GREEN, info, ANSI_RESET);
    } else {
        fprintf(stderr, "%s[cpu][db] domain not found: %u!%s\n", ANSI_RED, offset, ANSI_RESET);
        assert(0);
    }
}

/*
 * exec_DISPLAY
 * 作用：执行显示指令。
 * 行为：
 *   - 根据操作码执行对应的显示操作；
 *   - 打印显示字符串。
 */
void exec_DISPLAY(CPU* cpu, uint16_t inst) {
    // 根据指令格式 [4bit op][10bit id][2bit rsv]
    uint16_t id = (inst >> 2) & 0x3FF;
    printf("%sdisplay %u%s\n", ANSI_BOLD_BLUE, id, ANSI_RESET);
    
    // 获取完整的display字符串（格式+变量）
    char* complete_string = get_complete_display_string(id);
    if (complete_string) {
        printf("%sdisplay(%s)%s\n", ANSI_BOLD_GREEN, complete_string, ANSI_RESET);
    } else {
        fprintf(stderr, "%s[cpu][db] display not found: %u!%s\n", ANSI_RED, id, ANSI_RESET);
    }
}

/*
 * exec_EXEC
 * 作用：执行执行指令。
 * 行为：
 *   - 根据操作码执行对应的执行操作；
 *   - 打印执行函数信息。
 */
void exec_EXEC(CPU* cpu, uint16_t inst_16) {
    uint16_t id = (inst_16 >> 2) & 0x3FF;
    printf("%sexec %u%s\n", ANSI_BOLD_BLUE, id, ANSI_RESET);
    char* info = get_exec_info(id);
    if (info) {
        printf("%sexec(%s)%s\n", ANSI_BOLD_GREEN, info, ANSI_RESET);
    } else {
        printf("%sExec: No info found for ID %u%s\n", ANSI_RED, id, ANSI_RESET);
        assert(0);
    }
}

/*
 * exec_EDGE_DETECT
 * 作用：执行边缘检测指令。
 * 行为：
 *   - 根据操作码执行对应的边缘检测操作；
 *   - 更新目标寄存器的值。
 */
void exec_EDGE_DETECT(CPU* cpu, uint16_t inst) {
    uint8_t dst = (inst >> 8) & 0xF;
    uint8_t src = (inst >> 4) & 0xF;
    uint8_t func = (inst >> 1) & 0x7;
    uint8_t curr = cpu->regs[src] & 0x1;
    uint8_t prev = cpu->prev_regs[src] & 0x1; // 前一个FCLK周期，注意这里不是TSL软核的时钟周期而是EMU的时钟周期的信号状态
    uint32_t res = 0;
    if (func == 0) { // 正沿（上升沿，信号从0变为1）
        printf("%sedge_detect r%u r%u==P%s\n", ANSI_BOLD_BLUE, dst, src, ANSI_RESET);
        res = (prev == 0 && curr == 1) ? 1 : 0;
    } else if (func == 1) { // 负沿（下降沿，信号从1变为0）
        printf("%sedge_detect r%u r%u==N%s\n", ANSI_BOLD_BLUE, dst, src, ANSI_RESET);
        res = (prev == 1 && curr == 0) ? 1 : 0;
    } else if (func == 2) { // 任意跳变（正沿或负沿，即信号状态发生变化）
        printf("%sedge_detect r%u r%u==T%s\n", ANSI_BOLD_BLUE, dst, src, ANSI_RESET);
        res = (prev != curr) ? 1 : 0;
    } else if (func == 3) { // 稳定低电平（连续2个FCLK周期保持0）
        printf("%sedge_detect r%u r%u==L%s\n", ANSI_BOLD_BLUE, dst, src, ANSI_RESET);
        res = (prev == 0 && curr == 0) ? 1 : 0;
    } else if (func == 4) { // 稳定高电平（连续2个FCLK周期保持1）
        printf("%sedge_detect r%u r%u==H%s\n", ANSI_BOLD_BLUE, dst, src, ANSI_RESET);
        res = (prev == 1 && curr == 1) ? 1 : 0;
    } else if (func == 5) { // 稳定状态（连续2个FCLK周期保持低或高，即无跳变）
        printf("%sedge_detect r%u r%u==S%s\n", ANSI_BOLD_BLUE, dst, src, ANSI_RESET);
        res = (prev == curr) ? 1 : 0;
    } else if (func == 6) { // 不关心（任何值都视为匹配）
        printf("%sedge_detect r%u r%u==X%s\n", ANSI_BOLD_BLUE, dst, src, ANSI_RESET);
        res = 1;
    } else {
        printf("%sedge_detect r%u==UNK(%u)%s\n", ANSI_BOLD_RED, src, func, ANSI_RESET);
        assert(0);
    }
    cpu->regs[dst] = res;
}

/*
 * decode_two_byte_inst
 * 作用：解码并执行2字节指令。
 * 行为：
 *   - 根据操作码执行对应的2字节指令操作；
 *   - 更新CPU状态。
 */
int decode_two_byte_inst(CPU* cpu, uint64_t inst) {
    uint16_t inst_16 = inst & 0xFFFF;
    uint8_t opcode = (inst_16 >> 12) & 0xF;
    switch (opcode) {
        case 0x4: // TRIGGER_POS
            exec_TRIGGER_POS(cpu, inst_16);
            break;
        case 0x5: // JMP
            exec_JMP(cpu, inst_16);
            break;
        case 0x7: // MOV (2-byte, register to register)
            exec_MOV(cpu, inst_16);
            break;
        case 0x9: // BL
            exec_BL(cpu, inst_16);
            break;
        case 0xA: // DOMAIN_SET
            exec_DOMAIN_SET(cpu, inst_16);
            break;
        case 0xB: // DISPLAY
            exec_DISPLAY(cpu, inst_16);
            break;
        case 0xC: // EXEC
            exec_EXEC(cpu, inst_16);
            break;
        case 0xE: // EDGE_DETECT
            exec_EDGE_DETECT(cpu, inst_16);
            break;
        default:
            fprintf(stderr, "%s[cpu][decode] 2-byte opcode:0x%x!%s\n", ANSI_RED, opcode, ANSI_RESET);
            return 0;
    }
    return 1;
}

//=====================================================================================
//   1BYTE Instruction Execution Functions
//=====================================================================================

/*
 * exec_TRIGGER
 * 作用：执行触发指令。
 * 行为：
 *   - 根据操作码执行对应的触发操作；
 *   - 打印触发信号样本信息。
 */
void exec_TRIGGER(CPU* cpu, uint8_t inst) {
    printf("%strigger%s\n", ANSI_BOLD_BLUE, ANSI_RESET);
    // 实际TRIGGER操作可在此实现
    printf("%sTime stop! Start trigger signal sample!%s\n", ANSI_BOLD_GREEN, ANSI_RESET);
}

/*
 * exec_RET
 * 作用：执行返回指令。
 * 行为：
 *   - 根据操作码执行对应的返回操作；
 *   - 更新PC寄存器。
 */
void exec_RET(CPU* cpu, uint8_t inst) {
    printf("%sret%s\n", ANSI_BOLD_BLUE, ANSI_RESET);
    // 实际RET操作可在此实现
    cpu->pc = cpu->regs[16];
    cpu->regs[16] = 0;
}

/*
 * exec_TIMER_SET
 * 作用：执行定时器设置指令
 * 行为：
 *   - 根据操作码执行对应的定时器设置操作；
 * 示例：
 *   exec_TIMER_SET(0, 0x0) => 无返回值 (定时器0重置)
 *   exec_TIMER_SET(0, 0x1) => 无返回值 (定时器0去使能)
 *   exec_TIMER_SET(0, 0x2) => 无返回值 (定时器0使能)
 */
void exec_TIMER_SET(CPU* cpu, uint8_t inst) {
    printf("%stimer_set%s\n", ANSI_BOLD_BLUE, ANSI_RESET);
    // 实际TIMER_SET操作可在此实现
    uint8_t id = (inst >> 3) & 0x1;
    uint8_t func = (inst >> 1) & 0x3;
    const char* id_names[] = {"0", "1"};
    const char* func_names[] = {"reset", "disable", "enable"};
    const char* fname = (func < 3) ? func_names[func] : "unknown";
    printf("%stimer%s %s%s\n", ANSI_BOLD_GREEN, id_names[id], fname, ANSI_RESET);
    if (func == 0) {
        cpu->timer[id] = 0;
    } else if (func == 1) {
        cpu->timer_enabled[id] = 0;
    } else if (func == 2) {
        cpu->timer_enabled[id] = 1;
    } else {
        fprintf(stderr, "%s[cpu][timer] unknown func: %u!%s\n", ANSI_RED, func, ANSI_RESET);
    }
}

/*
 * decode_one_byte_inst
 * 作用：解码并执行1字节指令。
 * 行为：
 *   - 根据操作码执行对应的1字节指令操作；
 *   - 更新CPU状态。
 */
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
        case 0x0F: // timer_set
            exec_TIMER_SET(cpu, inst_8);
            break;
        default: {
            fprintf(stderr, "%s[cpu][decode] 1-byte opcode:0x%x!%s\n", ANSI_RED, opcode, ANSI_RESET);
            return 0;
        }
    }
    return 1;
}

//=====================================================================================
//   Dump Register Info
//=====================================================================================

/*
 * dump_registers
 * 作用：打印CPU寄存器状态。
 * 行为：
 *   - 打印16个寄存器状态；
 *   - 打印PC寄存器状态。
 */
void dump_registers(CPU *cpu) {
    char* abi[] = { // Application Binary Interface registers
        "R0", "R1",  "R2",  "R3",
        "R4", "R5",  "R6",  "R7",
        "R8", "R9",  "R10",  "R11",
        "R12", "R13",  "R14",  "R15",
    };

    int N = 16;
    for (int i = 0; i < N; i++) {
        print_color(ANSI_BOLD);
        printf("   %4s: %#-13.2x  ", abi[i], cpu->regs[i]);
        print_color(ANSI_RESET);
        if (i % 4 == 3)
            printf("\n");
    }

    print_color(ANSI_BOLD);
    printf("   %4s: %#-13.2x  ", "C0", cpu->counters[0]);
    printf("   %4s: %#-13.2x  ", "C1", cpu->counters[1]);
    printf("   %4s: %#-13.2x  ", "T0", cpu->timer[0]);
    printf("   %4s: %#-13.2x  ", "T1", cpu->timer[1]);
    printf("   %4s: %#-13.2x  ", "PC", cpu->pc);
    print_color(ANSI_RESET);
}

//=====================================================================================
//   Cpu Execution root function
//=====================================================================================

/*
 * cpu_execute
 * 作用：执行CPU指令。
 * 行为：
 *   - 根据指令长度解码并执行指令；
 *   - 更新CPU状态。
 */
int cpu_execute(CPU *cpu, uint64_t inst, uint8_t inst_length) {
    // 打印当前指令地址
    print_color(ANSI_YELLOW);
    printf("\n%#.8x -> ", cpu->pc);
    print_color(ANSI_RESET);

    for (int i = 0; i < 16; i++) cpu->prev_regs[i] = 1; // 前一个FCLK周期，注意这里不是TSL软核的时钟周期而是EMU的时钟周期的信号状态，这里赋值模拟

    cpu->pc += inst_length; // update pc for next cpu cycle

    if (inst_length == 1) {
        decode_one_byte_inst(cpu, inst);
    } else if (inst_length == 2) {
        decode_two_byte_inst(cpu, inst);
    } else if (inst_length == 4) {
        decode_four_byte_inst(cpu, inst);
    } else if (inst_length == 8) {
        decode_eight_byte_inst(cpu, inst);
    } else {
        fprintf(stderr, "%s[-] ERROR-> inst_length:0x%x!%s\n", ANSI_RED, inst_length, ANSI_RESET);
        return 0;
    }

    // 执行定时器 tick 并跳转
    timer_tick_and_jump(cpu);

    return 1;  // 明确返回执行状态（1表示正常，0表示异常）
}

/*
 * cpu_cleanup
 * 作用：释放CPU相关资源。
 * 行为：
 *   - 释放显示信息表、执行信息表、域信息表、定时器信息表等资源。
 */
void cpu_cleanup(CPU *cpu) {
    free_display_info_table();
    free_exec_info_table();
    free_domain_info_table();
    free_timer_info_table();
}
