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

PREFIX := /usr
EXEC_PREFIX := $(PREFIX)
BOOTDIR := /boot
INCLUDEDIR := $(PREFIX)/include
LIBDIR := $(EXEC_PREFIX)/lib

KERNELDIR := $(CURDIR)/kernel
LIBCDIR := $(CURDIR)/libc

CC := $(CC) --sysroot=$(DESTDIR) -isystem=$(INCLUDEDIR)

.PHONY: all clean run install-headers build-kernel build-libc

all: $(DESTDIR)$(BOOTDIR)/$(KERNEL_BIN)

run: | $(DESTDIR)$(BOOTDIR)/$(KERNEL_BIN)
	qemu-system-$(HOSTARCH) --kernel $(DESTDIR)$(BOOTDIR)/$(KERNEL_BIN)
	#qemu-system-$(HOSTARCH) --cdrom $(KERNEL_ISO)

$(DESTDIR)$(INCLUDEDIR):
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	make -C $(LIBCDIR) install-headers
	make -C $(KERNELDIR) install-headers

build-libc: $(DESTDIR)$(INCLUDEDIR)
	make -C $(LIBCDIR) $(BINARIES)

build-kernel: $(DESTDIR)$(INCLUDEDIR)
	make -C $(KERNELDIR) $(KERNEL_BIN)

$(DESTDIR)$(LIBDIR):
	make -C $(LIBCDIR) install-libs

$(DESTDIR)$(BOOTDIR)/$(KERNEL_BIN): $(DESTDIR)$(INCLUDEDIR) $(DESTDIR)$(LIBDIR)
	make -C $(KERNELDIR) install-kernel

$(KERNEL_ISO): $(BOOTDIR)/$(KERNEL_BIN)
	$(file > grub.cfg,menuentry "yornel" { \
		multiboot /boot/yornel.kernel \
	})
	mkdir -p isodir/boot/grub
	cp $(DESTDIR)$(BOOTDIR)/$(KERNEL_BIN) isodir/boot/$(KERNEL_BIN)
	mv grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue /usr/lib/grub/i386-pc -o yornel.iso isodir

clean:
	rm -Rf $(DESTDIR) $(KERNEL_ISO) isodir
	make -C $(KERNELDIR) clean
	make -C $(LIBCDIR) clean
