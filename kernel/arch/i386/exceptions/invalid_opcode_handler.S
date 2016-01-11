.section .text
.global invalid_opcode_handler
.type invalid_opcode_handler, @function
invalid_opcode_handler:
	push %gs
	mov 0,%gs
	movw $0x364f, %gs:0xb8000 #"6"

	pushal
	#call handler_main
	popal

	movl $0x204f364f, %gs:0xb8000 #" 6"
	iret

