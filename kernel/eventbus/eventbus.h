#ifndef __OLLIOS_EVENTBUS_H
#define __OLLIOS_EVENTBUS_H

#include "kstd/vector.h"
#include "kstd/linkedlist.h"
#include "eventdef.h"
#include "types.h"

struct EventData {
public:
	u32 size;
	u32 source;
	u32 type;
};


class EventBus {
public:

private:
	LinkedList<EventData> events;
};

#endif