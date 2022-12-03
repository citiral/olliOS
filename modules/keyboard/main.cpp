#include "types.h"
#include "keyboard.h"
#include "filesystem/file.h"
#include "filesystem/stream.h"
#include "filesystem/chunkedstream.h"
#include "threading/scheduler.h"
#include <stdio.h>

using namespace keyboard;

void KeyboardDriverThread(KeyboardDriver* driver, fs::Stream* keyboard) {
	fs::FileHandle* handle = keyboard->open();
	while (1) {
		driver->dataMutex.lock();

		while (true) {
			VirtualKeyEvent event;
			size_t read = driver->read(&event, sizeof(VirtualKeyEvent));

			if (read == 0) {
				break;
			}
			
			handle->write(&event, sizeof(event), 0);
		}
	}
}

extern "C" void module_load(fs::File* root, const char* argv)
{
	UNUSED(argv);

	fs::Stream* keyboardStream = new fs::ChunkedStream("keyboard", sizeof(VirtualKeyEvent) * 128, sizeof(VirtualKeyEvent));
    root->get("sys")->bind(keyboardStream);

	driver = new KeyboardDriver();
	driver->init();
	driver->setScanCodeSet(SCANSET_2);
	//driver->setScanCodeTranslation(false);

	threading::scheduler->schedule(new threading::Thread(nullptr, nullptr, KeyboardDriverThread, driver, keyboardStream));
}
