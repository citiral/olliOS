#include "eventbus.h"
#include "eventbus/eventconsumer.h"
#include "threading/scheduler.h"

EventBus eventbus;

EventBus::EventBus(): listeners_last(nullptr), _consumer_count(0), consumer_lock(1)
{
    
}

void EventBus::pushEvent(u32 type, u32 size, void* data)
{

    // Make new event
    Event *event = (Event*) malloc(sizeof(Event) + size);
    event->type = type;
    memcpy(event + 1, data, size);

    // Push it to all consumers
    consumer_lock.lock();
    event->lifetime = consumers.size();
    for (size_t i = 0 ; i < consumers.size() ; i++) {
        consumers[i]->pushEvent(event);
    }
    consumer_lock.release();
}

EventConsumer* EventBus::createConsumer()
{
    _consumer_lock.lock();
    EventConsumer* consumer = new EventConsumer();
    consumers.push_back(consumer);
    _consumer_lock.release();
    return consumer;
}
