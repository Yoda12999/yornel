#include "gdt.h"

#include <stddef.h>

#define GDT_MEM_LOW 0
#define GDT_MEM_LEN 0xFFFFFFFF

#define GDT_EXE 0x08
#define GDT_READ 0x02
#define GDT_WRITE 0x02

// kernel runs on ring 0
#define DPL_KERNEL 0

#define GDT_ENTRY(gdt_type, gdt_base, gdt_limit, gdt_dpl) {	\
	.limit_low	= (((gdt_limit) >> 12) & 0xFFFF),			\
	.base_low	= ((gdt_base) & 0xFFFF),					\
	.base_mid	= (((gdt_base) >> 16) & 0xFF),				\
	.type		= gdt_type,									\
	.one		= 1,										\
	.dpl		= gdt_dpl,									\
	.present	= 1,										\
	.limit_high	= ((gdt_limit) >> 28),						\
	.available	= 0,										\
	.zero		= 0,										\
	.op_size	= 1,										\
	.gran		= 1,										\
	.base_high	= (((gdt_base) >> 24) & 0xFF),				\
}

struct GDT_entry GDT[GDT_SIZE] = {
	[_GDT_NULL] = {0}, // null entry required
	[_KERNEL_CS] = GDT_ENTRY(GDT_EXE | GDT_READ, 0, 0xFFFFFFFF, DPL_KERNEL),
	[_KERNEL_DS] = GDT_ENTRY(GDT_WRITE,	0, 0xFFFFFFFF, DPL_KERNEL)
};

static inline void load_gdt(void* base, size_t size) {
	struct {
		uint16_t limit;
		uintptr_t base;
	} __attribute__((packed)) GDTR = {(uint16_t) size - 1, (uintptr_t) base};
	
	asm("lgdt %0": : "m" (GDTR));
}

void gdt_init(void) {
	load_gdt(GDT, sizeof(GDT));
}

