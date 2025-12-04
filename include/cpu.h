#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "bus.h"

// VU13P中，BRAM数为2688，URAM数为1280
// 单个Block RAM是36K bit，单个Ultra RAM是288K bit
// BRAM数为 2688 * 36K = 94.5M bit = 12MB
// URAM数为 1280 * 288K = 360M bit = 45MB
// 总内存为94.5M + 360M = 454.5M bit = 56.8M Byte

typedef struct CPU {
    uint32_t regs[17];          // 16 32-bit registers (R0-R15)
    uint32_t pc;                // 32-bit program counter
    struct BUS bus;             // CPU connected to BUS
    uint32_t prev_regs[16];     // 上一周期（或上一条指令执行前）寄存器快照：用于比较变化、生成display信息、触发trace/观察点，避免在同一周期内读写竞争；R0-R15按顺序对应
    uint8_t  domain;
    uint32_t counters[2];
    uint32_t timer[2];
    uint8_t  timer_enabled[2];
    uint32_t timer_threshold[2];   // 计时器阈值，当计数达到该值触发跳转
    uint32_t timer_target_pc[2];   // 计时器触发后的目标PC（绝对地址，DRAM 基址空间）
} CPU;

// CPU基本操作函数
void cpu_init(struct CPU *cpu);
uint64_t cpu_fetch(struct CPU *cpu, uint8_t *inst_length);
int cpu_execute(struct CPU *cpu, uint64_t inst, uint8_t inst_length);
void dump_registers(struct CPU *cpu);
void cpu_cleanup(struct CPU *cpu);

// DB信息表相关函数
char* get_complete_display_string(uint32_t id);
void set_info_base(const char* dir);
void set_ansi_color_enabled(int enabled);

#endif
