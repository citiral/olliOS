#ifndef __OLLIOS_EVENTBUS_H
#define __OLLIOS_EVENTBUS_H

#include "kstd/vector.h"
#include "eventdef.h"

class BusDevice;
class EventListener;
struct Event;

// An event bus
class EventBus
{
public:
	EventBus();
	~EventBus();

	// Registers a new event listener 
	BusDevice& registerListener(EventListener& listener);
	void fireEvent(BusDevice& device, Event& event);

private:
	std::vector<BusDevice*> _devices;
};

// A device on the event bus
class BusDevice
{
public:
	BusDevice(EventBus& bus, EventListener& listener, const int id);
	~BusDevice();

	void fireEvent(Event& event);
	bool handleEvent(Event& event);
	int id();

private:
	EventBus& _bus;
	EventListener& _listener;
	const int _id;
};

// A class that can listen to an event.
class EventListener
{
public:
	// Try to handle an event. Should return false if it can't handle the event, should return true if it can.
	virtual bool handleEvent(Event& event) = 0;
};

// Represents an event that can be sent.
struct Event
{
	EventType type; // The type of request
	EventTarget target; // Device that is target by the request
	EventTarget sender; // Device that sent the request

	size_t payloadSize = 0; // Size of the event that was sent.
};

// Represents a response that can be sent back after an event was received
struct Response : Event
{

};

extern EventBus kernelBus;

#endif