000000 0000a500 ldc 0xa5
000001 0000a500 ldc 0xA5
000002 fffffb01 adc -5
000003 00000000 ldc +0
000004 00000000 ldc -0
000005 ffffb90A adj -071
000006 0000160A adj +022
000007          Start:
000007 fffff000 ldc -0x10
000008 0000000B a2sp
000009 00000006 loop: add
00000a 00000e00 ldc loop1
00000b          loop2:
00000b fffff6 SET -10
00000c fffffe0D call loop2
00000d fffff90D call Start
00000e 0000002d loop1: data 45
00000f c4653600 var1: data -1000000000