;;kernel.asm
bits 32
section .multiboot
align 4
	;multiboot
	dd 0x1BADB002				;magic
	dd 0x03						;flags
	dd - (0x1BADB002 + 0x03)	;checksum
	
section .text
global start
global read_port
global write_port
global load_gdt
global load_idt
global keyboard_handler

extern kmain
extern keyboard_handler_main

read_port:
	mov edx, [esp + 4]	;copy arguement from stack
	in al, dx			;read to al (b8 of eax) using port dx (b16 of edx)
	ret					;return

write_port:
	mov edx, [esp + 4]
	mov al, [esp + 4 + 4]
	out dx, al
	ret

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

start:
	cli						;block interupts - might be redundant
	mov esp, stack_space	;set stack pointer
	call kmain
	
	cli
.loop:
	hlt						;halt cpu
	jmp .loop

section .bss, nobits
resb 8192
stack_space:

