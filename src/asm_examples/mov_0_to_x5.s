    .org 0x1000
main:
    addi x5, x0, 0


; mov_0_to_x5.o:	file format ELF64-riscv
; Disassembly of section .text:
; 0000000000000000 .text:
;		...
; 0000000000001000 main:
;     addi x5, x0, 0
;    1000: 93 02 00 00                  	mv	t0, zero
