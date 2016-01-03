#structure: folders never start or end with /

CC = i686-elf-gcc
AS = i686-elf-as
AR = i686-elf-ar

ARCH = arch/i686

ROOT = ../root/

LIBC++_CCFLAGS = -D__is_libcpp -D__is_kernel -std=gnu++11 -O2 -g -Wall -Wextra -fno-exceptions $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -fdiagnostics-color=auto -nostdlib
LIBK++_CCFLAGS = -D__is_libkpp -D__is_kernel -std=gnu++11 -fbuiltin -ffreestanding -O2 -g -Wall -Wextra -fno-exceptions $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -fdiagnostics-color=auto -nostdlib
LIBSC = $(ROOT)/usr/lib/libc.a
LIBSK = $(ROOT)/usr/lib/libk.a

INCLUDE = -I $(ROOT)usr/include

LIBC++_CPP = $(wildcard abi/*.cpp)
LIBK++_CPP = $(wildcard abi/*.cpp)
HEADERS = $(wildcard include/*.h)

BINARIES = libc++.a libk++.a
LIBC++_OBJ = $(LIBC++_CPP:.cpp=.o)
LIBK++_OBJ = $(LIBK++_CPP:.cpp=.libk++.o)

.PHONY: all clean install install-headers install-binaries

all: install-headers $(BINARIES) install-binaries

libc++.a: $(LIBC++_OBJ)
	$(AR) rcs ../build/$@ $(addprefix ../build/,$(notdir $(LIBC++_OBJ))) $(LIBSC)

libk++.a: $(LIBK++_OBJ)
	$(AR) rcs ../build/$@ $(addprefix ../build/,$(notdir $(LIBK++_OBJ))) $(LIBSK)

%.o: %.cpp
	$(CC) -c $< -o ../build/$(notdir $@) $(LIBC++_CCFLAGS)

%.libk++.o: %.cpp
	$(CC) -c $< -o ../build/$(notdir $@) $(LIBK++_CCFLAGS)

clean:
	rm -f $(addprefix $(ROOT)usr/bin/, $(BINARIES))

install: install-headers install-binaries

install-headers:
	cp -Rv $(HEADERS) $(ROOT)usr/include

install-binaries:
	cp $(addprefix ../build/,$(BINARIES)) $(ROOT)usr/lib
