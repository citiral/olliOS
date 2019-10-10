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

void EventConsumer::registerListener(u32 type, void* context, decltype(EventListener::callback) callback, bool oneshot)
{
    EventListener listener;
    listener.type = type;
    listener.context = context;
    listener.callback = callback;
    listener.oneshot = oneshot;

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
        size_t i = 0;
        EventListener listener;

        while (true) {
            // Always lock during listener list operations
            listeners_lock.lock();

            // Get the next listener qualifying for the event
            do {
                // Break if we went through the list
                if (i >= listeners.size()) {
                    listeners_lock.release();
                    break;
                }

                // Get the next listener, and break if it matches the event
                listener = listeners[i];
                i++;
            } while (listener.type != event->type);

            // If it is a oneshot, remove it from the list
            if (listener.oneshot) {
               i--;
               listeners.erase(i); 
            }

            // Release the listener lock to allow adding listeners during an event
            listeners_lock.release();

            // And run the event
            listener.callback(listener.context, event);
        }

        for (size_t i = 0 ; i < listeners.size() ; i++) {
            if (listeners[i].type == event->type) {
                listeners[i].callback(listeners[i].context, event);
            }
        }
        listeners_lock.release();
    }
}
