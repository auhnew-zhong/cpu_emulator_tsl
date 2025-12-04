1. jmpc（0x0，大小：32bit）：
   [4bit op][4bit fun][4bit src1][4bit src2][8bit addr][8bit rsv]
   功能：
  （1）信号与信号比较（>、<、>=、<=、==、!=）
  （2）信号与立即数比较（>、<、>=、<=、==、!=）
  （3）信号边沿检测（P、N）
   fun 编码：
   0x0==、0x1!=、0x2>、0x3<、0x4>=、0x5<=、0x6上升沿(P)、0x7下降沿(N)
   addr 为相对偏移（有符号 8bit）
2. arith_op（0x1，大小：32bit）：
   [4bit op][4bit fun][4bit dest][4bit src1][4bit src2][12bit rsv]
   功能：
  （1）信号与信号运算（&、|、^）
  （2）信号与立即数运算（&、|、^）
  （3）单个信号运算（~（前端转换为与-1异或）、&、|、^）
  （4）特殊运算符（concat、isunknow()）
   fun 编码：
   0x0与、0x1或、0x2异或、0x3缩位与、0x4缩位或、0x5缩位异或、
   0x6concat、0x7isunknow()、0x8加法(+)、0x9减法(-)、0xA–0xF保留
3. logic_op（0x2，大小：16bit）（实际不需要，后续优化）：
   [4bit op][4bit fun][1bit dest][1bit src1][1bit src2][5bit rsv]，其余位保留
   功能：逻辑运算（&&、||、!）
4. trigger（0x3，大小：8bit）
    [4bit op][4bit rsv]
    功能：仿真暂停
5. trigger_pos（0x4，大小：16bit）
    [4bit op][7bit pos][5bit rsv]
    功能：设定触发采样范围
6. jmp（0x5，大小：16bit）
    [4bit op][8bit addr][4bit rsv]
    功能：跳转指令
    addr 为相对偏移（有符号 8bit）
7. bit_slice（0x6，大小：32bit）
    [4bit op][4bit dest][4bit src][5bit end][5bit start][10bit rsv]
    功能：位截取（end 为高位索引，start 为低位索引）
8. mov（0x7）
    - 立即数形态 MOVI（func=0，大小：64bit）
      [4bit op][1bit func=0][4bit dest][32bit imm][23bit rsv]
      功能：加载立即数到寄存器
    - 寄存器形态 MOV（func=1，大小：16bit）
      [4bit op][1bit func=1][4bit dest][4bit src][3bit rsv]
      功能：寄存器到寄存器赋值
9. ret（0x8，大小：8bit）
    [4bit op][4bit rsv]
    功能：返回
10. bl（0x9，大小：16bit）
    [4bit op][10bit addr][2bit rsv]
    功能：跳转指令
    addr 为相对偏移（有符号 10bit）
11. domain_set（0xa，大小：16bit）
    [4bit op][8bit domain_id][4bit rsv]
    功能：域设定
12. display（0xb，大小：16bit）
    [4bit op][10bit id][2bit rsv]
    功能：打印函数
13. exec（0xc，大小：16bit）
    [4bit op][10bit id][2bit rsv]
    功能：执行函数
14. load（0xd，大小：32bit）
    [4bit op][4bit dest][24bit addr]
    功能：从内存加载数据到寄存器
15. edge_detect（0xe，大小：16bit）
    [4bit op][4bit dst][4bit src][3bit func][1bit rsv]
    功能：信号边沿检测 （上升沿、下降沿等）
16. timer_set（0xf，大小：8bit）
    [4bit op][1bit id][2bit func][1bit rsv]
    功能：id 为计数器 ID；func 为操作（reset/disable/enable）
