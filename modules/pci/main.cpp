#include "eventbus/eventconsumer.h"
#include "pci.h"
#include <stdio.h>

static void startup_listener(void* context, Event* event)
{
    printf("Hello world from pci!\n");
    ata::driver.initialize();
}

extern "C" void module_load(EventConsumer* bus)
{
    bus->listen(EVENT_TYPE_STARTUP, nullptr, startup_listener);
}
