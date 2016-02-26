export
HOST = i686-elf
HOSTARCH = i386

AR = $(HOST)-ar
AS = $(HOST)-as
CC = $(HOST)-gcc

KERNEL_BIN = yornel.kernel
BINARIES = libc.a libg.a libk.a
KERNEL_IMG = yornel.img

DESTDIR := $(CURDIR)/sysroot

PREFIX := /usr
EXEC_PREFIX := $(PREFIX)
BOOTDIR := $(DESTDIR)/boot
INCLUDEDIR := $(PREFIX)/include
LIBDIR := $(EXEC_PREFIX)/lib

KERNELDIR := $(CURDIR)/kernel
LIBCDIR := $(CURDIR)/libc

CC := $(CC) --sysroot=$(DESTDIR) -isystem=$(INCLUDEDIR) -g

.PHONY: all clean run install-headers build-kernel build-libc

all: $(BOOTDIR)/$(KERNEL_BIN)

debug: | $(BOOTDIR)/$(KERNEL_BIN) grub.img
	qemu-system-i386 -fda grub.img -hda fat:$(DESTDIR) -no-reboot -no-shutdown -boot order=a -s

run: | $(BOOTDIR)/$(KERNEL_BIN) grub.img
	qemu-system-i386 -fda grub.img -hda fat:$(DESTDIR) -no-reboot -no-shutdown -boot order=a

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

$(BOOTDIR)/$(KERNEL_BIN): $(DESTDIR)$(INCLUDEDIR) $(DESTDIR)$(LIBDIR)
	make -C $(KERNELDIR) install-kernel

grub.img: grub.cfg
	grub-mkimage -O i386-pc -c grub.cfg -o tmp.img biosdisk multiboot multiboot2 normal ls cat help elf chain configfile fat lsmmap mmap msdospart part_msdos vga vga_text
	cat /usr/lib/grub/i386-pc/boot.img tmp.img > grub.img
	rm tmp.img

$(KERNEL_IMG): $(BOOTDIR)/$(KERNEL_BIN) grub.img
	dd if=/dev/zero of=$(KERNEL_IMG) bs=512 count=131072
	sudo parted $(KERNEL_IMG) -- mklabel msdos mkpart primary fat32 2048s -1s set 1 boot on
	sudo losetup /dev/loop0 $(KERNEL_IMG)
	sudo losetup /dev/loop1 $(KERNEL_IMG) -o 1048576
	sudo mkfs.fat -F 32 /dev/loop1
	mkdir mnt
	sudo mount /dev/loop1 mnt
	cp -R grub.cfg $(BOOTDIR)/grub
	sudo cp -R $(DESTDIR)/* mnt
	sudo grub-install --root-directory=mnt --no-floppy --modules="normal part_msdos multiboot configfile disk elf fat help lsmmap ls mmap multiboot2" --target=i386-pc /dev/loop0
	sudo sync
	sudo umount mnt
	sudo losetup -d /dev/loop0
	sudo losetup -d /dev/loop1
	rmdir mnt

clean:
	rm -Rf $(DESTDIR) $(KERNEL_IMG) isodir grub.img
	make -C $(KERNELDIR) clean
	make -C $(LIBCDIR) clean
