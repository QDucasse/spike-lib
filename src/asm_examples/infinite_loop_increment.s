    .org 0x1000
main:
    addi   x6, x6, 1
    j      main


# infinite_loop_increment.o:	file format ELF64-riscv
# Disassembly of section .text:
# 0000000000000000 .text:
# 		...
# 0000000000001000 main:
# ;     addi   x6, x6, 1
#     1000: 13 03 13 00                  	addi	t1, t1, 1
# 
# 0000000000001004 .Ltmp2:
# ;     j      main
#     1004: 6f 00 00 00                  	j	0