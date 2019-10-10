#include "eventbus/eventconsumer.h"


EventConsumer::EventConsumer(): events_lock(1), events_count(0), listeners_lock(1) 
{

}

void EventConsumer::push_event(Event* event)
{
    events_lock.lock();
    events.push(event);
    events_lock.release();
    events_count.release();
}

void EventConsumer::listen(u32 type, void* context, decltype(EventListener::callback) callback)
{
    EventListener listener;
    listener.type = type;
    listener.context = context;
    listener.callback = callback;
    listener.oneshot = false;

    listeners_lock.lock();
    listeners.push_back(listener);
    listeners_lock.release();
}

void EventConsumer::listen_once(u32 type, void* context, decltype(EventListener::callback) callback)
{
    EventListener listener;
    listener.type = type;
    listener.context = context;
    listener.callback = callback;
    listener.oneshot = true;

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
                    goto end_loop;
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
end_loop:

        // Optionally destroy the event
        int lifetime = __atomic_sub_fetch(&event->lifetime, 1, __ATOMIC_SEQ_CST);

        if (lifetime == 0) {
            delete event;
        }
    }
}
