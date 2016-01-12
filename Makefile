export
HOST = i686-elf
HOSTARCH = i386

AR = $(HOST)-ar
AS = $(HOST)-as
CC = $(HOST)-gcc

KERNEL_BIN = yornel.kernel
BINARIES = libc.a libg.a libk.a
KERNEL_ISO = yornel.iso

DESTDIR := $(CURDIR)/sysroot
INCLUDEDIR := $(DESTDIR)$(PREFIX)/include

PREFIX := /usr
EXEC_PREFIX := $(PREFIX)
BOOTDIR := $(DESTDIR)/boot
LIBDIR := $(DESTDIR)$(EXEC_PREFIX)/lib

KERNELDIR := $(CURDIR)/kernel
LIBCDIR := $(CURDIR)/libc

CC := $(CC) --sysroot=$(DESTDIR) -isystem=$(INCLUDEDIR)

.PHONY: all clean run install-headers build-kernel build-libc

all: $(BOOTDIR)/$(KERNEL_BIN)

run: | $(BOOTDIR)/$(KERNEL_BIN)
	qemu-system-$(HOSTARCH) --kernel $(BOOTDIR)/$(KERNEL_BIN)
	#qemu-system-$(HOSTARCH) --cdrom $(KERNEL_ISO)

$(INCLUDEDIR):
	mkdir -p $(INCLUDEDIR)
	make -C $(LIBCDIR) install-headers
	make -C $(KERNELDIR) install-headers

build-libc: $(INCLUDEDIR)
	make -C $(LIBCDIR) $(BINARIES)

build-kernel: $(INCLUDEDIR)
	make -C $(KERNELDIR) $(KERNEL_BIN)

$(LIBDIR):
	make -C $(LIBCDIR) install-libs

$(BOOTDIR)/$(KERNEL_BIN): $(INCLUDEDIR) $(LIBDIR)
	make -C $(KERNELDIR) install-kernel

$(KERNEL_ISO): $(BOOTDIR)/$(KERNEL_BIN)
	$(file > grub.cfg,menuentry "yornel" { \
		multiboot /boot/yornel.kernel \
	})
	mkdir -p isodir/boot/grub
	cp $(BOOTDIR)/$(KERNEL_BIN) isodir/boot/$(KERNEL_BIN)
	mv grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue /usr/lib/grub/i386-pc -o yornel.iso isodir

clean:
	rm -Rf $(DESTDIR) $(KERNEL_ISO) isodir
	make -C $(KERNELDIR) clean
	make -C $(LIBCDIR) clean
