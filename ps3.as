.entry LOOP2
.entry TXT
.entry SIZE
.extern OUT1
.extern OUT2
MAIN2:	red r3
	lea TXT,r2
	cmp r2,SIZE
LOOP2:	add r2,r3
	jsr OUT2
	bne OUT1
	clr r4
	mov MAT2[r3][r2],r4
	prn r4
	rts
END2:	stop
TXT:	.string "xyz"
SIZE:	.data 10,-1,8
MAT2:	.mat [3][2] 1,0,2,0,3,0
