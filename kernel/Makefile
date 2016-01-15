HOST ?= i686-elf
HOSTARCH ?= i386

AR ?= $(HOST)-ar
AS ?= $(HOST)-as
CC ?= $(HOST)-gcc

ARCHDIR ?= arch/$(HOSTARCH)

KERNEL_BIN ?= yornel.kernel

CFLAGS ?= -O2 -g
CPPFLAGS ?= 
LDFLAGS ?= 
LIBS ?= 

include $(ARCHDIR)/make.config

CFLAGS := $(CFLAGS) -ffreestanding -fbuiltin -Wall -Wextra $(KERNEL_ARCH_CFLAGS)
CPPFLAGS := $(CPPFLAGS) -D__is_yornel_kernel -Iinclude $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS := $(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS := $(LIBS) -nostdlib -lgcc -lk $(KERNEL_ARCH_LIBS)

OBJS := \
$(KERNEL_ARCH_OBJS) \
kernel/kernel.o \

CRTI_OBJ := $(ARCHDIR)/crti.o
CRTBEGIN_OBJ := $(shell $(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ := $(shell $(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=crtend.o)
CRTN_OBJ := $(ARCHDIR)/crtn.o

ALL_OUR_OBJS := \
$(CRTI_OBJ) \
$(OBJS) \
$(CRTN_OBJ) \

OBJ_LINK_LIST := \
$(CRTI_OBJ) \
$(CRTBEGIN_OBJ) \
$(OBJS) \
$(CRTEND_OBJ) \
$(CRTN_OBJ) \

LINK_SCRIPT = $(ARCHDIR)/link.ld

.PHONY: all clean install-kernel

clean:
	rm -f $(KERNEL_BIN) $(ALL_OUR_OBJS) *.o */*.o */*/*.o

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(OBJ_LINK_LIST) $(LINK_SCRIPT)
	$(CC) -T $(LINK_SCRIPT) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS)

%.o: %.S
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

install-kernel: $(KERNEL_BIN)
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp $(KERNEL_BIN) $(DESTDIR)$(BOOTDIR)

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -RTv include $(DESTDIR)$(INCLUDEDIR)
