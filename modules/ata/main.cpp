#include "eventbus/eventconsumer.h"
#include "ata.h"
#include <stdio.h>

static void startup_listener(void* context, Event* event)
{
    printf("Hello world from ata!\n");
    ata::driver.initialize();
}

extern "C" void module_load(EventConsumer* bus)
{
    bus->listen(EVENT_TYPE_STARTUP, nullptr, startup_listener);
}
