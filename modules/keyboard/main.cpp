#include "types.h"
#include "keyboard.h"
#include "fs/bindings.h"
#include "threading/scheduler.h"
#include <stdio.h>

using namespace bindings;
using namespace keyboard;

void KeyboardDriverThread(KeyboardDriver* driver, OwnedBinding* keyboard) {
	while (1) {
		driver->dataMutex.lock();

		while (true) {
			VirtualKeyEvent event;
			size_t read = driver->read(&event, sizeof(VirtualKeyEvent));

			if (read == 0) {
				break;
			}

            keyboard->provide(&event, sizeof(event));
		}
	}
}

extern "C" void module_load(Binding* root)
{
    OwnedBinding* keyboard = root->get("sys")->add(new OwnedBinding("keyboard"));

	driver = new KeyboardDriver();
	driver->setScanCodeSet(SCANSET_2);
	driver->setScanCodeTranslation(false);

	threading::scheduler->schedule(new threading::Thread(KeyboardDriverThread, driver, keyboard));
}
