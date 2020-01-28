#structure: folders never start /, always end with /

CC = i686-elf-g++
AS = i686-elf-as
ARCH = arch/i686/
MAKE = make

ROOT = ../root/
BUILD = ../build/
OUTDIR = $(BUILD)$(MODULE)/

INCLUDE = -I $(ROOT)usr/include
CCFLAGS = -D__is_kernel -std=gnu++11 -ffreestanding -O0 -Wall -Wextra -fno-exceptions -fno-rtti $(INCLUDE) -Wno-write-strings --sysroot=$(ROOT) -nostdlib -fno-threadsafe-statics -Werror=return-type -m32 -g -mgeneral-regs-only -MD
LDFLAGS =  -std=gnu++11 -Wall -Wextra -nostdlib -m32 -r -Xlinker '-Ur'
LIBS = $(ROOT)/usr/lib/libk.a

MODULE_CPP = $(wildcard $(MODULE)/*.cpp)
HEADERS = $(wildcard $(MODULE)/*.h)
OBJECTS = $(addprefix $(OUTDIR), $(notdir $(MODULE_CPP:.cpp=.o)))

DEPS = $(OBJECTS:.o=.d)

-include $(DEPS)

.phony: all clean install

all: $(addprefix $(ROOT)usr/include/$(MODULE)/, $(notdir $(HEADERS))) $(OUTDIR) $(BUILD)$(MODULE).so install

clean:
	rm -fr $(OUTDIR)
	rm ollios.iso

install: $(ROOT)boot/$(MODULE).so $(addprefix $(ROOT)usr/include/$(MODULE)/, $(notdir $(HEADERS)))

$(ROOT)boot/%.so: $(BUILD)%.so
	cp $^ $@
	
$(ROOT)usr/include/$(MODULE)/%.h: $(MODULE)/%.h
	mkdir $(shell dirname $@) -p
	cp $^ $@

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(BUILD)$(MODULE).so: $(OBJECTS)
	$(CC) -fpic -o $@ $^ $(LDFLAGS)

$(OUTDIR)%.o: $(MODULE)/%.cpp
	$(CC) -c $< -o $@ $(CCFLAGS)
