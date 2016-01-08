#include <stddef.h>

#include <kernel/kb.h>
#include "io.h"
#include <kernel/tty.h>
#include <kernel/idt.h>

#define KEYBOARD_STATUS 0x64
#define KEYBOARD_DATA 0x60

#define ESC 0

#define LSFT_CODE 0x2a
#define RSFT_CODE 0x36
#define LSFT 0
#define RSFT 0

#define LCTL 0
#define LALT 0

#define META 0

#define CLCK_CODE 0x3a
#define NLCK_CODE 0x45
#define SLCK_CODE 0x46
#define CLCK 0
#define NLCK 0
#define SLCK 0

#define INS 0
#define HOME 0
#define PGUP 0
#define DEL 127
#define END 0
#define PGDN 0

#define UPAR 0
#define LTAR 0
#define RTAR 0
#define DNAR 0

#define F1 0
#define F2 0
#define F3 0
#define F4 0
#define F5 0
#define F6 0
#define F7 0
#define F8 0
#define F9 0
#define F10 0
#define F11 0
#define F12 0

uint8_t keyboard_map[128] = {
	0, ESC, '1', '2', '3', '4', '5', '6', '7', '8',
	'9', '0', '-', '=', '\b', '\t',	'q', 'w', 'e', 'r',
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', LCTL,
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'\'', '`', LSFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n',
	'm', ',', '.', '/', RSFT, '*', LALT, ' ', CLCK, F1,
	F2, F3, F4, F5, F6, F7, F8, F9, F10, NLCK,
	/* Keypad */
	SLCK, HOME, UPAR, PGUP, '-', LTAR, '5', RTAR, '+', END,
	DNAR, PGDN, INS, DEL, 0, 0, 0, F11, F12, 0,
	
	//don't exist on normal keyboards
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 99 */,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 109 */,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 119 */,
	0, 0, 0, 0, 0, 0, 0, 0 /* 127 */
};

uint8_t shift_map[52] = {
	/* 2 offset */
	'!', '@', '#', '$', '%', '^', '&', '*',
	'(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',
	'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', LCTL,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'"', '~', LSFT, '|', 'Z', 'X', 'C', 'V', 'B', 'N',
	'M', '<', '>', '?'
};

uint8_t nlck_map[13] = {
	/* Keypad - 71 offset*/
	'7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.'
};

extern void keyboard_handler(void);

void kb_init(void) {
	// 0xFD = 11111101 - IRQ1 only enabled, keyboard
	outb(PIC1_DAT, 0xFD);
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
					terminal_putchar('\r');
				}
				terminal_putchar(keycode);
		}
		
		/*
		kprint_num(scancode, 16);
		terminal_putchar(' ');
		*/
		
		status = inb(KEYBOARD_STATUS);
	}
}