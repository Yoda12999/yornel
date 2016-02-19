#include <kernel/page_frame.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

enum mem_type {
	FREE,
	RECLAIMABLE,
	RESERVED,
	NVS,
};

struct mmap_entry {
	uintptr_t addr;
	size_t len;
	enum mem_type type;
};

struct {
	uint8_t num;
	struct mmap_entry entries[32];
} mmap;

#define PAGE_SIZE (0x1000)
#define MAX_MEMORY (0x100000000)

#define ERROR NULL;

extern uint32_t endkernel;

static uintptr_t first_page_frame = 0;

size_t npages = 0;
uint32_t frame_map[MAX_MEMORY / PAGE_SIZE / 32];
void* pre_frames[32];

static enum mem_type convert_type(uint32_t tag_type) {
	switch(tag_type) {
		case MULTIBOOT_MEMORY_AVAILABLE:
			return FREE;
		case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
			return RECLAIMABLE;
		case MULTIBOOT_MEMORY_NVS:
			return NVS;
		default:
			return RESERVED;
	}
}

//sort the grub memmory map using in place merge sort
static void sort_mmap(multiboot_memory_map_t* m, size_t first, size_t last) {
	if(first >= last) {
		return;
	}

	size_t mid = (first + last)/2;
	sort_mmap(m, first, mid);
	sort_mmap(m, mid + 1, last);

	size_t left = first;
	size_t right = last;
	if(m[mid].addr <= m[right].addr) {
		return;
	}

	while(left <= mid && right <= last) {
		if(m[left].addr >= m[right].addr) {
			left++;
		} else {
			multiboot_memory_map_t tmp;
			tmp = m[right];
			memmove(m + left + 1, m + left, (right - left)*sizeof(multiboot_memory_map_t));
			m[left] = tmp;
			left++;
			mid++;
			right++;
		}
	}
}

void mmap_init(struct multiboot_tag_mmap* mmap_tag) {
	first_page_frame = (PAGE_SIZE - ((uintptr_t) &endkernel % PAGE_SIZE)) + (uintptr_t) &endkernel;

	//set pages with reserved segments as RESERVED
	mmap.num = 1;
	mmap.entries[0].addr = 0;
	mmap.entries[0].len = 0;
	mmap.entries[0].type = 0;

	size_t num_entries = ((uintptr_t) mmap_tag - 16)/mmap_tag->entry_size;

	for(unsigned int i = 0; i < num_entries; i++) {
		printf("Index: %d Addr: 0x%p Len: %x Type: %d\n\r", i,
			mmap_tag->entries[i].addr, mmap_tag->entries[i].len, mmap_tag->entries[i].type);
	}
	
	sort_mmap((multiboot_memory_map_t*) mmap_tag->entries, 0, num_entries - 1);

	//the table is now sorted  from lowest start addr to highest
	//I should probably print this to make sure
	for(unsigned int i = 0; i < num_entries; i++) {
		printf("Index: %d Addr: 0x%p Len: %x Type: %d\n\r", i,
			mmap_tag->entries[i].addr, mmap_tag->entries[i].len, mmap_tag->entries[i].type);
	}

	//convert bios table to our shit
	for(unsigned int i = 0; i < num_entries; i++) {
		uintptr_t prev_end = mmap.entries[mmap.num - 1].addr + mmap.entries[mmap.num - 1].len + 1;

		if(convert_type(mmap_tag->entries[i].type) == mmap.entries[mmap.num - 1].type) {
			//it's a continuation
			ptrdiff_t diff = mmap_tag->entries[i].addr - prev_end + mmap_tag->entries[i].len;
			//double check that we are actually extending this
			if(diff > 0) {
				mmap.entries[mmap.num - 1].len += diff;
			}
		} else if(mmap.entries[mmap.num - 1].addr == mmap_tag->entries[i].addr) {
			//they start at the same point, shit
			//welp, find which one trunps the other and which one really comes first
			if(convert_type(mmap_tag->entries[i].type) > mmap.entries[mmap.num - 1].type) {
				//the previous chunk was less important than than the next one
				if(mmap.entries[mmap.num].len > mmap_tag->entries[i].len) {
					//new chunk is samller than existing so there is still some of existing left
					mmap.entries[mmap.num].addr = mmap_tag->entries[i].addr + mmap_tag->entries[i].len + 1;
					mmap.entries[mmap.num].len = mmap.entries[mmap.num - 1].len - mmap_tag->entries[i].len;
					mmap.entries[mmap.num].type = mmap.entries[mmap.num - 1].type;

					mmap.entries[mmap.num - 1].len = mmap_tag->entries[i].len;
					mmap.entries[mmap.num - 1].type = convert_type(mmap_tag->entries[i].type);
					mmap.num++;
				} else {
					//existing is the same or smaller so we need to completely overwrite
					mmap.entries[mmap.num - 1].len = mmap_tag->entries[i].len;
					mmap.entries[mmap.num - 1].type = convert_type(mmap_tag->entries[i].type);
				}
			} else if(mmap.entries[mmap.num].len < mmap_tag->entries[i].len) {
				//the previous chunk was more important than than the next one, but the next one is longer 
				mmap.entries[mmap.num].addr = prev_end;
				mmap.entries[mmap.num].len = mmap_tag->entries[i].len - mmap.entries[mmap.num - 1].len;
				mmap.entries[mmap.num].type = convert_type(mmap_tag->entries[i].type);
				mmap.num++;
			}
		} else if(mmap_tag->entries[i].addr < prev_end) {
			//we have an overlap, but we start after the previous
			//it's different so find out if we cut from the prev chunk or the new one
			//FREE < RECLAIMABLE < RESERVED < NVS
			if(convert_type(mmap_tag->entries[i].type) > mmap.entries[mmap.num - 1].type) {
				//the previous chunk was less important than than the next one
				mmap.entries[mmap.num - 1].len = mmap.entries[0].addr - mmap_tag->entries[i].addr;

				mmap.entries[mmap.num].addr = mmap_tag->entries[i].addr;
				mmap.entries[mmap.num].len = mmap_tag->entries[i].len;
				mmap.entries[mmap.num].type = convert_type(mmap_tag->entries[i].type);
				mmap.num++;
			} else {
				//the previous chunk was more important than than the next one
				mmap.entries[mmap.num].addr = prev_end;
				mmap.entries[mmap.num].len = mmap_tag->entries[i].len;
				mmap.entries[mmap.num].type = convert_type(mmap_tag->entries[i].type);
				mmap.num++;
			}
		} else {
			//new block, which may or may not start exactly after the previous one
			mmap.entries[mmap.num].addr = mmap_tag->entries[i].addr;
			mmap.entries[mmap.num].len = mmap_tag->entries[i].len;
			mmap.entries[mmap.num].type = convert_type(mmap_tag->entries[i].type);
			mmap.num++;
		}
	}

	//set pages with used sections as used
	npages = 0;
}

static void* kalloc_frame_int(bool restart) {
	static size_t i = 0;

	if(restart) {
		i = 0;
	}

	while(frame_map[i/32] != 0xFFFFFFFF) {
		i += 32;
		if(i == npages) {
			return ERROR;
		}
	}

	while(i%32 > 0) { //finish up searching current bitmap
		i++;
		if(i == npages) {
			return ERROR;
		} else if((frame_map[i/32] & (1 << i%32)) == 0) {
			frame_map[i/32] |= 1 << i%32;
			return (void*) (first_page_frame + (i * PAGE_SIZE));
		}
	}

	return ERROR;
}

void* kalloc_frame() {
	static bool allocate = true;
	static uint8_t pframe = 0;

	if(pframe >= 32) {
		allocate = true;
	}

	if(allocate) {
		for(int i = 0; i < 32; i++) {
			//TODO: check for error
			pre_frames[i] = kalloc_frame_int(i == 0);
		}
		pframe = 0;
		allocate = false;
	}

	return (void*) pre_frames[pframe++];
}

void kfree_frame(void* f) {
	ptrdiff_t d = (uintptr_t) f -  first_page_frame;
	size_t s = d / 0x1000;
	frame_map[s/32] &= ~(1 << s%32);
}
