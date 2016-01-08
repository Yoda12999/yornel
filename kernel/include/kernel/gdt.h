#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <stdint.h>

#define GDT_SIZE 3
#define GDT_MEM_LOW 0
#define GDT_MEM_LEN 0xFFFFFFFF

#define GDT_EXE 0x08
#define GDT_READ 0x02
#define GDT_WRITE 0x02

// kernel runs on ring 0
#define DPL_KERNEL 0

enum {
	_GDT_NULL,
	_KERNEL_CS,
	_KERNEL_DS
};

#define GDT_NULL (_GDT_NULL << 3)
#define KERNEL_CS (_KERNEL_CS << 3)
#define KERNEL_DS (_KERNEL_DS << 3)

struct GDT_entry {
	uint16_t limit_low; // low 8 bits of length of memory this descriptor describes
	uint16_t base_low; // low 16 bits of base address
	uint8_t base_mid; // middle 8 bits of base
	
	uint8_t type :4; // flags for type of memory this describes
	uint8_t one :1;
	uint8_t dpl :2; // desciptor privilege level - ring level
	uint8_t present :1; // 1 for valid entry
	
	uint8_t limit_high :4; // top 4 bits of limit
	uint8_t available :1;
	uint8_t zero :1;
	uint8_t op_size :1; // (0) 16 or (1) 32 bit
	uint8_t gran :1; // if set limit is count of 4K blocks indead of bytes
	
	uint8_t base_high;
} __attribute__((packed));

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

struct GDT_entry GDT[GDT_SIZE];

void gdt_init(void);

#endif
