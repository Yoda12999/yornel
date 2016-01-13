#pragma once

#include <stdint.h>
#include <stddef.h>

void kprint_char(char c);
void kprint(const char *data, size_t size);
void kprint_string(const char *str);
void clear_screen(void);

