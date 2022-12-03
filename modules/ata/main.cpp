#include "filesystem/file.h"
#include "ata.h"
#include <stdio.h>

extern "C" void module_load(fs::File* root, const char* argv)
{
    fs::File* pci = root->get("sys/pci");
    
    if (pci) {
            ata::driver.initialize(pci);
    } else {
        printf("Error: sys/pci not found.\n");
    }
}
