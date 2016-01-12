HOST ?= i686-elf
HOSTARCH ?= i386

AR ?= $(HOST)-ar
AS ?= $(HOST)-as
CC ?= $(HOST)-gcc

ARCHDIR ?= arch/$(HOSTARCH)

CFLAGS ?= -O2 -g
CPPFLAGS ?= 
LDFLAGS ?= 
LIBS ?= 

include $(ARCHDIR)/make.config

CFLAGS := $(CFLAGS) -Wall -Wextra $(ARCH_CFLAGS)
CPPFLAGS := $(CPPFLAGS) -D__is_yornel_libc -Iinclude $(ARCH_CPPFLAGS)
LIBK_CFLAGS := $(CFLAGS) -ffreestanding -fbuiltin $(KERNEL_ARCH_CFLAGS)
LIBK_CPPFLAGS := $(CPPFLAGS) -D__is_yornel_kernel $(KERNEL_ARCH_CPPFLAGS)

FREEOBJS := \
$(ARCH_FREEOBJS) \
stdio/printf.o \
stdio/putchar.o \
stdio/puts.o \
stdlib/abort.o \
string/memcmp.o \
string/memcpy.o \
string/memmove.o \
string/memset.o \
string/strlen.o \

HOSTEDOBJS := \
$(ARCH_HOSTEDOBJS) \

OBJS := \
$(FREEOBJS) \
$(HOSTEDOBJS) \

LIBK_OBJS := $(FREEOBJS: .o=.libk.o)

BINARIES ?= libc.a libg.a libk.a

.PHONY: all clean install-libs

all: $(BINARIES)

libc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

libg.a:
	$(AR) rcs $@

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

%.o: %.c
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

%.o: %.S
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

%.libk.o: %.c
	$(CC) -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

%.libk.o: %.S
	$(CC) -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

clean:
	rm -f $(BINARIES) $(OBJS)  *.o */*.o */*/*.o
	
install-libs: $(BINARIES)
	mkdir -p $(LIBDIR)
	cp $(BINARIES) $(LIBDIR)

install-headers:
	mkdir -p $(INCLUDEDIR)
	cp -RTv include $(INCLUDEDIR)
