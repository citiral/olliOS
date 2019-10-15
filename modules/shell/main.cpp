#include "types.h"
#include "eventbus/eventbus.h"
#include "eventbus/eventconsumer.h"
#include <stdio.h>

static void startup_listener(void* context, Event* event)
{
    printf("shell started\n");
}

static void on_keyboard_data(void* context, Event* event)
{
    u16* kbevent = (u16*) event->data;
    printf("data: %c\n", *kbevent);
}

extern "C" void module_load(EventConsumer* bus)
{
    bus->listen(EVENT_TYPE_KEYBOARD, nullptr, on_keyboard_data);
    bus->listen(EVENT_TYPE_STARTUP, nullptr, startup_listener);
}
