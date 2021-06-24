    .org 0x1000
main:
    li x7, 0x99999900
    sd x5, 0(x7)

# Output of llvm-objdump -S
# store_unmapped.o:	file format ELF64-riscv
# Disassembly of section .text:
# 0000000000000000 .text:
# 		...
# 0000000000001000 main:
#     li x7, 0x99999900
#     1000: b7 d3 04 00                  	lui	t2, 77
#     1004: 9b 83 d3 cc                  	addiw	t2, t2, -819
#     1008: 93 93 d3 00                  	slli	t2, t2, 13
#     100c: 93 83 03 90                  	addi	t2, t2, -1792
# 0000000000001010 .Ltmp2:
#     sd x5, 0(x7)
#     1010: 23 b0 53 00                  	sd	t0, 0(t2)