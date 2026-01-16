#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include "../include/color.h"
#include "../include/info_db.h"


// 示例信号表，可以根据实际需求扩展
struct signal_entry signal_table[] = {
    {0x00000004, 0x1001cccc},
    {0x00001000, 0x10001111},
    {0x00001004, 0x000050ee},
    {0x00001008, 0x0000600f},
    {0x00001024, 0x000011f1},
    {0x00002048, 0x000022a2},
    {0x00004096, 0x000033b3},
    {0x00004104, 0x10011ccd},
    {0x00008192, 0x000044c4},
};

const int signal_table_size = sizeof(signal_table) / sizeof(signal_table[0]);

/*
 * get_signal_value
 * 作用：根据信号地址查询信号值。
 * 行为：
 *   - 遍历信号表，查找匹配的地址；
 *   - 如果找到，返回对应值；
 *   - 如果未找到，打印错误日志并返回 0。
 * 示例：
 *   get_signal_value(0x00000004) => 0x1001cccc
 *   get_signal_value(0x00001000) => 0x10001111
 *   get_signal_value(0x00008192) => 0x000044c4
 *   get_signal_value(0x00009000) => 错误日志，返回 0
 */
uint32_t get_signal_value(uint32_t addr) {
    for (int i = 0; i < signal_table_size; ++i) {
        if (signal_table[i].addr == addr)
            return signal_table[i].value;
    }
    fprintf(stderr, "[cpu][signal] not found: 0x%x\n", addr);
    return 0;
}

typedef struct display_info_entry {
    uint32_t id;
    char* format;
    char* content;
} display_info_entry;

static display_info_entry* display_info_table = NULL;
static int display_info_table_size = 0;
static char info_base_dir[512] = "examples";

/*
 * set_info_base
 * 作用：设置信息库基目录，用于后续拼接如 "display_info.db"、"exec_info.db" 等文件路径。
 * 行为：
 *   - 如果传入的是包含 '/' 的路径（文件或目录），取最后一个 '/' 之前的部分作为目录；
 *   - 如果不包含 '/'，则直接将整个字符串作为目录；
 *   - 进行安全拷贝并在末尾补 '\0'，避免越界。
 * 兼容性：仅处理 POSIX 分隔符 '/'，不对 Windows '\\' 做特殊处理。
 * 示例：
 *   set_info_base("/home/user/project/examples/display_info.db") => info_base_dir = "/home/user/project/examples"
 *   set_info_base("/home/user/project/examples/")                => info_base_dir = "/home/user/project/examples"
 *   set_info_base("examples")                                    => info_base_dir = "examples"
 *   set_info_base("/examples")                                   => info_base_dir = ""（空字符串）；后续拼接将形成 "/display_info.db" 绝对路径。
 */
void set_info_base(const char* dir) {
    const char* last_slash = strrchr(dir, '/');  // 查找最后一个 '/'，以区分目录与文件名
    if (last_slash) {
        size_t len = last_slash - dir;           // 取最后一个 '/' 之前的目录长度
        if (len >= sizeof(info_base_dir)) len = sizeof(info_base_dir) - 1; // 边界保护
        strncpy(info_base_dir, dir, len);        // 拷贝目录子串
        info_base_dir[len] = '\0';              // NUL 终止
    } else {
        strncpy(info_base_dir, dir, sizeof(info_base_dir) - 1);           // 无 '/'，整体作为目录
        info_base_dir[sizeof(info_base_dir) - 1] = '\0';                  // NUL 终止
    }
}

/*
 * init_display_info_table
 * 作用：加载 display 信息表（格式字符串与完整内容），用于将执行事件映射为可读输出。
 * 输入：无（使用全局 info_base_dir 拼接 "display_info.db"）。
 * 文件格式：每行形如 {0xID,"FORMAT",CONTENT...}
 * 流程：
 *   1) 统计行数以分配表内存；
 *   2) 逐行解析：提取十六进制 ID、双引号中的 FORMAT、其后的 CONTENT；
 *   3) 将解析结果写入 display_info_table，并记录表大小；
 *   4) 失败容错：无法打开文件或单行解析失败时跳过该行。
 * 资源管理：成功后调用者可通过 free_display_info_table 释放内存。
 */
void init_display_info_table() {
    char path[1024];
    snprintf(path, sizeof(path), "%s/display_info.db", info_base_dir);
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "[info_db][display] open failed: %s\n", path);
        return;
    }
    int lines = 0; char ch;
    while(!feof(file)) { ch = fgetc(file); if (ch == '\n') lines++; }
    rewind(file);
    display_info_table = (display_info_entry*)malloc(lines * sizeof(display_info_entry));
    if (!display_info_table) { fclose(file); return; }
    char line[512]; int i = 0;
    while (fgets(line, sizeof(line), file) && i < lines) {
        if (line[0] != '{') continue;
        uint32_t id; if (sscanf(line, "{0x%x,", &id) != 1) continue;
        char* format_start = strchr(line, '"');
        if (!format_start) continue;
        char* format_end = strchr(format_start + 1, '"');
        if (!format_end) continue;
        int format_len = format_end - format_start - 1;
        char* format = (char*)malloc(format_len + 1);
        if (!format) continue;
        strncpy(format, format_start + 1, format_len);
        format[format_len] = '\0';
        char* content_start = strchr(line, ',');
        if (!content_start) { free(format); continue; }
        content_start++; while (*content_start == ' ') content_start++;
        char* content_end = strrchr(line, '}');
        if (!content_end) { free(format); continue; }
        *content_end = '\0';
        size_t content_len = strlen(content_start);
        char* content = (char*)malloc(content_len + 1);
        if (!content) { free(format); continue; }
        strcpy(content, content_start);
        display_info_table[i].id = id;
        display_info_table[i].format = format;
        display_info_table[i].content = content;
        i++;
    }
    display_info_table_size = i;
    fclose(file);
}

/*
 * free_display_info_table
 * 作用：释放 display 信息表的动态内存，避免泄漏。
 * 行为：逐项释放 format/content，再释放表指针并重置大小。
 */
void free_display_info_table() {
    if (display_info_table) {
        for (int i = 0; i < display_info_table_size; i++) {
            if (display_info_table[i].format) free(display_info_table[i].format);
            if (display_info_table[i].content) free(display_info_table[i].content);
        }
        free(display_info_table);
        display_info_table = NULL;
        display_info_table_size = 0;
    }
}

/*
 * get_display_format
 * 作用：通过 ID 查找对应的格式串（只读）。
 * 返回：找到则返回指针；否则返回 NULL。指针生命周期随表而定。
 */
char* get_display_format(uint32_t id) {
    for (int i = 0; i < display_info_table_size; i++) {
        if (display_info_table[i].id == id) return display_info_table[i].format;
    }
    return NULL;
}

/*
 * get_complete_display_string
 * 作用：通过 ID 获取完整的展示字符串（如已渲染好的文本）。
 * 返回：写入并返回静态缓冲区 result；未命中返回 NULL。
 * 注意：返回缓冲区为静态存储，非线程安全，后续调用会覆盖。
 */
char* get_complete_display_string(uint32_t id) {
    static char result[1024];
    result[0] = '\0';
    for (int i = 0; i < display_info_table_size; i++) {
        if (display_info_table[i].id == id && display_info_table[i].content) {
            strncpy(result, display_info_table[i].content, sizeof(result) - 1);
            result[sizeof(result) - 1] = '\0';
            return result;
        }
    }
    return NULL;
}

typedef struct simple_entry {
    uint32_t id;
    char* content;
} simple_entry;

static simple_entry* exec_info_table = NULL; static int exec_info_table_size = 0;
static simple_entry* domain_info_table = NULL; static int domain_info_table_size = 0;

/*
* 作用：通用加载器，读取 exec/domain/timer 等简单信息表。
* 文件格式：每行形如 {0xID,CONTENT...}
* 行为：统计行数分配内存，逐行解析 ID 与 CONTENT 字段，填充 simple_entry 数组。
* 容错：无法打开文件直接返回；单行解析失败跳过；内存分配失败则终止并释放文件句柄。
*/
static void init_simple_table(const char* filename, simple_entry** table, int* table_size) {
    char path[1024]; snprintf(path, sizeof(path), "%s/%s", info_base_dir, filename);
    FILE* file = fopen(path, "r"); if (!file) { fprintf(stderr, "[info_db][%s] open failed: %s\n", filename, path); return; }
    int lines = 0; char ch; while(!feof(file)) { ch = fgetc(file); if (ch == '\n') lines++; }
    rewind(file);
    *table = (simple_entry*)malloc(lines * sizeof(simple_entry)); if (!*table) { fclose(file); return; }
    char line[1024]; int i = 0;
    while (fgets(line, sizeof(line), file) && i < lines) {
        if (line[0] != '{') continue;
        uint32_t id; if (sscanf(line, "{0x%x,", &id) != 1) continue;
        char* start = strchr(line, ','); if (!start) continue; start++; while (*start == ' ') start++;
        char* end = strrchr(line, '}'); if (!end) continue; *end = '\0';
        size_t len = strlen(start); char* content = (char*)malloc(len + 1); if (!content) continue; strcpy(content, start);
        (*table)[i].id = id; (*table)[i].content = content; i++;
    }
    *table_size = i; fclose(file);
}

/*
 * free_simple_table
 * 作用：释放 simple_entry 表（exec/domain/timer）。
 * 行为：逐项释放 content，清空指针并重置大小。
 */
static void free_simple_table(simple_entry** table, int* table_size) {
    if (*table) {
        for (int i = 0; i < *table_size; i++) { if ((*table)[i].content) free((*table)[i].content); }
        free(*table);
        *table = NULL;
        *table_size = 0;
    }
}

/*
 * get_simple_info
 * 作用：在给定 simple_entry 表中按 ID 查找文本内容。
 * 返回：找到返回指针；未找到返回 NULL。生命周期随表而定。
 */
static char* get_simple_info(uint32_t id, simple_entry* table, int table_size) {
    for (int i = 0; i < table_size; i++) if (table[i].id == id) return table[i].content; return NULL;
}

void init_exec_info_table() { init_simple_table("exec_info.db", &exec_info_table, &exec_info_table_size); }
void free_exec_info_table() { free_simple_table(&exec_info_table, &exec_info_table_size); }
char* get_exec_info(uint32_t id) { return get_simple_info(id, exec_info_table, exec_info_table_size); }

void init_domain_info_table() { init_simple_table("domain_info.db", &domain_info_table, &domain_info_table_size); }
void free_domain_info_table() { free_simple_table(&domain_info_table, &domain_info_table_size); }
char* get_domain_info(uint32_t id) { return get_simple_info(id, domain_info_table, domain_info_table_size); }

/*
 * 作用：模拟定时器计数并跳转。
 * 行为：
 *   - 对每个定时器：
 *     - 若使能，增加计数器；
 *     - 若超过阈值，禁用并重置计数器；
 *     - 若有目标 PC，跳转执行。
 */
void timer_tick_and_jump(CPU* cpu) {
    for (int id = 0; id < 2; ++id) {
        if (cpu->timer_enabled[id]) {
            uint64_t c = cpu->timer[id] + 1;
            cpu->timer[id] = c;
            if (c >= cpu->timer_threshold[id]) {
                cpu->timer[id] = 0;
                if (cpu->timer_target_pc[id]) {
                    printf("%sTimer %d reached %" PRIu64 ", jump -> %#.8x%s\n", ANSI_BOLD_GREEN, id, cpu->timer_threshold[id], cpu->timer_target_pc[id], ANSI_RESET);
                    cpu->pc = cpu->timer_target_pc[id];
                } else {
                    printf("%s[cpu][timer] threshold reached (id=%d) but no target%s\n", ANSI_BOLD_RED, id, ANSI_RESET);
                }
            }
        }
    }
}

/*
 * info_db_init_all
 * 作用：统一初始化所有信息表，加载 display/exec/domain/timer 四类 DB，便于后续查询与输出。
 * 输出：打印各表的加载数量，便于排查缺失文件或解析异常。
 */
void info_db_init_all(CPU* cpu) {
    // 初始化显示信息表
    init_display_info_table();

    // 初始化执行信息表
    init_exec_info_table();

    // 初始化域信息表
    init_domain_info_table();

    // 打印数据库加载信息
    print_color(ANSI_BOLD);
    printf("[DB INFO]:\n");
    print_color(ANSI_RESET);
    print_color(ANSI_BOLD_WHITE);
    printf("   DISPLAY:%d EXEC:%d DOMAIN:%d\n", 
           display_info_table_size,
           exec_info_table_size,
           domain_info_table_size);
    print_color(ANSI_RESET);
}
