#include "fs/bindings.h"
#include <stdio.h>

using namespace bindings;


extern "C" void module_load(Binding* root)
{
    root->get("dev")->enumerate([](Binding* parent, Binding* child) {
        printf("Device %s found\n", child->name.c_str());
        return true;
    }, true);
}
