#if !defined(__cplusplus)
#include <stdbool.h>
#endif
#include <stddef.h>
#include <stdint.h>
 
#include "keyboard_map.h"
#include "io.h"
#include "idt.h"
#include "gdt.h"

#define LINES 25
#define COLUMNS 80
#define SCREEN_SIZE (LINES * COLUMNS * 2)

#define KEYBOARD_STATUS 0x64
#define KEYBOARD_DATA 0x60

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

