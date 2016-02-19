#define ASM_FILE 1
#include <kernel/multiboot2.h>

#ifdef MULTIBOOT2
# define MULTIBOOT_CHECKSUM -(multiboot_header_end - multiboot_header + MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386)
#else
# define MULTIBOOT_CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
# define MULTIBOOT_HEADER_FLAGS MULTIBOOT_PAGE_ALIGN & MULTIBOOT_MEMORY_INFO
#endif

.section .multiboot
.align MULTIBOOT_INFO_ALIGN
multiboot_header:
#ifdef MULTIBOOT2
	.int MULTIBOOT2_HEADER_MAGIC
	.int MULTIBOOT_ARCHITECTURE_I386
	.int multiboot_header_end - multiboot_header
#else
	.int MULTIBOOT_HEADER_MAGIC
	.int MULTIBOOT_HEADER_FLAGS
#endif
	.int MULTIBOOT_CHECKSUM
request_tag_start:
	.short MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
	.short 0
	.int framebuffer_tag_end - framebuffer_tag_start
	.int MULTIBOOT_TAG_TYPE_MMAP
request_tag_end:
framebuffer_tag_start:
	.short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
	.short MULTIBOOT_HEADER_TAG_OPTIONAL
	.int framebuffer_tag_end - framebuffer_tag_start
	.int 1024
	.int 768
	.int 32
framebuffer_tag_end:
	.short MULTIBOOT_HEADER_TAG_END
	.short MULTIBOOT_TAG_TYPE_END
	.int 8
multiboot_header_end:

.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global _start
.type _start, @function
_start:
	mov $stack_top, %esp	#set stack pointer
	pushl $0
	popf
	pushl %eax				#Multiboot magic number
	pushl %ebx				#Multiboot info

	call kearly

	#call _init

	call kmain

	cli
.Lhang:
	hlt
	jmp .Lhang
.size _start, . - _start
