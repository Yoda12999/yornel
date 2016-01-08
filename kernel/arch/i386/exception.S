.section .text
.global exc_08_handler
.type exc_08_handler, @function
exc_08_handler:
	push %gs
	mov 0x10,%gs
	movw $0x3820,0xb8000
	# '8' in the top-left corner means we're handling
	#  a GPF exception right ATM.

	# your 'normal' handler comes here
	pushal
	push %ds
	push %es
	mov %ax,0x0010
	mov %ax,%ds
	mov %ax,%es
 
	#call dfExcHandler

	pop %es
	pop %ds
	popal
	
	movl $0x2020382d,0xb8000
	# the '8' moved one character to the right, letting
	# us know that the exception has been handled properly
	# and that normal operations continues.
	pop %gs
	iret

.global exc_0d_handler
.type exc_0d_handler, @function
exc_0d_handler:
	push %gs
	mov 0x10,%gs
	movw $0x4420,0xb8000
	# D in the top-left corner means we're handling
	#  a GPF exception right ATM.

	# your 'normal' handler comes here
	pushal
	push %ds
	push %es
	mov %ax,0x0010
	mov %ax,%ds
	mov %ax,%es
 
	#call gpfExcHandler

	pop %es
	pop %ds
	popal
	
	movl $0x2020442d,0xb8000
	# the 'D' moved one character to the right, letting
	# us know that the exception has been handled properly
	# and that normal operations continues.
	pop %gs
	iret
