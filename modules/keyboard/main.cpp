#include "types.h"
#include "keyboard.h"
#include "bindings.h"
#include "file.h"
#include "virtualfile.h"
#include "threading/scheduler.h"
#include <stdio.h>

using namespace fs;
using namespace keyboard;

void KeyboardDriverThread(KeyboardDriver* driver, Stream* keyboard) {
	static bool first = true;
	FileHandle* handle = keyboard->open();
	while (1) {
		driver->dataMutex.lock();

		while (true) {
			VirtualKeyEvent event;
			size_t read = driver->read(&event, sizeof(VirtualKeyEvent));

			if (read == 0) {
				break;
			}

			handle->write(&event, sizeof(event));
		}
	}
}

extern "C" void module_load(File* root, const char* argv)
{
	Stream* keyboardStream = new ChunkedStream("keyboard", sizeof(VirtualKeyEvent) * 128, sizeof(VirtualKeyEvent));
    File* keyboard = root->get("sys")->bind(keyboardStream);

	driver = new KeyboardDriver();
	driver->setScanCodeSet(SCANSET_2);
	driver->setScanCodeTranslation(false);

	threading::scheduler->schedule(new threading::Thread(nullptr, nullptr, KeyboardDriverThread, driver, keyboardStream));
}
