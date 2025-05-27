#include "../includes/dram.h"
#include <stdio.h>
#include <assert.h>

uint64_t dram_load_8(DRAM* dram, uint64_t addr){
    return (uint64_t) dram->mem[addr - DRAM_BASE];
}

uint64_t dram_load_16(DRAM* dram, uint64_t addr){
    return ((uint64_t)dram->mem[addr-DRAM_BASE] << 8) |
           (uint64_t)dram->mem[addr-DRAM_BASE + 1];
}

uint64_t dram_load_32(DRAM* dram, uint64_t addr){
    return ((uint64_t)dram->mem[addr-DRAM_BASE] << 24) |
           ((uint64_t)dram->mem[addr-DRAM_BASE + 1] << 16) |
           ((uint64_t)dram->mem[addr-DRAM_BASE + 2] << 8) |
           (uint64_t)dram->mem[addr-DRAM_BASE + 3];
}

uint64_t dram_load_64(DRAM* dram, uint64_t addr){
    return ((uint64_t)dram->mem[addr-DRAM_BASE] << 56) |
           ((uint64_t)dram->mem[addr-DRAM_BASE + 1] << 48) |
           ((uint64_t)dram->mem[addr-DRAM_BASE + 2] << 40) |
           ((uint64_t)dram->mem[addr-DRAM_BASE + 3] << 32) |
           ((uint64_t)dram->mem[addr-DRAM_BASE + 4] << 24) |
           ((uint64_t)dram->mem[addr-DRAM_BASE + 5] << 16) |
           ((uint64_t)dram->mem[addr-DRAM_BASE + 6] << 8) |
           (uint64_t)dram->mem[addr-DRAM_BASE + 7];
}

uint64_t dram_load(DRAM* dram, uint64_t addr, uint64_t size) {
    switch (size) {
        case 8:  return dram_load_8(dram, addr);  break;
        case 16: return dram_load_16(dram, addr); break;
        case 32: return dram_load_32(dram, addr); break;
        case 64: return dram_load_64(dram, addr); break;
        default: {
            fprintf(stderr, "[-] ERROR-> dram load error!\n");
            assert(0);
            break;
        }
    }
    return 1;
}

void dram_store_8(DRAM* dram, uint64_t addr, uint64_t value) {
    dram->mem[addr-DRAM_BASE] = (uint8_t) (value & 0xff);
}

void dram_store_16(DRAM* dram, uint64_t addr, uint64_t value) {
    dram->mem[addr-DRAM_BASE]     = (uint8_t)((value >> 8) & 0xff);
    dram->mem[addr-DRAM_BASE + 1] = (uint8_t)(value & 0xff);
}

void dram_store_32(DRAM* dram, uint64_t addr, uint64_t value) {
    dram->mem[addr-DRAM_BASE]     = (uint8_t)((value >> 24) & 0xff);
    dram->mem[addr-DRAM_BASE + 1] = (uint8_t)((value >> 16) & 0xff);
    dram->mem[addr-DRAM_BASE + 2] = (uint8_t)((value >> 8) & 0xff);
    dram->mem[addr-DRAM_BASE + 3] = (uint8_t)(value & 0xff);
}

void dram_store_64(DRAM* dram, uint64_t addr, uint64_t value) {
    dram->mem[addr-DRAM_BASE]     = (uint8_t)((value >> 56) & 0xff);
    dram->mem[addr-DRAM_BASE + 1] = (uint8_t)((value >> 48) & 0xff);
    dram->mem[addr-DRAM_BASE + 2] = (uint8_t)((value >> 40) & 0xff);
    dram->mem[addr-DRAM_BASE + 3] = (uint8_t)((value >> 32) & 0xff);
    dram->mem[addr-DRAM_BASE + 4] = (uint8_t)((value >> 24) & 0xff);
    dram->mem[addr-DRAM_BASE + 5] = (uint8_t)((value >> 16) & 0xff);
    dram->mem[addr-DRAM_BASE + 6] = (uint8_t)((value >> 8) & 0xff);
    dram->mem[addr-DRAM_BASE + 7] = (uint8_t)(value & 0xff);
}

void dram_store(DRAM* dram, uint64_t addr, uint64_t size, uint64_t value) {
    switch (size) {
        case 8:  dram_store_8(dram, addr, value);  break;
        case 16: dram_store_16(dram, addr, value); break;
        case 32: dram_store_32(dram, addr, value); break;
        case 64: dram_store_64(dram, addr, value); break;
        default: {
            fprintf(stderr, "[-] ERROR-> dram store error!\n");
            break;
        }
    }
}
