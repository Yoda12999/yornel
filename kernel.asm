;;kernel.asm
bits 32
section .text
	;multiboot
	dd 0x1BADB002				;magic
	dd 0x00						;flags
	dd - (0x1BADB002 + 0x00)	;checksum

global start
extern kmain

start:
	cli						;block interupts
	mov esp, stack_space	;set stack pointer
	call kmain
	hlt						;halt cpu

section .bss
resb 8192
stack_space:
