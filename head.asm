section .text
global keyboard_handler

extern keyboard_handler_main

keyboard_handler:
	push ds
	push es
	push fs
	push gs
	pushad
	call keyboard_handler_main
	popad
	pop gs
	pop fs
	pop es
	pop ds
	iretd						;used because returning from interrupt

