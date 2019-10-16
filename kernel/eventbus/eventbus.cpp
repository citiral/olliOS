#include "eventbus.h"
#include "eventbus/eventconsumer.h"
#include "threading/scheduler.h"

EventBus eventbus;

EventBus::EventBus(): consumers_lock(1)
{
    
}

void EventBus::emit(u32 type, u32 size, void* data)
{

    // Make new event
    Event *event = (Event*) malloc(sizeof(Event) + size);
    event->type = type;
    memcpy(event + 1, data, size);

    // Push it to all consumers
    consumers_lock.lock();
    event->lifetime = consumers.size();
    for (size_t i = 0 ; i < consumers.size() ; i++) {
        consumers[i]->push_event(event);
    }
    consumers_lock.release();
}

EventConsumer* EventBus::create_consumer()
{
    consumers_lock.lock();
    EventConsumer* consumer = new EventConsumer();
    consumers.push_back(consumer);
    consumers_lock.release();
    return consumer;
}
