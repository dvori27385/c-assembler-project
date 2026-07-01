.entry L
.entry UNDEF
.extern E1
L:	lea #7,r1
jsr E1,r2
prn "abc"
NUMS:	.data 5,five,7
add r8,r0
M:	.mat [2][2] 1,2,3
L:	stop
