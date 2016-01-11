#pragma once

#include <stdint.h>
#include <stdbool.h>

void int_enable(void);
void int_disable(void);
bool add_interrupt(uint8_t id, void (*handler)(void), uint8_t type_attr);
bool rm_interrupt(uint8_t id);
void int_init(void);
void EOI(void);

