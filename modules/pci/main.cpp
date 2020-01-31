#include "types.h"
#include "fs/bindings.h"
#include "pci.h"
#include <stdio.h>

using namespace bindings;

extern "C" void module_load(Binding* root)
{
    PCI::init(root->get("sys")->add(new OwnedBinding("pci")));
}
