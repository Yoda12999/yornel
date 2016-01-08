#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/kb.h>

#if defined(__cplusplus)
extern "C"
#endif
void kernel_early(void) {
	gdt_init();
	idt_init();
	terminal_initialize();
	kb_init();
}

#if defined(__cplusplus)
extern "C"
#endif
void kernel_main(void) {
	printf("Welcome to Yornel!\n");
	interrupt_enable();
	
	while(1) {
		//keyboard_handler_main();
		asm volatile("hlt");
	}
}
