#include "bindings.h"
#include "ata.h"
#include <stdio.h>

using namespace bindings;

extern "C" void module_load(Binding* root, const char* argv)
{
    root->get("sys")->enumerate([](Binding* root, Binding* child) {
        if (child->name == "pci") {
            printf("initing ata\n");
            ata::driver.initialize(child);
            return false;
        }
        return true;
    }, true);
}
