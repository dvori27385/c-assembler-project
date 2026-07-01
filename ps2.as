.entry START
.entry COUNT
.entry MSG
.extern EXT1
.extern EXT2
START:	lea MSG,r0
	cmp r0,#0
	jsr EXT1
	mov M[r2][r3],r1
	add r1,COUNT
LOOP:	bne EXT2
	inc r1
	dec r2
	prn r1
	jmp NEXT
MSG:	.string "hello"
COUNT:	.data 3,7,-2
M:	.mat [2][2] 5,6,7,8
NEXT:	sub r1,r2
END:	stop
