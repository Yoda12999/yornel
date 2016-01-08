#ifndef _IDT_H
#define _IDT_H

#include <stdint.h>

void inline load_idt(void* base, uint16_t size);

#endif
