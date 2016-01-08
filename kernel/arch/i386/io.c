#include "io.h"

inline void outb(uint16_t port, uint8_t val) {
	asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile ( "inb %[port], %[result]"
					: [result] "=a"(ret)
					: [port] "Nd"(port) );
	return ret;
}
