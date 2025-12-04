#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "include/cpu.h"
#include "include/color.h"

/*
 * read_file
 * 作用：从文件读取数据并加载到CPU的DRAM中。
 * 行为：
 *   - 打开指定的二进制文件；
 *   - 读取文件内容到内存缓冲区；
 *   - 将缓冲区内容复制到CPU的DRAM中；
 *   - 关闭文件。
 * 示例：
 *   read_file(cpu, "program.bin") => 无返回值
 */
void read_file(CPU* cpu, char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("%sUnable to open file %s%s\n", ANSI_RED, filename, ANSI_RESET);
        return;
    }

    if (fseek(file, 0, SEEK_END) != 0) { fclose(file); printf("%sSeek error%s\n", ANSI_RED, ANSI_RESET); return; }
    unsigned long fileLen = ftell(file);
    if (fileLen == (unsigned long)-1) { fclose(file); printf("%sTell error%s\n", ANSI_RED, ANSI_RESET); return; }
    if (fseek(file, 0, SEEK_SET) != 0) { fclose(file); printf("%sSeek error%s\n", ANSI_RED, ANSI_RESET); return; }

    uint8_t *buffer = (uint8_t *)malloc(fileLen);
    if (!buffer) { fclose(file); printf("%sMemory error%s\n", ANSI_RED, ANSI_RESET); return; }

    size_t read_bytes = fread(buffer, 1, fileLen, file);
    fclose(file);
    if (read_bytes != fileLen) { printf("%sRead error (%zu/%lu)%s\n", ANSI_RED, read_bytes, fileLen, ANSI_RESET); free(buffer); return; }

    printf("%s[MEMORY INFO]:%s", ANSI_BOLD, ANSI_RESET);
    for (size_t i = 0; i < read_bytes; i++) {
        if (i % 16 == 0) {
            printf("\n   %4s%.8lx:%s ", ANSI_YELLOW, (unsigned long)i, ANSI_RESET);
        }
        printf("%s%02x%s ", ANSI_BOLD, buffer[i], ANSI_RESET);
    }
    printf("\n");

    size_t copy_bytes = read_bytes;
    if (copy_bytes > (size_t)DRAM_SIZE) {
        printf("%sWarning: file too large, truncating to %zu bytes%s\n", ANSI_YELLOW, (size_t)DRAM_SIZE, ANSI_RESET);
        copy_bytes = (size_t)DRAM_SIZE;
    }
    memcpy(cpu->bus.dram.mem, buffer, copy_bytes);
    printf("\n%sSuccessfully loaded %s (%zu bytes)%s!\n", ANSI_BOLD, filename, copy_bytes, ANSI_RESET);
    free(buffer);
}

/*
 * main
 * 作用：主函数，初始化CPU、读取文件并执行指令。
 * 行为：
 *   - 检查命令行参数，确保提供了一个二进制文件路径；
 *   - 设置信息基础目录，支持直接传递文件路径；
 *   - 初始化CPU、寄存器和程序计数器；
 *   - 从文件读取指令并加载到DRAM中；
 *   - 进入主循环，执行指令直到PC返回0或触发异常；
 *   - 清理资源，包括关闭文件和释放内存。
 * 示例：
 *   emulator program.bin => 无返回值
 */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("%sUsage: tsl_cpu_emulator <filename.bin>%s\n", ANSI_RED, ANSI_RESET);
        exit(1);
    }

    printf("\n%s==================================================================================%s\n", ANSI_BOLD_WHITE, ANSI_RESET);
    printf("%s                          Emulator exec start!                        %s\n", ANSI_BOLD_GREEN, ANSI_RESET);
    printf("%s==================================================================================%s\n", ANSI_BOLD_WHITE, ANSI_RESET);

    // Set info base dir using input path (support passing file path directly)
    set_info_base(argv[1]);

    // Initialize cpu, registers and program counter
    struct CPU cpu;
    cpu_init(&cpu);

    // Read input file
    read_file(&cpu, argv[1]);

    // cpu loop
    while (1) {
        // inst length
        uint8_t inst_length;

        // fetch
        uint64_t inst = cpu_fetch(&cpu, &inst_length);

        // execute
        if (!cpu_execute(&cpu, inst, inst_length))
            break;

        // dump registers
        dump_registers(&cpu);

        if (cpu.pc == 0)
            break;
    }

    // 清理资源
    cpu_cleanup(&cpu);

    printf("\n%s==================================================================================%s\n", ANSI_BOLD_WHITE, ANSI_RESET);
    printf("%s                          Emulator exec successfully!                        %s\n", ANSI_BOLD_GREEN, ANSI_RESET);
    printf("%s==================================================================================%s\n", ANSI_BOLD_WHITE, ANSI_RESET);

    return 0;
}
