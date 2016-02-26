#include <stdio.h>

#if defined(__is_yornel_kernel)
#include <kernel/kprint.h>
#endif

int putchar(int ic) {
#if defined(__is_yornel_kernel)
	kprint_char((char) ic);
#else
	// TODO: You need to implement a write system call.
#endif
	return ic;
}
