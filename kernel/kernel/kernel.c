#include <kernel/interrupt.h>
#include <kernel/keyboard.h>
#include <kernel/tty.h>
#include <kernel/vga.h>
#include <kernel/kprint.h>
#include <stddef.h>
#include <stdint.h>
#include <kernel/multiboot2.h>
#include <kernel/page_frame.h>

#define GDT_SIZE 3

enum {
	_GDT_NULL,
	_KERNEL_CS,
	_KERNEL_DS,
	_KERNEL_TSS
};

#define GDT_NULL (_GDT_NULL << 3)
#define KERNEL_CS (_KERNEL_CS << 3)
#define KERNEL_DS (_KERNEL_DS << 3)
#define KERNEL_TSS (_KERNEL_TSS << 3)

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

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kearly(void) {
	gdt_init();
	int_init();
	vga_init();
	kb_init();
}

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kmain(uint32_t magic, uintptr_t mbi) {
	const char *str = "Welcome to Yornel\n\n\r";

	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		//printf("Invalid magic number: 0x%x\n", (unsigned) magic);
		return;
	}

	if(mbi & 7) {
		//printf("Unaligned mbi: 0x%x\n", addr);
		return;
	}

	uint32_t mb_size = *(uint32_t*) mbi;
	for(struct multiboot_tag* tag = (struct multiboot_tag*) (mbi + 8);
		tag->type != MULTIBOOT_TAG_TYPE_END;
		tag = (struct multiboot_tag*) ((uintptr_t) tag + ((tag->size + 7) & ~7))) {
		switch(tag->type) {
			case MULTIBOOT_TAG_TYPE_MMAP:
				mmap_init((struct multiboot_tag_mmap*) tag);
				break;
		}
	}

	term_init(&kterm, VGA_WIDTH, VGA_HEIGHT, VGA_MEMORY);

	// splash screen
	kprint(str);

	int_enable();

	while(1) {
		asm volatile("hlt");
	}
}

