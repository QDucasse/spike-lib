    .org 0x1000
main:
    li x7, 0x99999900
    sd x5, 0(x7)

# End of objdump -s
# ...
# 0fc0 00000000 00000000 00000000 00000000  ................
# 0fd0 00000000 00000000 00000000 00000000  ................
# 0fe0 00000000 00000000 00000000 00000000  ................
# 0ff0 00000000 00000000 00000000 00000000  ................
# 1000 b7d30400 9b83d3cc b6039383 039023b0  ..............#.
# 1010 5300                                 S.              

