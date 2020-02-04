#include "bindings.h"
#include "ata.h"
#include <stdio.h>

using namespace bindings;

extern "C" void module_load(Binding* root)
{
    root->get("sys")->enumerate([](Binding* root, Binding* child) {
        if (child->name == "pci") {
            ata::driver.initialize(child);
            return false;
        }
        return true;
    }, true);
}
