#ifndef _KERNEL_KB_H
#define _KERNEL_KB_H

#include <stdint.h>

struct Mod_keys {
	uint8_t shift :2;		//set with either shift key
	uint8_t r_shift :1;	//set with only the right shift key
	uint8_t ctrl :2;
	uint8_t r_ctrl :1;
	uint8_t alt :2;
	uint8_t r_alt :1;
	uint8_t meta :2;
	uint8_t r_meta :1;
	uint8_t clck :1;
	uint8_t nlck :1;
	uint8_t slck :1;
} mod_keys;

extern void kb_init(void);

#endif
