    .org 0x1000
main: 
    li x7, 0x97
    jr x7

# The jump instruction cuts the least significant bit
# 0x97 will be brought to 0x96 that is not aligned
# However, 0x99 would be transformed to 0x98 that is aligned

# Output of llvm-obj -S
# call_to_misaligned.o:	file format ELF64-riscv
# Disassembly of section .text:
# 0000000000000000 .text:
# 		...
# 0000000000001000 main:
# ;     li x7, 0x97
#     1000: 93 03 70 09                  	addi	t2, zero, 151
# 0000000000001004 .Ltmp2:
# ;     jr x7
#     1004: 67 80 03 00                  	jr	t2
