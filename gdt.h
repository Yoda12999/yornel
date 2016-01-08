#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <stdint.h>

void inline load_gdt(void* base, uint16_t size);

#endif

