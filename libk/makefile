#structure: folders never start or end with /

CC = i686-elf-gcc
AS = i686-elf-as
AR = i686-elf-ar

ARCH = arch/i686

ROOT = ../root/

LIBK_CCFLAGS = -D__is_libk -D__is_kernel -std=gnu++11 -fbuiltin -ffreestanding -O2 -g -Wall -Wextra -fno-rtti -Wno-write-strings -fno-exceptions $(INCLUDE) --sysroot=$(ROOT) -nostdlib

INCLUDE = -I $(ROOT)usr/include

LIBK_CPP = $(wildcard string/*.cpp) $(wildcard stdio/*.cpp) $(wildcard stdlib/*.cpp)
HEADERS = $(wildcard include/*.h)

BINARIES = libk.a
LIBK_OBJ = $(LIBK_CPP:.cpp=.o)

.PHONY: all clean install install-headers install-binaries

all: install-headers $(BINARIES) install-binaries

libk.a: $(LIBK_OBJ)
	$(AR) rcs ../build/$@ $(addprefix ../build/,$(notdir $(LIBK_OBJ)))

%.o: %.cpp
	$(CC) -c $< -o ../build/$(notdir $@) $(LIBK_CCFLAGS)

clean:
	rm -f $(addprefix $(ROOT)usr/bin/, $(BINARIES))

install: install-headers install-binaries

install-headers:
	cp -Rv $(HEADERS) $(ROOT)usr/include

install-binaries:
	cp $(addprefix ../build/,$(BINARIES)) $(ROOT)usr/lib
