#pragma once

#include <stdint.h>
#include <stddef.h>

struct terminal {
	size_t width;
	size_t height;
	size_t row;
	size_t col;
	size_t tab_size;
	uint8_t color;
	uint16_t* buffer;
};

void term_init(struct terminal term, size_t width, size_t height, void* buffer);
void term_putchar_at(struct terminal term, char c, uint8_t color, size_t x, size_t y);
void term_putchar(struct terminal term, char c);
void term_write(struct terminal term, const char *data, size_t size);
void term_write_string(struct terminal term, const char *str);
void term_clear_screen(struct terminal term);
