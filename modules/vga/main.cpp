#include "vga/vga.h"
#include "types.h"
#include "fs/bindings.h"
#include <stdio.h>
#include "kstd/new.h"

using namespace bindings;

VgaDriver vgaDriver;

extern "C" void module_load(Binding* root)
{
    new (&vgaDriver) VgaDriver();

    root->create("vga", [](OwnedBinding* vga, size_t size, const void* data){
        (void) vga;
        vgaDriver.write(data, size);
        return true;
    });
}
