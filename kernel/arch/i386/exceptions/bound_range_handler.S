.section .text
.global bound_range_handler
.type bound_range_handler, @function
bound_range_handler:
	push %gs
	mov 0,%gs
	movw $0x354f, %gs:0xb8000 #"5"

	pushal
	#call handler_main
	popal

	movl $0x204f354f, %gs:0xb8000 #" 5"
	iret

