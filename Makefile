CFLAGS += -ffreestanding

SRCS := head.asm kernel.c
OBJS := $(foreach src,$(SRCS),$(basename $(src)).o)

LINK_SCRIPT := link.ld

KERNEL_BIN := yornel

all: $(KERNEL_BIN)

.PHONY: clean
clean:
	rm $(KERNEL_BIN)
	rm $(OBJS)

.PHONY: run
run: $(KERNEL_BIN)
	qemu-system-i386 --kernel $(KERNEL_BIN)

$(KERNEL_BIN): $(OBJS)
	ld -m elf_i386 -T $(LINK_SCRIPT) -o $@ $(OBJS)

%.o: %.asm
	nasm -f elf32 $< -o $@

%.o: %.c
	gcc -m32 $(CFLAGS) -c $< -o $@

