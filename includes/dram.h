#ifndef DRAM_H
#define DRAM_H

#include <stdint.h>

// TSL源程序假设有200条指令，指令最大字节为8B，总字节数为 200 * 8 = 1600B
// 对应的1600B / 1024 = 1.56KB，1.56KB / 1024 = 0.0156MB，
// 所以DRAM大小至少为 1.56KB * 2 = 3.12KB
// 为了方便，我们将DRAM大小设置为 1024 * 32 = 32KB，小于一个BRAM的块RAM大小
// 同时，为了方便，我们将DRAM的基地址设置为 0x00010000
#define DRAM_SIZE 1024*32
#define DRAM_BASE 0x00010000

typedef struct DRAM {
	uint8_t mem[DRAM_SIZE];     // Dram memory of DRAM_SIZE
} DRAM;

uint64_t dram_load(DRAM* dram, uint64_t addr, uint64_t size);
uint64_t dram_load_8(DRAM* dram, uint64_t addr);
uint64_t dram_load_16(DRAM* dram, uint64_t addr);
uint64_t dram_load_32(DRAM* dram, uint64_t addr);
uint64_t dram_load_64(DRAM* dram, uint64_t addr);

void dram_store(DRAM* dram, uint64_t addr, uint64_t size, uint64_t value);
void dram_store_8(DRAM* dram, uint64_t addr, uint64_t value);
void dram_store_16(DRAM* dram, uint64_t addr, uint64_t value);
void dram_store_32(DRAM* dram, uint64_t addr, uint64_t value);
void dram_store_64(DRAM* dram, uint64_t addr, uint64_t value);

#endif
