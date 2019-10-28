#include "types.h"
#include "kernelshell.h"
#include "eventbus/eventbus.h"
#include "eventbus/eventconsumer.h"
#include <stdio.h>

KernelShell shell;

static void startup_listener(void* context, Event* event)
{
    printf("shell started\n");
}

static void on_keyboard_data(void* context, Event* event)
{
    static bool skip = true;
    if (skip) {
        skip = false;
        return;
    }
    VirtualKeyEvent* kbevent = (VirtualKeyEvent*) event->data;

    shell.enter(*kbevent);
    //printf("data: %c\n", *kbevent);
}

extern "C" void module_load(EventConsumer* bus)
{
    new (&shell) KernelShell();

    //bus->listen(EVENT_TYPE_KEYBOARD, nullptr, on_keyboard_data);
    bus->listen(EVENT_TYPE_KEYBOARD, nullptr, [](void* context, Event* event) {
        VirtualKeyEvent* kbevent = (VirtualKeyEvent*) event->data;
        shell.enter(*kbevent);
    });
    bus->listen(EVENT_TYPE_STARTUP, nullptr, startup_listener);
}
