#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/yornel.kernel isodir/boot/yornel.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "yornel" {
	multiboot /boot/yornel.kernel
}
EOF
grub-mkrescue /usr/lib/grub/i386-pc -o yornel.iso isodir
