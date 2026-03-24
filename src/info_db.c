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

typedef struct builtin_info_entry {
    uint32_t id;
    char* type;
    char* content;
} builtin_info_entry;

static builtin_info_entry* builtin_info_table = NULL;
static int builtin_info_table_size = 0;
static char info_base_dir[512] = "examples";

/*
 * set_info_base
 * 作用：设置信息库基目录，用于后续拼接如 "builtin_info.db"、"domain_info.db" 等文件路径。
 * 行为：
 *   - 如果传入的是包含 '/' 的路径（文件或目录），取最后一个 '/' 之前的部分作为目录；
 *   - 如果不包含 '/'，则直接将整个字符串作为目录；
 *   - 进行安全拷贝并在末尾补 '\0'，避免越界。
 * 兼容性：仅处理 POSIX 分隔符 '/'，不对 Windows '\\' 做特殊处理。
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
 * init_builtin_info_table
 * 作用：加载 builtin 信息表，包含 display/exec/force/release/dump/get/set/load 等统一操作。
 * 输入：无（使用全局 info_base_dir 拼接 "builtin_info.db"）。
 * 文件格式：每行形如 {0xID, "TYPE", "CONTENT", [ARGS...]}
 */
void init_builtin_info_table() {
    char path[1024];
    snprintf(path, sizeof(path), "%s/builtin_info.db", info_base_dir);
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "[info_db][builtin] open failed: %s\n", path);
        return;
    }
    int lines = 0; char ch;
    while(!feof(file)) { ch = fgetc(file); if (ch == '\n') lines++; }
    rewind(file);
    builtin_info_table = (builtin_info_entry*)malloc(lines * sizeof(builtin_info_entry));
    if (!builtin_info_table) { fclose(file); return; }
    char line[1024]; int i = 0;
    while (fgets(line, sizeof(line), file) && i < lines) {
        if (line[0] != '{') continue;
        uint32_t id; 
        if (sscanf(line, "{0x%x,", &id) != 1) continue;
        
        // 提取 TYPE
        char* type_start = strchr(line, '"');
        if (!type_start) continue;
        char* type_end = strchr(type_start + 1, '"');
        if (!type_end) continue;
        int type_len = type_end - type_start - 1;
        char* type = (char*)malloc(type_len + 1);
        if (!type) continue;
        strncpy(type, type_start + 1, type_len);
        type[type_len] = '\0';

        // 提取 CONTENT
        char* content_start = strchr(type_end + 1, '"');
        if (!content_start) { free(type); continue; }
        char* content_end = strchr(content_start + 1, '"');
        if (!content_end) { free(type); continue; }
        int content_len = content_end - content_start - 1;
        char* content = (char*)malloc(content_len + 1);
        if (!content) { free(type); continue; }
        strncpy(content, content_start + 1, content_len);
        content[content_len] = '\0';

        builtin_info_table[i].id = id;
        builtin_info_table[i].type = type;
        builtin_info_table[i].content = content;
        i++;
    }
    builtin_info_table_size = i;
    fclose(file);
}

/*
 * free_builtin_info_table
 * 作用：释放 builtin 信息表的动态内存。
 */
void free_builtin_info_table() {
    if (builtin_info_table) {
        for (int i = 0; i < builtin_info_table_size; i++) {
            if (builtin_info_table[i].type) free(builtin_info_table[i].type);
            if (builtin_info_table[i].content) free(builtin_info_table[i].content);
        }
        free(builtin_info_table);
        builtin_info_table = NULL;
        builtin_info_table_size = 0;
    }
}

/*
 * get_builtin_info
 * 作用：通过 ID 获取完整的展示字符串。
 */
char* get_builtin_info(uint32_t id) {
    for (int i = 0; i < builtin_info_table_size; i++) {
        if (builtin_info_table[i].id == id) return builtin_info_table[i].content;
    }
    return NULL;
}

/*
 * get_builtin_type
 * 作用：通过 ID 获取指令类型（display/exec等）。
 */
char* get_builtin_type(uint32_t id) {
    for (int i = 0; i < builtin_info_table_size; i++) {
        if (builtin_info_table[i].id == id) return builtin_info_table[i].type;
    }
    return NULL;
}

typedef struct simple_entry {
    uint32_t id;
    char* content;
} simple_entry;

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
    // 初始化 builtin 信息表
    init_builtin_info_table();

    // 初始化域信息表
    init_domain_info_table();

    // 打印数据库加载信息
    print_color(ANSI_BOLD);
    printf("[DB INFO]:\n");
    print_color(ANSI_RESET);
    print_color(ANSI_BOLD_WHITE);
    printf("   BUILTIN:%d DOMAIN:%d\n", 
           builtin_info_table_size,
           domain_info_table_size);
    print_color(ANSI_RESET);
}
