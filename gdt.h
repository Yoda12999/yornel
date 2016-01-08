#pragma once

#include <stdint.h>

#define GDT_SIZE 3

/* 
 * Define our GDT that we'll use - We know everything upfront, so we just
 * initalize it with the correct settings.
 *
 * This sets up the NULL, entry, and then the kernel's CS and DS segments,
 * which just span all 4GB of memory.
 */
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
};

struct GDT_entry GDT[GDT_SIZE];

void gdt_init(void);

