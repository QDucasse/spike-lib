    .org 0x1000
    .equ toto, 1234
 main:
    auipc  x7, 0
    addi   x7, x7, 250
    ld     x7, 0(x7)
    
# load_pc_relative_value.o:	file format ELF64-riscv
# Disassembly of section .text:
# 0000000000000000 .text:
# 		...
# 0000000000001000 main:
# ;     auipc  x7, 0
#     1000: 97 03 00 00                  	auipc	t2, 0
# 0000000000001004 .Ltmp2:
# ;     addi   x7, x7, 250
#     1004: 93 83 a3 0f                  	addi	t2, t2, 250
# 0000000000001008 .Ltmp3:
# ;     ld     x7, 0(x7)
#     1008: 83 b3 03 00                  	ld	t2, 0(t2)
