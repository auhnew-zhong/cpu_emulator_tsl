
examples/counter_timer_domain.o:	file format elf32-tsl

Disassembly of section .text:

00000000 <main>:
       0: 4b 40        	trigger_pos	90
       2: 70 10 01 cc cc 00 00 00      	mov %r0, 268553420
       a: d1 00 00 04  	load %r1, 4
       e: 01 10 01 00  	jmpc 1, %r1, %r0, 1
      12: 30           	trigger
      13: 90 04        	bl	1
      15: 80           	ret

00000016 <inst1>:
      16: a0 00        	domain_set 0
      18: d0 00 10 08  	load %r0, 4104
      1c: 61 02 94 00  	bit_slice %r1, %r0, 5, 5
      20: e1 12        	edge_detect %r1, %r1, 1
      22: 72 00 00 00 01 00 00 00      	mov %r2, 1
      2a: 01 12 04 00  	jmpc 1, %r1, %r2, 4
      2e: f0           	timer_set 0, 0
      2f: b0 04        	display	1
      31: 80           	ret
      32: d1 00 10 00  	load %r1, 4096
      36: d2 00 00 04  	load %r2, 4
      3a: 18 88 40 00  	bit_op %r2, %r2, %r1, 2
      3e: 62 20 0c 00  	bit_slice %r2, %r2, 0, 3
      42: 73 00 00 00 0d 00 00 00      	mov %r3, 13
      4a: 03 23 33 00  	jmpc 3, %r2, %r3, 51
      4e: f2           	timer_set 1, 0
      4f: b0 08        	display	2
      51: e0 00        	edge_detect %r0, %r0, 0
      53: 72 00 00 00 01 00 00 00      	mov %r2, 1
      5b: 00 02 02 00  	jmpc 0, %r0, %r2, 2
      5f: 50 20        	jmp	2
      61: 30           	trigger
      62: 80           	ret
      63: d0 00 10 04  	load %r0, 4100
      67: 10 04 00 00  	bit_op %r0, %r1, %r0, 0
      6b: 60 00 84 00  	bit_slice %r0, %r0, 1, 1
      6f: e0 02        	edge_detect %r0, %r0, 1
      71: 71 00 00 00 01 00 00 00      	mov %r1, 1
      79: 01 01 02 00  	jmpc 1, %r0, %r1, 2
      7d: 30           	trigger
      7e: 80           	ret
      7f: b0 0c        	display	3
      81: 80           	ret
