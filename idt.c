#include "idt.h"

void inline load_idt(void* base, uint16_t size) {
	struct {
		uint16_t limit;
		uint64_t base;
	} __attribute__((packed)) IDTR = {size - 1, base};
	
	asm("lidt %0": : "m" (IDTR));
	asm("sti");
}

