HOST = i686-elf
HOSTARCH := i386

AR = $(HOST)-ar
AS = $(HOST)-as
CC = $(HOST)-gcc

# Configure the cross-compiler to use the desired system root.
CC := $(CC) --sysroot=$(CURDIR)/sysroot

# Work around that the -elf gcc targets doesn't have a system include directory
# because configure received --without-headers rather than --with-sysroot.
CC := $(CC) -isystem=$(INCLUDEDIR)

CFLAGS ?= -O2 -g
CPPFLAGS ?= 
LDFLAGS ?= 
LIBS ?= 

DESTDIR ?=
PREFIX ?= /usr/local
EXEC_PREFIX ?=$(PREFIX)
BOOTDIR ?= $(EXEC_PREFIX)/boot
INCLUDEDIR ?= $(PREFIX)/include

CFLAGS := $(CFLAGS) -ffreestanding -fbuiltin -Wall -Wextra
CPPFLAGS := $(CPPFLAGS) -D__is_yornel_kernel -Iinclude
LDFLAGS := $(LDFLAGS)
LIBS := $(LIBS) -nostdlib -lgcc

ARCHDIR := arch/$(HOSTARCH)

#include $(ARCHDIR)/make.config

CFLAGS := $(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS := $(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS := $(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS := $(LIBS) $(KERNEL_ARCH_LIBS)

#OBJS := \
#$(KERNEL_ARCH_OBJS) \
#kernel/kernel.o \

#CRTI_OBJ := $(ARCHDIR)/crti.o
#CRTBEGIN_OBJ := $(shell $(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=crtbegin.o)
#CRTEND_OBJ := $(shell $(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=crtend.o)
#CRTN_OBJ := $(ARCHDIR)/crtn.o


#ALL_OUR_OBJS := \
#$(CRTI_OBJ) \
#$(OBJS) \
#$(CRTN_OBJ) \

#OBJ_LINK_LIST := \
#$(CRTI_OBJ) \
#$(CRTBEGIN_OBJ) \
#$(OBJS) \
#$(CRTEND_OBJ) \
#$(CRTN_OBJ) \

SRCS := kernel.c boot.S gdt.c idt.c io.c keyboard_handler.S interrupt.c df_handler.S
OBJS := $(foreach src,$(SRCS),$(basename $(src)).o)

LINK_SCRIPT := link.ld

KERNEL_BIN := yornel.kernel

all: $(KERNEL_BIN)

.PHONY: clean run install install-headers install-kernel

clean:
	rm $(KERNEL_BIN) $(OBJS)

run: $(KERNEL_BIN)
	qemu-system-i386 --kernel $(KERNEL_BIN)

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -RTv include $(DESTDIR)$(INCLUDEDIR)

install-kernel: $(KERNEL_BIN)
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp yornel.kernel $(DESTDIR)$(BOOTDIR)

$(KERNEL_BIN): $(OBJS) $(LINK_SCRIPT)
	$(CC) -T $(LINK_SCRIPT) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS)

%.o: %.S
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

