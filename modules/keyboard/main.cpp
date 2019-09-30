#include "eventbus/eventbus.h"
#include <stdio.h>

static void startup_listener(void* context, u32 type, u32 source, u32 destination, u32 size, void* data)
{
    printf("Hello world from startup listener!\n");
}

extern "C" void module_load()
{
    eventbus.registerListener(EVENT_TYPE_STARTUP, EVENT_TARGET_MAIN, EVENT_TARGET_MAIN, nullptr, startup_listener);
}
