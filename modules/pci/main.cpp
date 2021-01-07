#include "types.h"
#include "pci.h"
#include "file.h"
#include "virtualfile.h"
#include <stdio.h>

extern "C" void module_load(fs::File* root, const char* argv)
{
    fs::File* pci = root->get("sys")->create("pci", FILE_CREATE_DIR);

    PCI::init(pci);
}
