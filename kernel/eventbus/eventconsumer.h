#ifndef __EVENTBUS_EVENTCONSUMER_H
#define __EVENTBUS_EVENTCONSUMER_H

#include "eventbus/eventbus.h"
#include "kstd/linkedlist.h"
#include "kstd/vector.h"

struct EventListener {
public:
	u32 type;
	void (*callback)(void* context, Event* event);
	void* context;
	EventListener *next;
};

class EventConsumer {
public:
    EventConsumer();
    void pushEvent(Event* event);
    void registerListener(u32 type, void* context, decltype(EventListener::callback) callback);

    void enter();

private:
    std::vector<EventListener> listeners;
    LinkedList<Event*> events;
    threading::Semaphore events_lock;
    threading::Semaphore events_count;
    threading::Semaphore listeners_lock;
};


#endif
