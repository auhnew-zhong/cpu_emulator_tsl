#ifndef INFO_DB_H
#define INFO_DB_H

#include <stdint.h>

#include "cpu.h"

//=====================================================================================
//   Signal table
//=====================================================================================
struct signal_entry {
    uint32_t addr;
    uint32_t value;
};

// 基础设施
void set_info_base(const char* dir);
void info_db_init_all(CPU* cpu);
uint32_t get_signal_value(uint32_t addr);

// Display 信息表
void init_display_info_table();
void free_display_info_table();
char* get_display_format(uint32_t id);
char* get_complete_display_string(uint32_t id);

// Exec 信息表
void init_exec_info_table();
void free_exec_info_table();
char* get_exec_info(uint32_t id);

// Domain 信息表
void init_domain_info_table();
void free_domain_info_table();
char* get_domain_info(uint32_t id);

// Timer 跳转
void timer_tick_and_jump(CPU* cpu);

#endif
