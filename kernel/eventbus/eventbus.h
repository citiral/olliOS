#ifndef __OLLIOS_EVENTBUS_H
#define __OLLIOS_EVENTBUS_H

#include "kstd/unordered_map.h"
#include "kstd/vector.h"
#include "kstd/linkedlist.h"
#include "eventdef.h"
#include "types.h"
#include "threading/semaphore.h"
#include "threading/spinlock.h"


struct EventListener {
public:
	u32 type;
	u32 sourceMask;
	u32 destinationMask;
	void (*callback)(void* context, u32 type, u32 source, u32 destination, u32 size, void* data);
	void* context;
	EventListener *next;
};


class EventBus {
public:
	EventBus();
	void pushEvent(u32 type, u32 source, u32 destination, u32 size, void* data);
	void registerListener(u32 type, u32 sourceMask, u32 destinationMask, void* context, decltype(EventListener::callback) callback);

private:
	void eventEntry(u32 type, u32 source, u32 destination, u32 size, void* data);

private:
	EventListener *listeners;
	EventListener *listeners_last;
};

extern EventBus eventbus;

#endif
