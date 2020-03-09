#include "types.h"
#include "iso9660.h"
#include "bindings.h"
#include <stdio.h>

using namespace bindings;

extern "C" void module_load(Binding* root, const char* argv)
{
	root->get("dev")->enumerate([](bindings::Binding* parent, bindings::Binding* child) {
		new Iso9660FileSystem(child);
		return true;
	}, true);
}
