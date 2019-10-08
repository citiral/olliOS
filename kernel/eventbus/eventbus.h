#ifndef __OLLIOS_EVENTBUS_H
#define __OLLIOS_EVENTBUS_H

#include "kstd/unordered_map.h"
#include "kstd/vector.h"
#include "kstd/linkedlist.h"
#include "eventdef.h"
#include "types.h"
#include "threading/semaphore.h"
#include "threading/spinlock.h"

#define EVENTBUS_RINGBUFFER_SIZE 512

class EventConsumer;

struct Event {
public:
    Event* next;
    u32 type;
    u32 lifetime;
    u8 data[0];
};


class EventBus {
public:
	EventBus();

	void pushEvent(u32 type, u32 size, void* data);
    EventConsumer* createConsumer();

private:
    std::vector<EventConsumer*> consumers;
    threading::Semaphore consumers_lock;
};

extern EventBus eventbus;

#endif
