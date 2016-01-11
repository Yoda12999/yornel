.section .text
.global breakpoint_handler
.type breakpoint_handler, @function
breakpoint_handler:
	push %gs
	mov 0,%gs
	movw $0x334f, %gs:0xb8000 #"3"

	pushal
	#call handler_main
	popal

	movl $0x204f334f, %gs:0xb8000 #" 3"
	pop %gs
	iret

