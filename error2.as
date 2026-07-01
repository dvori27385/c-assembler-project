.entry A
.extern B
1BAD:	add r1,r2
A:	.mat [2][3] 1,2,3,4,5
mov r3,#5
.datas 7,8,9
jmp [A]
stop
