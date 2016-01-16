#include <kernel/tty.h>

#include <kernel/vga.h>
#include <string.h>

void term_init(struct terminal* term, size_t width, size_t height, void* buffer) {
	term->width = width;
	term->height = height;
	term->row = 0;
	term->col = 0;
	term->tab_size = 4;
	term->color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
	term->buffer = (uint16_t*) buffer;
}

void term_putchar_at(struct terminal* term, char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y*term->width + x;
	term->buffer[index] = make_vgaentry(c, color);
}

void term_putchar(struct terminal* term, char c) {
	switch(c) {
		case '\n':
			term->row++;
			break;
		case '\r':
			term->col = 0;
			return;
		case '\t':
			term->col += term->tab_size - 1;
			break;
		default:
			term_putchar_at(term, c, term->color, term->col, term->row);
	}

	if(++term->col == term->width) {
		term->col = 0;
		term->row++;
	}
	if(term->row == term->height) {
		term->row = 0;
	}
}

void term_write(struct terminal* term, const char *str, size_t size) {
	for(size_t i = 0; i < size; i++) {
		term_putchar(term, str[i]);	
	}
}

void term_write_string(struct terminal* term, const char *str) {
	term_write(term, str, strlen(str));
}

void term_clear_screen(struct terminal* term) {
	for(size_t y = 0; y < term->height; y++) {
		for(size_t x = 0; x < term->width; x++) {
			const size_t index = y*term->width + x;
			term->buffer[index] = make_vgaentry(' ', term->color);
		}
	}
}
