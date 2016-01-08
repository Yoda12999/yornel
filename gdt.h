#ifndef _GDT_H
#define _GDT_H

#include <stdint.h>

void load_gdt(void* base, uint16_t size);

#endif

