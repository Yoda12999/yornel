.section .text
.global overflow_handler
.type overflow_handler, @function
overflow_handler:
	push %gs
	mov 0,%gs
	movw $0x344f, %gs:0xb8000 #"4"

	pushal
	#call handler_main
	popal

	movl $0x204f344f, %gs:0xb8000 #" 4"
	pop %gs
	iret

