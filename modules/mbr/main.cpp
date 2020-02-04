#include "bindings.h"
#include <stdio.h>

using namespace bindings;

void check_mbr(Binding* bind)
{
    u8 header[512];

    // Read the mbr header
    bind->read(header, 512, 0);

    if (header[0x1BC] != 0 || header[0x1BD] != 0) {
        printf("Reserved not zero!\n");
        return;
    }

    if (header[0x1FE] != 0x55 || header[0x1FF] != 0xAA) {
        printf("Header not valid %X %X!\n", header[0x1FE], header[0x1FF]);
        return;
    }

    printf("found mbr\n");
}

extern "C" void module_load(Binding* root)
{
    root->get("dev")->enumerate([](Binding* parent, Binding* child) {
        printf("Device %s found\n", child->name.c_str());
        check_mbr(child);
        return true;
    }, true);
}
