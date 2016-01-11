.section .text
.global debug_handler
.type debug_handler, @function
debug_handler:
	push %gs
	mov 0,%gs
	movw $0x314f, %gs:0xb8000 #"1"

	pushal
	#call handler_main
	popal

	movl $0x204f314f, %gs:0xb8000 #" 1"
	pop %gs
	iret

