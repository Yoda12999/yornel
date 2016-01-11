#include "idt.h"

#include <stddef.h>
#include <kernel/io.h>

static void load_idt(void* base, size_t size) {
	struct {
		uint16_t limit;
		uintptr_t base;
	} __attribute__((packed)) IDTR = {(uint16_t) size - 1, (uintptr_t) base};
	
	asm("lidt %0": : "m" (IDTR));
}

void idt_init(void) {
	// ICW1 - begin init - sets to wait for three more bytes on data ports
	outb(PIC1_CMD, 0x11);
	outb(PIC2_CMD, 0x11);
	
	// ICW2 - remap offset of IDT because first 32 interrupts are for CPU
	outb(PIC1_DAT, 0x20);
	outb(PIC2_DAT, 0x28);
	
	// ICW3 - set everything with no slaves
	outb(PIC1_DAT, 0x00);
	outb(PIC2_DAT, 0x00);
	
	// ICW4 - enviroment info - 80x86 mode
	outb(PIC1_DAT, 0x01);
	outb(PIC2_DAT, 0x01);
	
	// mask interupts
	outb(PIC1_DAT, 0xff);
	outb(PIC1_DAT, 0xff);
	
	load_idt(IDT, sizeof(IDT));
}

