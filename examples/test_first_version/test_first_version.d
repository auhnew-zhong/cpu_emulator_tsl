
examples/test_first_version.o:	file format elf32-tsl

Disassembly of section .text:

00000000 <main>:
       0: 72 80 00 00 05 00 00 00      	mov %r5, 10
       8: 73 00 00 00 06 00 00 00      	mov %r6, 12
      10: 4b 40        	trigger_pos	90
      12: 70 08 00 e6 66 00 00 00      	mov %r0, 268553420
      1a: d1 00 00 04  	load %r1, 4
      1e: 01 10 01 00  	jmpc 1, %r1, %r0, 1
      22: 30           	trigger
      23: 90 04        	bl	1
      25: 80           	ret

00000026 <inst1>:
      26: a0 00        	domain_set 0
      28: d1 00 10 08  	load %r1, 4104
      2c: 60 12 94 00  	bit_slice %r0, %r1, 5, 5
      30: e2 02        	edge_detect %r2, %r0, 1
      32: 70 00 00 00 00 80 00 00      	mov %r0, 1
      3a: 01 20 08 00  	jmpc 1, %r2, %r0, 8
      3e: 13 05 00 00  	arith_op %r0, %r5, %r0, 3
      42: f0           	timer_set 0, 0
      43: b0 00        	display	0
      45: 80           	ret
      46: a0 00        	domain_set 0
      48: 71 7f ff ff ff 80 00 00      	mov %r2, 4294967295
      50: 13 55 20 00  	arith_op %r5, %r5, %r2, 3
      54: d2 00 10 00  	load %r2, 4096
      58: d3 00 00 04  	load %r3, 4
      5c: 12 33 20 00  	arith_op %r3, %r3, %r2, 2
      60: 63 30 0c 00  	bit_slice %r3, %r3, 0, 3
      64: 72 00 00 00 06 80 00 00      	mov %r4, 13
      6c: 03 34 15 00  	jmpc 3, %r3, %r4, 21
      70: f2           	timer_set 1, 0
      71: b0 04        	display	1
      73: 72 80 00 00 00 00 00 00      	mov %r5, 0
      7b: e1 10        	edge_detect %r1, %r1, 0
      7d: 00 10 02 00  	jmpc 0, %r1, %r0, 2
      81: 51 00        	jmp	16
      83: 30           	trigger
      84: 80           	ret
      85: 70 00 00 00 07 80 00 00      	mov %r0, 15
      8d: 01 60 01 00  	jmpc 1, %r6, %r0, 1
      91: 30           	trigger
      92: 80           	ret
      93: d1 00 10 04  	load %r1, 4100
      97: 10 12 10 00  	arith_op %r1, %r2, %r1, 0
      9b: 61 10 84 00  	bit_slice %r1, %r1, 1, 1
      9f: e1 12        	edge_detect %r1, %r1, 1
      a1: 01 10 02 00  	jmpc 1, %r1, %r0, 2
      a5: 30           	trigger
      a6: 80           	ret
      a7: b0 08        	display	2
      a9: 80           	ret
