#include "types.h"
#include "keyboard.h"
#include "eventbus/eventbus.h"
#include "eventbus/eventconsumer.h"
#include <stdio.h>

static void startup_listener(void* context, Event* event)
{
    keyboard::initialize();
}

extern "C" void module_load(EventConsumer* bus)
{
    bus->listen(EVENT_TYPE_STARTUP, nullptr, startup_listener);
}
