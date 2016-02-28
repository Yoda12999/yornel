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
BUILDDIR := $(CURDIR)/build

PREFIX := /usr
EXEC_PREFIX := $(PREFIX)
BOOTDIR := $(DESTDIR)/boot
INCLUDEDIR := $(PREFIX)/include
LIBDIR := $(EXEC_PREFIX)/lib

KERNELDIR := $(CURDIR)/kernel
LIBCDIR := $(CURDIR)/libc

CC := $(CC) --sysroot=$(DESTDIR) -isystem=$(INCLUDEDIR) -g

.PHONY: all clean run install-headers build-kernel build-libc

all: $(BOOTDIR)/$(KERNEL_BIN) grub.img

debug: | $(BOOTDIR)/$(KERNEL_BIN) grub.img
	qemu-system-i386 -fda grub.img -hda fat:$(DESTDIR) -no-reboot -no-shutdown -boot order=a -s

run: | $(BOOTDIR)/$(KERNEL_BIN) grub.img
	qemu-system-i386 -fda grub.img -hda fat:$(DESTDIR) -no-reboot -no-shutdown -boot order=a

build-kernel: $(KERNELDIR)/$(KERNEL_BIN)

build-libc: $(addprefix $(LIBCDIR)/,$(BINARIES))

install-headers: $(DESTDIR)$(INCLUDEDIR)

$(DESTDIR)$(INCLUDEDIR): $(KERNELDIR)/include $(LIBCDIR)/include | $(DESTDIR)$(PREFIX)
	mkdir $(DESTDIR)$(INCLUDEDIR)
	cp -RTv $(KERNELDIR)/include $(DESTDIR)$(INCLUDEDIR)
	cp -RTv $(LIBCDIR)/include $(DESTDIR)$(INCLUDEDIR)

$(addprefix $(LIBCDIR)/,$(BINARIES)): $(DESTDIR)$(INCLUDEDIR) $(LIBCDIR)
	make -C $(LIBCDIR) $(notdir $@)

$(addprefix $(DESTDIR)$(LIBDIR)/,$(BINARIES)): $(addprefix $(LIBCDIR)/,$(BINARIES)) | $(DESTDIR)$(LIBDIR)
	cp -f $(addprefix $(LIBCDIR)/,$(BINARIES)) $(DESTDIR)$(LIBDIR)

$(KERNELDIR)/$(KERNEL_BIN): $(DESTDIR)$(INCLUDEDIR) $(addprefix $(DESTDIR)$(LIBDIR)/,$(BINARIES)) $(KERNELDIR)
	make -C $(KERNELDIR) $(KERNEL_BIN)

$(BOOTDIR)/$(KERNEL_BIN): $(KERNELDIR)/$(KERNEL_BIN) | $(BOOTDIR)
	cp $(KERNELDIR)/$(KERNEL_BIN) $@

$(DESTDIR)$(LIBDIR): | $(DESTDIR)$(EXEC_PREFIX)
	mkdir $@

$(BOOTDIR): | $(DESTDIR)
	mkdir $@

$(DESTDIR)$(PREFIX): | $(DESTDIR)
	mkdir $@

$(DESTDIR)$(EXEC_PREFIX): | $(DESTDIR)
	mkdir $@

$(DESTDIR):
	mkdir $@

grub.img: grub.cfg
	grub-mkimage -O i386-pc -c grub.cfg -o tmp.img biosdisk multiboot multiboot2 normal ls cat help elf chain configfile fat lsmmap mmap msdospart part_msdos vga vga_text
	cat /usr/lib/grub/i386-pc/boot.img tmp.img > $@
	rm tmp.img

$(KERNEL_IMG): $(BOOTDIR)/$(KERNEL_BIN) grub.img
	dd if=/dev/zero of=$@ bs=512 count=131072
	sudo parted $@ -- mklabel msdos mkpart primary fat32 2048s -1s set 1 boot on
	sudo losetup /dev/loop0 $@
	sudo losetup /dev/loop1 $@ -o 1048576
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
