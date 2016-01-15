#pragma once

#include <kernel/tty.h>

struct terminal kterm;

static inline void kprint_char(char c) {
	term_putchar(kterm, c);
}

static inline void kprint(const char* str) {
	term_write_string(kterm, str);
}
