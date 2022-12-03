#include "types.h"
#include "pci.h"
#include "filesystem/file.h"
#include <stdio.h>

extern "C" void module_load(fs::File* root, const char* argv)
{
    UNUSED(argv);
    fs::File* pci = root->get("sys")->create("pci", FILE_CREATE_DIR);

    PCI::init(pci);
}
