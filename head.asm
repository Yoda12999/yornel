section .text
global load_gdt
global load_idt
global keyboard_handler

extern keyboard_handler_main

load_gdt:
	mov edx, [esp + 4]
	lgdt [edx]
	ret
	
load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	sti					;turn on interrupts
	ret

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

