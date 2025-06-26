1. jmpc（0x0）：
   [4bit op][4bit fun][4bit src1][4bit src2][8bit addr][8bit rsv]
   功能：
  （1）信号与信号比较（>、<、>=、<=、==、!=）
  （2）信号与立即数比较（>、<、>=、<=、==、!=）
  （3）信号边沿检测（P、N）
2. bit_op（0x1）： 
   [4bit op][2bit fun][4bit dest][4bit src1][4bit src2][14bit rsv]
   [4bit op][2bit fun][4bit dest][4bit src1][2bit rsv]
    功能：
  （1）信号与信号运算（&、|、^）
  （2）信号与立即数运算（&、|、^）
  （3）单个信号运算（！、~、&、|）
3. logic_op（0x2）：
   [4bit op][4bit fun][1bit dest][1bit src1][1bit src2]
   功能：逻辑运算（&&、||）
4. trigger (0x3)
    [4bit op][4bit rsv]
    功能：仿真暂停
5. trigger_pos（0x4）
    [4bit op][7bit pos][5bit rsv]
    功能：设定触发采样范围
6. jmp指令（0x5）
    [4bit op][8bit addr][4bit rsv]
    功能：跳转指令
7. bit_slice（0x6）
    [4bit op][4bit dest][4bit src][5bit start][5bit end][10bit rsv]
    功能：位截取
8. mov指令（0x7）
    [4bit op][4bit dest][32bit imm][24bit rsv]
    功能：加载立即数到寄存器
9. ret指令（0x8）
    [4bit op][4bit rsv]
    功能：返回
10. bl指令（0x9）
    [4bit op][10bit addr][2bit rsv]
    功能：跳转指令
11. nop指令（0xa）
    [4bit op]
    功能：空操作
12. display指令（0xb）
    [4bit op][10bit id][2bit rsv]
    功能：打印函数
13. exec指令 （0xc）
    [4bit op][10bit addr][10bit addr1][10bit len]
    功能：执行系统函数
14. load指令 （0xd）
    [4bit op][4bit dest][24bit addr]
    功能：从内存加载数据到寄存器