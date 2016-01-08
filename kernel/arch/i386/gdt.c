#include <kernel/gdt.h>

struct GDT_entry GDT[GDT_SIZE] = {
	[_GDT_NULL] = {0}, // null entry required
	[_KERNEL_CS] = GDT_ENTRY(GDT_EXE | GDT_READ, 0, 0xFFFFFFFF, DPL_KERNEL),
	[_KERNEL_DS] = GDT_ENTRY(GDT_WRITE,	0, 0xFFFFFFFF, DPL_KERNEL)
};

static inline void lgdt(void* base, uint16_t size) {
	struct {
		uint16_t length;
		void*    base;
	} __attribute__((packed)) GDTR = { size - 1, base };

	asm ("lgdt %0" : : "m"(GDTR));  // let the compiler choose an addressing mode
}

void gdt_init(void) {
	lgdt(GDT, sizeof(GDT));
}