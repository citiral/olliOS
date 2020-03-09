#include "types.h"
#include "bindings.h"
#include "pci.h"
#include <stdio.h>

using namespace bindings;

extern "C" void module_load(Binding* root, const char* argv)
{
    PCI::init(root->get("sys")->add(new OwnedBinding("pci")));
}
