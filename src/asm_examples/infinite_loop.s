    .org 0x1000
main:
    add    x5, x6, x7
    add    x5, x6, x7
    j main

; Output of llvm-objdump -S
; infinite_loop.o:	file format ELF64-riscv
; Disassembly of section .text:
; 0000000000000000 .text:
; 		...
; 0000000000001000 main:
;     add    x5, x6, x7
;     1000: b3 02 73 00                  	add	t0, t1, t2
; 0000000000001004 .Ltmp2:
;     add    x5, x6, x7
;     1004: b3 02 73 00                  	add	t0, t1, t2
; 0000000000001008 .Ltmp3:
;     j main
;     1008: 6f 00 00 00                  	j	0
