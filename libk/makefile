#structure: folders never start or end with /

CC = i686-elf-g++
AS = i686-elf-as
AR = i686-elf-ar

ARCH = arch/i686

ROOT = ../root/
BUILD = ../build/

LIBK_CCFLAGS = -D__is_libk -D__is_kernel -std=gnu++11 -fbuiltin -ffreestanding -O0 -g -Wall -Wextra -fno-rtti -Wno-write-strings -fno-exceptions $(INCLUDE) --sysroot=$(ROOT) -nostdlib -m32 -mgeneral-regs-only -MD

INCLUDE = -I $(ROOT)usr/include

LIBK_C = $(wildcard string/*.c) $(wildcard stdio/*.c) $(wildcard stdlib/*.c)
HEADERS = $(wildcard include/*.h)

BINARIES = libk.a
LIBK_OBJ = $(addprefix $(BUILD), $(notdir $(LIBK_C:.c=.o)))
DEPS = $(LIBK_OBJ:.o=.d)

.PHONY: all clean install
all: $(BUILD)$(BINARIES) install


$(BUILD)%.a: $(LIBK_OBJ)
	$(AR) rcs ../build/$@ $(addprefix ../build/,$(notdir $(LIBK_OBJ)))

$(BUILD)%.o: string/%.c
	$(CC) -c $< -o $@ $(LIBK_CCFLAGS)
$(BUILD)%.o: stdio/%.c
	$(CC) -c $< -o $@ $(LIBK_CCFLAGS)
$(BUILD)%.o: stdlib/%.c
	$(CC) -c $< -o $@ $(LIBK_CCFLAGS)

clean:
	rm -f $(BUILD)$(BINARIES) $(LIBK_OBJ)

install: $(addprefix $(ROOT)usr/, $(HEADERS)) $(ROOT)usr/lib/$(BINARIES)

$(ROOT)usr/%.h: %.h
	cp $^ $@

$(ROOT)usr/lib/%: $(BUILD)%
	cp $^ $@

-include $(DEPS)
