#define ASM_FILE 1
#include <kernel/multiboot2.h>

# define MULTIBOOT_CHECKSUM -(multiboot_header_end - multiboot_header + MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386)

.section .multiboot
.align MULTIBOOT_INFO_ALIGN
multiboot_header:
	.int MULTIBOOT2_HEADER_MAGIC
	.int MULTIBOOT_ARCHITECTURE_I386
	.int multiboot_header_end - multiboot_header
	.int MULTIBOOT_CHECKSUM
request_tag_start:
	.align MULTIBOOT_TAG_ALIGN
	.short MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
	.short 0
	.int request_tag_end - request_tag_start
	.int MULTIBOOT_TAG_TYPE_MMAP
request_tag_end:
	.align MULTIBOOT_TAG_ALIGN
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

	call _init

	call kmain

	cli
.Lhang:
	hlt
	jmp .Lhang
.size _start, . - _start
