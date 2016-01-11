.section .text
.global not_available_handler
.type not_available_handler, @function
not_available_handler:
	push %gs
	mov 0,%gs
	movw $0x374f, %gs:0xb8000 #"7"

	pushal
	#call handler_main
	popal

	movl $0x204f374f, %gs:0xb8000 #" 7"
	iret

