#structure: folders never start or end with /

CC = i686-elf-g++
AS = i686-elf-as
AR = i686-elf-ar

ARCH = arch/i686

ROOT = ../root/

LIBK_CCFLAGS = -D__is_libk -D__is_kernel -std=gnu++11 -fbuiltin -ffreestanding -O0 -g -Wall -Wextra -fno-rtti -Wno-write-strings -fno-exceptions $(INCLUDE) --sysroot=$(ROOT) -nostdlib -m32 -mgeneral-regs-only

INCLUDE = -I $(ROOT)usr/include

LIBK_C = $(wildcard string/*.c) $(wildcard stdio/*.c) $(wildcard stdlib/*.c)
HEADERS = $(wildcard include/*.h)

BINARIES = libk.a
LIBK_OBJ = $(LIBK_C:.c=.o)

.PHONY: all clean install install-headers install-binaries

all: install-headers $(BINARIES) install-binaries

libk.a: $(LIBK_OBJ)
	$(AR) rcs ../build/$@ $(addprefix ../build/,$(notdir $(LIBK_OBJ)))

%.o: %.c
	$(CC) -c $< -o ../build/$(notdir $@) $(LIBK_CCFLAGS)

clean:
	rm -f $(addprefix $(ROOT)usr/bin/, $(BINARIES))

install: install-headers install-binaries

install-headers:
	cp -Rv $(HEADERS) $(ROOT)usr/include

install-binaries:
	cp $(addprefix ../build/,$(BINARIES)) $(ROOT)usr/lib
