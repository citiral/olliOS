#include "vga/vga.h"
#include "types.h"
#include "fs/bindings.h"
#include <stdio.h>
#include "kstd/new.h"

using namespace bindings;

VgaDriver* vgaDriver = 0;

extern "C" void module_load(Binding* root)
{
    vgaDriver = new VgaDriver();
    root->create("vga", [](OwnedBinding* vga, size_t size, const void* data){
        (void) vga;
        vgaDriver->write(data, size);
        return true;
    });
}
