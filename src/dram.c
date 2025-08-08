#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <execinfo.h>  // For backtrace and backtrace_symbols
#include <unistd.h>    // For STDOUT_FILENO
#include "../include/dram.h"  // Include the header with DRAM_BASE definition

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
    // Check if address is within DRAM bounds
    if (addr >= DRAM_SIZE) {
        fprintf(stderr, "[-] ERROR: Address 0x%08lx is outside DRAM bounds (0x%08x)\n", 
                addr, DRAM_SIZE);
        return 0;
    }
    
    // Check if access would cross DRAM boundary
    if (addr + (size / 8) > DRAM_SIZE) {
        fprintf(stderr, "[-] ERROR: Access at 0x%08lx with size %lu would cross DRAM boundary\n", 
                addr, size);
        return 0;
    }
    
    switch (size) {
        case 8:  return dram_load_8(dram, addr);
        case 16: return dram_load_16(dram, addr);
        case 32: return dram_load_32(dram, addr);
        case 64: return dram_load_64(dram, addr);
        default: {
            fprintf(stderr, "[-] ERROR: Invalid access size: %lu at address 0x%08lx\n", size, addr);
            fprintf(stderr, "           Valid sizes are: 8, 16, 32, 64 bits\n");
            fprintf(stderr, "           Called from: %p\n", __builtin_return_address(0));
            
            // Print a backtrace for debugging
            #ifdef __linux__
            void *array[10];
            size_t bt_size = backtrace(array, 10);
            char **bt_strings = backtrace_symbols(array, bt_size);
            if (bt_strings != NULL) {
                fprintf(stderr, "Backtrace (most recent call first):\n");
                for (size_t i = 0; i < bt_size; i++) {
                    fprintf(stderr, "  %s\n", bt_strings[i]);
                }
                free(bt_strings);
            }
            #endif
            
            // Instead of asserting, return 0 to allow graceful error handling
            return 0;
        }
    }
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
