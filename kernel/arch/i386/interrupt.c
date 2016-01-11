#include <kernel/interrupt.h>

#include <stddef.h>
#include "idt.h"
#include <kernel/io.h>
#include "exceptions/exception.h"

inline void int_enable(void) {
	asm("sti");
}

inline void int_disable(void) {
	asm("cli");
}

bool add_interrupt(uint8_t id, void (*handler)(void), uint8_t type_attr) {
	//TODO: check if already filled
	uintptr_t handler_address = (uintptr_t) handler;
	IDT[id].offset_low = handler_address & 0xffff;
	IDT[id].selector = CS_OFFSET;
	IDT[id].zero = 0;
	IDT[id].type_attr = type_attr; // INTERRUPT_GATE
	IDT[id].offset_high = (handler_address & 0xffff0000) >> 16;
	
	return true;
}

bool rm_interrupt(uint8_t id) {
	//TODO: check if already empty
	IDT[id].offset_low = 0;
	IDT[id].selector = 0;
	IDT[id].type_attr = 0;
	IDT[id].offset_high = 0;
	
	return true;
}

void int_init(void) {
	idt_init();
}

inline void EOI(void) {
	outb(PIC1_CMD, 0x20);
}

