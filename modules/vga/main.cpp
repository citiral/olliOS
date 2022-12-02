#include "file.h"
#include "txt_mode.h"

extern "C" void module_load(fs::File* root, const char* argv)
{
    auto driver = new TxtModeDriver(root, multiboot::instance);
}