#include "eventbus.h"
#include "threading/scheduler.h"

EventBus::EventBus(): listeners(nullptr), listeners_last(nullptr)
{
    
}

void EventBus::pushEvent(u32 type, u32 source, u32 destination, u32 size, void* data)
{
    threading::scheduler->schedule(new threading::Thread(&EventBus::eventEntry, this, type, source, destination, size, data));
}


void EventBus::registerListener(u32 type, u32 sourceMask, u32 destinationMask, void* context, decltype(EventListener::callback) callback)
{
    EventListener* listener = new EventListener;
    listener->type = type;
    listener->sourceMask = sourceMask;
    listener->destinationMask = destinationMask;
    listener->callback = callback;
    listener->context = context;
    listener->next = nullptr;

    if (listeners_last == nullptr) {
        listeners = listener;
        listeners_last = listener;       
    } else {
        listeners_last->next = listener;
        listeners_last = listener;
    }
}

void EventBus::eventEntry(u32 type, u32 source, u32 destination, u32 size, void* data)
{
    EventListener *listener = listeners;

    while (listener != nullptr) {
        if (((listener->sourceMask & source) != 0) && ((listener->destinationMask & destination) != 0)) {
            listener->callback(listener->context, type, source, destination, size, data);
        }
    }
}
