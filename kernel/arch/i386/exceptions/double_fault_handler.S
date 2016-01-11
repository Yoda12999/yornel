.section .text
.global double_fault_handler
.type double_fault_handler, @function
double_fault_handler:
	push %gs
	mov 0,%gs
	movw $0x384f, %gs:0xb8000 #"8"

	pushal
	#call handler_main
	popal

	movl $0x204f384f, %gs:0xb8000 #" 8"
	pop %gs
	iret

