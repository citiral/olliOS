#include "eventbus/eventconsumer.h"


EventConsumer::EventConsumer(): events_lock(1), events_count(0), listeners_lock(1), listeners()
{

}

void EventConsumer::pushEvent(Event* event)
{
    events_lock.lock();
    events.push(event);
    events_lock.release();
    events_count.release();
}

void EventConsumer::registerListener(u32 type, void* context, decltype(EventListener::callback) callback)
{
    EventListener listener;
    listener.type = type;
    listener.context = context;
    listener.callback = callback;

    listeners_lock.lock();
    listeners.push_back(listener);
    listeners_lock.release();
}

void EventConsumer::enter()
{
    while (true) {
        // Wait until an event becomes available
        events_count.lock(); 

        // Get the event
        events_lock.lock();
        Event* event = events.pop();
        events_lock.release();

        // Run all listeners
        listeners_lock.lock();
        for (size_t i = 0 ; i < listeners.size() ; i++) {
            if (listeners[i].type == event->type) {
                listeners[i].callback(listeners[i].context, event);
            }
        }
        listeners_lock.release();
    }
}
