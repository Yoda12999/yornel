/*
 *	kernel.c
 */

#if !defined(__cplusplus)
#include <stdbool.h>
#endif
#include <stddef.h>
#include <stdint.h>
 
#include "keyboard_map.h"
#include "io.h"

#define LINES 25
#define COLUMNS 80
#define SCREEN_SIZE (LINES * COLUMNS * 2)

#define GDT_SIZE 3
#define GDT_MEM_LOW 0
#define GDT_MEM_LEN 0xFFFFFFFF

#define GDT_EXE 0x08
#define GDT_READ 0x02
#define GDT_WRITE 0x02

// kernel runs on ring 0
#define DPL_KERNEL 0

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

#define IDT_SIZE 256
#define KERNEL_CS_OFFSET KERNEL_CS

#define PIC1_CMD 0x20
#define PIC1_DAT 0x21
#define PIC2_CMD 0xA0
#define PIC2_DAT 0xA1

#define KEYBOARD_STATUS 0x64
#define KEYBOARD_DATA 0x60

extern void keyboard_handler(void);

uint32_t cursor_loc = 0;
uint8_t tab_size = 4;

struct Mod_keys {
	uint8_t shift :2;		//set with either shift key
	uint8_t r_shift :1;	//set with only the right shift key
	uint8_t ctrl :2;
	uint8_t r_ctrl :1;
	uint8_t alt :2;
	uint8_t r_alt :1;
	uint8_t meta :2;
	uint8_t r_meta :1;
	uint8_t clck :1;
	uint8_t nlck :1;
	uint8_t slck :1;
} mod_keys;

uint8_t *vidptr = (uint8_t*) 0xb8000;	// video mem starts here. Maybe make 16 bit addressed instead...

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

struct GDT_ptr {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

extern void load_gdt(struct GDT_ptr *gdt_ptr);

void gdt_init(void) {
	struct GDT_ptr gdt_ptr;
	
	gdt_ptr.limit = sizeof(GDT);
	gdt_ptr.base = (uint64_t) GDT;
	load_gdt(&gdt_ptr);
}

// IDT - Interupt Descriptor Table - matches interupt number to interupt handler address
struct IDT_entry {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_high;
} __attribute__((packed)); // makes sure that gcc doesn't add padding for any reason

struct IDT_entry IDT[IDT_SIZE];

struct IDT_ptr {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

extern void load_idt(struct IDT_ptr *idt_ptr);

void idt_init(void) {
	uint64_t keyboard_address;
	struct IDT_ptr idt_ptr;
	
	// populate IDT entry for keyboard interrupt
	keyboard_address = (uint64_t) keyboard_handler;
	IDT[0x21].offset_low = keyboard_address & 0xffff;
	IDT[0x21].selector = 0x08; // KERNEL_CODE_SEGMENT_OFFSET
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = 0x8e; // INTERRUPT_GATE
	IDT[0x21].offset_high = (keyboard_address & 0xffff0000) >> 16;
	
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
	
	// fill IDT descriptor
	idt_ptr.limit = sizeof(IDT);
	idt_ptr.base = (uint64_t) IDT;
	load_idt(&idt_ptr);
}

void kprint_char(char c) {
	switch(c) {
		case '\n':
			cursor_loc += COLUMNS;
			return;
		case '\r':
			cursor_loc -= cursor_loc%COLUMNS;
			return;
		case '\t':
			cursor_loc += tab_size;
			return;
		default:
			vidptr[cursor_loc*2] = c;
			vidptr[cursor_loc*2 + 1] = 0x07;
			cursor_loc++;
	}
}

void kprint(const char *str) {
	size_t i = 0;

	while(str[i] != '\0') {
		kprint_char(str[i]);
		i++;
	}
}

void kprint_num(int32_t num, uint32_t radix) {
	if(num < 0) {
		num = -num;
		kprint_char('-');
	}
	
	if(num/radix) {
		kprint_num(num/radix, radix);
	}
	
	if(num%radix <= 9) {
		kprint_char(num%radix + '0');
	} else {
		kprint_char(num%radix + 'a' - 10);
	}
}

void kb_init(void) {
	// 0xFD = 11111101 - IRQ1 only enabled, keyboard
	outb(0x21, 0xFD);
}

void keyboard_handler_main(void) {
	uint8_t status;
	uint8_t scancode;
	uint8_t keycode;
	
	// write EOI to allow more interrupts
	outb(PIC1_CMD, 0x20);
	
	status = inb(KEYBOARD_STATUS);
	// lowest bit set if buffer empty
	while(status & 0x01) {
		scancode = inb(KEYBOARD_DATA);
		
		if(scancode >= 0x80) {
			// released key
			switch(scancode - 0x80) {
				case RSFT_CODE:
					mod_keys.r_shift = 0;
				case LSFT_CODE:
					mod_keys.shift -= 1;
					break;
			}
			return;
		}
		
		switch(scancode) {
			case 0xe0:
			case 0xe1:
				// escaped keycodes
				status = inb(KEYBOARD_STATUS);
				if(status & 0x01) {
					scancode = inb(KEYBOARD_DATA);
					if(scancode >= 0x80) {
						// released key
					}
				}
				break;
			case RSFT_CODE:
				mod_keys.r_shift = 1;
			case LSFT_CODE:
				mod_keys.shift += 1;
				break;
			case CLCK_CODE:
				mod_keys.clck = !mod_keys.clck;
			case NLCK_CODE:
				mod_keys.nlck = !mod_keys.nlck;
			case SLCK_CODE:
				mod_keys.slck = !mod_keys.slck;
			default:
				keycode = keyboard_map[scancode];
				
				if(mod_keys.shift && scancode >= 2 && scancode < 54) {
					keycode = shift_map[scancode - 2];
					if(mod_keys.clck && keycode >= 'A' && keycode <= 'Z') {
						keycode += 'a' - 'A';
					}
				} else if(mod_keys.clck && keycode >= 'a' && keycode <= 'z') {
					keycode -= 'a' - 'A';
				}
				
				if(keycode == 0) {
					return;
				} else if(keycode == '\n') {
					kprint_char('\r');
				}
				kprint_char(keycode);
		}
		
		/*
		kprint_num(scancode, 16);
		kprint_char(' ');
		*/
		
		status = inb(KEYBOARD_STATUS);
	}
}

void clear_screen(void) {
	size_t i = 0;
	
	while(i < 80 * 25 * 2) {
		vidptr[i] = ' ';
		// attribute byte - light grey on black
		vidptr[i + 1] = 0x07;
		i = i + 2;
	}
}

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kearly(void) {
}

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kmain(void) {
	const char *str = "Welcome to Yornel\n\n\r";
	
	clear_screen();
	
	// splash screen
	kprint(str);
	
	// init
	gdt_init();
	idt_init();
	kb_init();
	
	while(1) {
		asm volatile("hlt");
	}
}

