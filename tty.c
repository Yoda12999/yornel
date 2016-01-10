#include "tty.h"

#include <stddef.h>

#define LINES 25
#define COLUMNS 80
#define SCREEN_SIZE (LINES * COLUMNS * 2)

uint32_t cursor_loc = 0;
uint8_t tab_size = 4;

// video mem starts here. Maybe make 16 bit addressed instead...
uint8_t *vidptr = (uint8_t*) 0xb8000;

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

void clear_screen(void) {
	size_t i = 0;
	
	while(i < 80 * 25 * 2) {
		vidptr[i] = ' ';
		// attribute byte - light grey on black
		vidptr[i + 1] = 0x07;
		i = i + 2;
	}
}
