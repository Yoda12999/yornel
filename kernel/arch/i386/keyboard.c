#include <kernel/keyboard.h>

#include <stdint.h>
#include <kernel/interrupt.h>
#include <kernel/io.h>
#include <kernel/keyboard_map.h>
#include <kernel/tty.h>

#define KEYBOARD_STATUS 0x64
#define KEYBOARD_DATA 0x60

extern void keyboard_handler(void);

void kb_init(void) {
	// 0xFD = 11111101 - IRQ1 only enabled, keyboard
	outb(0x21, 0xFD);
	
	add_interrupt(0x21, keyboard_handler, 0x8e);
}

void keyboard_handler_main(void) {
	uint8_t status;
	uint8_t scancode;
	uint8_t keycode;
	
	// write EOI to allow more interrupts
	EOI();
	
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

