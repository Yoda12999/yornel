.section .text
.global keyboard_handler
.type keyboard_handler, @function
keyboard_handler:
	push %ds
	push %es
	push %fs
	push %gs
	pushal
	call keyboard_handler_main
	popal
	pop %gs
	pop %fs
	pop %es
	pop %ds
	iret

