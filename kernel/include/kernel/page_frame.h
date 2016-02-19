#pragma once

#include <kernel/multiboot2.h>
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE (0x1000)
#define MAX_MEMORY (0x100000000)

void mmap_init(struct multiboot_tag_mmap* mmap_tag);
void* kalloc_frame(void);
void kfree_frame(void* f);
