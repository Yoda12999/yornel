#pragma once

#include <stdint.h>

void kprint_char(char c);
void kprint(const char *str);
void kprint_num(int32_t num, uint32_t radix);
void clear_screen(void);

