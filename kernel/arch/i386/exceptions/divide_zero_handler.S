.section .text
.global divide_zero_handler
.type divide_zero_handler, @function
divide_zero_handler:
	push %gs
	mov 0,%gs
	movw $0x304f, %gs:0xb8000 #"0"

	pushal
	#call handler_main
	popal

	movl $0x204f304f, %gs:0xb8000 #" 0"
	pop %gs
	iret

