#include "gdt.h"

void load_gdt(void* base, uint16_t size) {
	struct {
		uint16_t limit;
		uint64_t base;
	} __attribute__((packed)) GDTR = {size - 1, base};
	
	asm("lgdt %0": : "m" (GDTR));
}

