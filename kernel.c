/*
 *	kernel.c
 */

#include "keyboard_map.h"

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

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);

unsigned int cursor_loc = 0;
char *vidptr = (char*) 0xb8000;	// video mem starts here

struct GDT_entry {
	unsigned short limit_low; // low 8 bits of length of memory this descriptor describes
	unsigned short base_low; // low 16 bits of base address
	unsigned char base_mid; // middle 8 bits of base
	
	unsigned char type :4; // flags for type of memory this describes
	unsigned char one :1;
	unsigned char dpl :2; // desciptor privilege level - ring level
	unsigned char present :1; // 1 for valid entry
	
	unsigned char limit_high :4; // top 4 bits of limit
	unsigned char available :1;
	unsigned char zero :1;
	unsigned char op_size :1; // (0) 16 or (1) 32 bit
	unsigned char gran :1; // if set limit is count of 4K blocks indead of bytes
	
	unsigned char base_high;
};

#define GDT_ENTRY(gdt_type, gdt_base, gdt_limit, gdt_dpl) {\
	.limit_low	= (((gdt_limit) >> 12) & 0xFFFF),    \
	.base_low	= ((gdt_base) & 0xFFFF),             \
	.base_mid	= (((gdt_base) >> 16) & 0xFF),       \
	.type = gdt_type,                                 \
	.one = 1,                                         \
	.dpl = gdt_dpl,                                   \
	.present = 1,                                     \
	.limit_high = ((gdt_limit) >> 28),                     \
	.available = 0,                                    \
	.zero = 0,                                        \
	.op_size = 1,                                     \
	.gran = 1,                                        \
	.base_high = (((gdt_base) >> 24) & 0xFF),         \
}

struct GDT_entry GDT[GDT_SIZE] = {
	[_GDT_NULL] = {0}, // null entry required
	[_KERNEL_CS] = GDT_ENTRY(GDT_EXE | GDT_READ, 0, 0xFFFFFFFF, DPL_KERNEL),
	[_KERNEL_DS] = GDT_ENTRY(GDT_WRITE,	0, 0xFFFFFFFF, DPL_KERNEL)
};

struct GDT_ptr {
	unsigned short limit;
	unsigned long base;
} __attribute__((packed));

extern void load_gdt(struct GDT_ptr *gdt_ptr);

void gdt_init(void) {
	struct GDT_ptr gdt_ptr;
	
	gdt_ptr.limit = sizeof(GDT);
	gdt_ptr.base = (unsigned long) GDT;
	load_gdt(&gdt_ptr);
}

// IDT - Interupt Descriptor Table - matches interupt number to interupt handler address
struct IDT_entry {
	unsigned short int offset_low;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_high;
} __attribute__((packed)); // makes sure that gcc doesn't add padding for any reason

struct IDT_entry IDT[IDT_SIZE];

struct IDT_ptr {
	unsigned short limit;
	unsigned long base;
} __attribute__((packed));

extern void load_idt(struct IDT_ptr *idt_ptr);

void idt_init(void) {
	unsigned long keyboard_address;
	struct IDT_ptr idt_ptr;
	
	// populate IDT entry for keyboard interrupt
	keyboard_address = (unsigned long) keyboard_handler;
	IDT[0x21].offset_low = keyboard_address & 0xffff;
	IDT[0x21].selector = 0x08; // KERNEL_CODE_SEGMENT_OFFSET
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = 0x8e; // INTERRUPT_GATE
	IDT[0x21].offset_high = (keyboard_address & 0xffff0000) >> 16;
	
	// ICW1 - begin init - sets to wait for three more bytes on data ports
	write_port(PIC1_CMD, 0x11);
	write_port(PIC2_CMD, 0x11);
	
	// ICW2 - remap offset of IDT because first 32 interrupts are for CPU
	write_port(PIC1_DAT, 0x20);
	write_port(PIC2_DAT, 0x28);
	
	// ICW3 - set everything with no slaves
	write_port(PIC1_DAT, 0x00);
	write_port(PIC2_DAT, 0x00);
	
	// ICW4 - enviroment info - 80x86 mode
	write_port(PIC1_DAT, 0x01);
	write_port(PIC2_DAT, 0x01);
	
	// mask interupts
	write_port(PIC1_DAT, 0xff);
	write_port(PIC1_DAT, 0xff);
	
	// fill IDT descriptor
	idt_ptr.limit = sizeof(IDT);
	idt_ptr.base = (unsigned long) IDT;
	load_idt(&idt_ptr);
}

void kb_init(void) {
	// 0xFD = 11111101 - IRQ1 only enabled, keyboard
	write_port(0x21, 0xFD);
}

void keyboard_handler_main(void) {
	unsigned char status;
	char keycode;
	
	// write EOI to allow more interrupts
	write_port(PIC1_CMD, 0x20);
	
	status = read_port(KEYBOARD_STATUS);
	// lowest bit set if buffer empty
	if(status & 0x01) {
		keycode = read_port(KEYBOARD_DATA);
		if(keycode <= 0) {
			return;
		}
		vidptr[cursor_loc*2] = keyboard_map[keycode];
		vidptr[cursor_loc*2 + 1] = 0x07;
		cursor_loc++;
	}
}

void kprint(const char *str) {
	unsigned int i = 0;

	while(str[i] != '\0') {
		vidptr[cursor_loc*2] = str[i];
		vidptr[cursor_loc*2 + 1] = 0x07;
		i++;
		cursor_loc++;
	}
}

void kprintln(void) {
	cursor_loc += COLUMNS;
}

void clear_screen(void) {
	unsigned int i = 0;
	
	while(i < 80 * 25 * 2) {
		vidptr[i] = ' ';
		// attribute byte - light grey on black
		vidptr[i + 1] = 0x07;
		i = i + 2;
	}
}
 
void kmain(void) {
	const char *str = "Welcome to Yornel";
	
	clear_screen();
	
	// splash screen
	kprint(str);
	kprintln();
	kprintln();
	
	// init
	gdt_init();
	idt_init();
	kb_init();
	
	while(1) {
		asm volatile("hlt");
	}
}

