#include "types.h"
#include "iso9660.h"
#include "file.h"
#include <stdio.h>

extern "C" void module_load(fs::File* root, const char* argv)
{
	fs::File* dev = root->get("dev");
	fs::FileHandle* desc = dev->open();
	
	fs::File* child = nullptr;

	while ((child = desc->next_child()) != nullptr) {
		new Iso9660FileSystem(child);
	}

	desc->close();
}
