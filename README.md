# TSL 软核 CPU 仿真器（C 实现）

该项目为 TSL 软核 CPU 的纯 C 仿真器，实现了指令获取、解码、执行、寄存器与内存模型、信息 DB 关联、彩色输出控制等，便于在主机上进行快速验证与调试。

## 功能概览
- 指令支持：1/2/4/8 字节长度的核心指令（`trigger/ret/timer_set/jmpc/arith_op/bit_slice/mov/movi/jmp/bl/domain_set/display/exec/load/edge_detect`）
- 寄存器与 PC：`R0-R15`（`R0` 只读为 0）、`PC` 程序计数器
- 总线与内存：`BUS` 挂载 `DRAM`，大小为 `DRAM_SIZE`（见 `include/dram.h`）
- 信息 DB：统一从 `info_base_dir` 加载 `display/exec/domain/timer` 信息，用于源级提示与日志
- 彩色输出：通过 `set_ansi_color_enabled(int)` 控制 ANSI 颜色输出，避免日志转存时出现转义字符

## 目录结构（关键）
- `src/`：核心实现（`cpu.c`、`color.c`、`info_db.c` 等）
- `include/`：头文件与常量（`cpu.h`、`opcodes.h`、`bus.h`、`dram.h`、`color.h`、`info_db.h`）
- `scripts/`：辅助脚本（`BinToAsm.py`、`BinToMem.py`）

### 脚本说明
- `BinToAsm.py`
  - 作用：将二进制指令文件反汇编为可读的助记符序列，便于快速检查编码是否符合期望
  - 变长识别：按首字节 `opcode` 判断长度；对 `mov` 变体通过检查 `func` 位与高位字节判别 `2B/8B`
  - 用法：
    ```bash
    python scripts/BinToAsm.py <input_binary>
    ```
  - 输出：逐条打印“偏移: 原始字节（大端） 助记符”；未知指令以 `UNKNOWN` 标注并前进 1 字节

- `BinToMem.py`
  - 作用：将二进制指令文件转换为“内存镜像”文本，每行一条指令的十六进制（大端），便于仿真加载或硬件初始化
  - 变长识别：同 `BinToAsm.py`；对 `mov` 采用 `func` 位与高位字节为零的条件判定短指令
  - 用法：
    ```bash
    python scripts/BinToMem.py <input_binary> <output_memory_txt>
    ```
  - 输出：文本文件，每行一个指令十六进制串；未知或残缺指令将跳过并继续扫描

提示：两者都采用大端字节拼接，输出便于与 `cpu_fetch` 的读取一致性校验；如需对尾部零填充进行截断，脚本已在长度识别与行生成处进行处理。
- `doc/`：架构与调试器设计文档

## 构建与运行
```bash
make
./emulator <binary.bin>
```

示例：
```bash
./emulator tests/test_first_version.bin
./emulator tests/test_second_version.bin
```

## 使用说明
- DB 基目录：仿真器会根据传入的二进制路径，自动调用 `set_info_base(argv[1])` 提取其目录作为 DB 基目录。
  - 例如：`/path/examples/test.bin` 会将 `info_base_dir` 设为 `/path/examples`，从该目录加载 `*.db` 文件
- 颜色输出：需要禁用 ANSI 颜色时，可在程序入口调用 `set_ansi_color_enabled(0)`
- 文件加载：`main.c` 中 `read_file` 具备长度校验与 DRAM 边界截断，超出将提示并截断拷贝

## 日志前缀规范
- DB 加载：`[info_db][display] open failed: <path>`、`[info_db][<filename>] open failed: <path>`
- 取指与解码：`[cpu][fetch] ...`、`[cpu][decode] ...`、`[cpu][inst_size] ...`
- 信号查询：`[cpu][signal] not found: 0x...`
- DB 未命中：`[cpu][db] display/domain/timer not found: ...`

## 文档
- 架构与框架：`doc/TSL软核CPU代码框架设计.md`
- 优化与改进：`doc/TSL软核CPU优化与改进建议.md`
- 调试器设计：`doc/TSL软核CPU调试器设计.md`

## 后续工作
- 完善 MOV 判定与位域解析，减少误判
- 增加命令行选项（静默/详细模式），控制 Dump 与日志
- 引入最小测试用例集，覆盖常用指令与 DB 查询路径
