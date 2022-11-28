#include "types.h"
#include "iso9660.h"
#include "file.h"
#include "filesystem.h"
#include <stdio.h>

extern "C" void module_load(fs::File* root, const char* argv)
{
	fs::registry->register_filesystem_implementation("ISO9660", [](fs::File* root, size_t offset, size_t length) {
		new Iso9660FileSystem(root, offset, length);
		return true;
	});
}
