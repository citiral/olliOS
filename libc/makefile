#structure: folders never start or end with /

CC = i686-elf-gcc
AS = i686-elf-as
AR = i686-elf-ar

ARCH = arch/i686

ROOT = ../root/

LIBC_CCFLAGS = -D__is_libc -D__is_kernel -std=gnu11 -O2 -g -Wall -Wextra -fno-exceptions $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -nostdlib
LIBK_CCFLAGS = -D__is_libk -D__is_kernel -std=gnu11 -fbuiltin -ffreestanding -O2 -g -Wall -Wextra -fno-exceptions $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -nostdlib

INCLUDE = -I $(ROOT)usr/include

LIBC_CPP = $(wildcard string/*.c)
LIBK_CPP = $(wildcard string/*.c)
HEADERS = $(wildcard include/*.h)

BINARIES = libc.a libk.a
LIBC_OBJ = $(LIBC_CPP:.c=.o)
LIBK_OBJ = $(LIBK_CPP:.c=.libk.o)

.PHONY: all clean install install-headers install-binaries

all: install-headers $(BINARIES) install-binaries

libc.a: $(LIBC_OBJ)
	$(AR) rcs ../build/$@ $(addprefix ../build/,$(notdir $(LIBC_OBJ)))

libk.a: $(LIBK_OBJ)
	$(AR) rcs ../build/$@ $(addprefix ../build/,$(notdir $(LIBK_OBJ)))

%.o: %.c
	$(CC) -c $< -o ../build/$(notdir $@) $(LIBC_CCFLAGS)

%.libk.o: %.c
	$(CC) -c $< -o ../build/$(notdir $@) $(LIBK_CCFLAGS)

clean:
	rm -f $(addprefix $(ROOT)usr/bin/, $(BINARIES))

install: install-headers install-binaries

install-headers:
	cp -Rv $(HEADERS) $(ROOT)usr/include

install-binaries:
	cp $(addprefix ../build/,$(BINARIES)) $(ROOT)usr/lib
