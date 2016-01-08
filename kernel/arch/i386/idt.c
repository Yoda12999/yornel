#include <kernel/idt.h>
#include "interupt.h"
#include "io.h"

struct IDT_entry IDT[IDT_SIZE];

static inline void lidt(void* base, uint16_t size) {
	struct {
		uint16_t length;
		void*    base;
	} __attribute__((packed)) IDTR = { size - 1, base };

	asm ("lidt %0" : : "m"(IDTR));  // let the compiler choose an addressing mode
}

void idt_init(void) {
	uint64_t handler_address;
	
	// general protection fault
	handler_address = (uint64_t) exc_0d_handler;
	IDT[0x0d].offset_low = handler_address & 0xffff;
	IDT[0x0d].selector = 0x08; // KERNEL_CODE_SEGMENT_OFFSET
	IDT[0x0d].zero = 0;
	IDT[0x0d].type_attr = 0x8e; // INTERRUPT_GATE
	IDT[0x0d].offset_high = (handler_address & 0xffff0000) >> 16;
	
	// populate IDT entry for keyboard interrupt
	handler_address = (uint64_t) keyboard_handler;
	IDT[0x21].offset_low = handler_address & 0xffff;
	IDT[0x21].selector = 0x08; // KERNEL_CODE_SEGMENT_OFFSET
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = 0x8e; // INTERRUPT_GATE
	IDT[0x21].offset_high = (handler_address & 0xffff0000) >> 16;
	
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
	
	lidt(IDT, sizeof(IDT));
}

void interrupt_enable(void) {
	asm ("sti");
}