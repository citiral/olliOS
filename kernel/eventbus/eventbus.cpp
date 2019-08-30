#include "eventbus.h"
#include "threading/scheduler.h"

EventBus::EventBus()
{

}

void EventBus::pushEvent(u32 type, u32 source, u32 destination, u32 size, void* data)
{
    threading::scheduler->schedule(new threading::Thread(&EventBus::eventEntry, this, type, source, destination, size, data));
}


void EventBus::registerListener(u32 type, u32 sourceMask, u32 destinationMask, void* context, decltype(EventListener::callback) callback)
{
    eventlock.lock();
    listeners[type].push_back(EventListener {
        .type = type,
        .sourceMask = sourceMask,
        .destinationMask = destinationMask,
        .callback = callback,
        .context = context,
    });
    eventlock.release();
}

void EventBus::eventEntry(u32 type, u32 source, u32 destination, u32 size, void* data)
{
    eventlock.lock();
    std::vector<EventListener> typelisteners = listeners[type];
    int count = typelisteners.size();
    eventlock.release();

    for (int i = 0 ; i < count ; i++) {
        if (((typelisteners[i].sourceMask & source) != 0) && ((typelisteners[i].destinationMask & destination) != 0)) {
            typelisteners[i].callback(typelisteners[i].context, type, source, destination, size, data);
        }
    }
}