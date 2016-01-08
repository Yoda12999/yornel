.section .text
.global keyboard_handler
.type keyboard_handler, @function
keyboard_handler:
	pushal
	cld
	call keyboard_handler_main
	popal
	iretl
