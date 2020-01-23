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

            keyboard->provide(sizeof(event), &event);
		}
	}
}

static int TestInteger1 = 0;
static int TestInteger2 = 0;
static int TestInteger3 = 0;

extern "C" void module_load(Binding* root)
{
    OwnedBinding* keyboard = root->create("keyboard");
	printf("TestInteger1, &%X, %X\n", &TestInteger1, TestInteger1);
	printf("TestInteger2, &%X, %X\n", &TestInteger2, TestInteger2);
	printf("TestInteger3, &%X, %X\n", &TestInteger3, TestInteger3);
    printf("D1:%X, %X\n", &driver, driver);
	driver = new KeyboardDriver();
	driver->setScanCodeSet(SCANSET_2);
	driver->setScanCodeTranslation(false);

	threading::scheduler->schedule(new threading::Thread(KeyboardDriverThread, driver, keyboard));
}
