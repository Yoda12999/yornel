#include "gdt.h"
#include "interrupt.h"
#include "keyboard.h"
#include "tty.h"

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kearly(void) {
	gdt_init();
	int_init();
	kb_init();
}

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kmain(void) {
	const char *str = "Welcome to Yornel\n\n\r";
	
	clear_screen();
	
	// splash screen
	kprint(str);
	
	int_enable();
	
	while(1) {
		asm volatile("hlt");
	}
}

