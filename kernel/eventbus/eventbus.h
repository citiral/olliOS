#ifndef __OLLIOS_EVENTBUS_H
#define __OLLIOS_EVENTBUS_H

#include "kstd/vector.h"
#include "eventdef.h"

class BusDevice;
class EventListener;
struct Event;
struct Response;

// An event bus
class EventBus
{
public:
	EventBus();
	~EventBus();

	// Registers a new event listener 
	BusDevice& registerListener(EventListener& listener);

	// Used by BusDevice
	int fireEvent(BusDevice& device, Event& event);
	void fireResponse(BusDevice& device, Response* response);

private:
	void setHeaders(Event& Event, BusDevice& device);

	std::vector<BusDevice*> _devices;
};

// A device on the event bus
class BusDevice
{
public:
	BusDevice(EventBus& bus, EventListener& listener, const int id);
	~BusDevice();

	// Used by EventBus
	bool handleEvent(Event& event);
	void handleResponse(Response* response);
	int id();

	// Used by event listeners
	int fireEvent(Event& event); // Fires an event and returns the id of the device that responded if target == TARGET_ANY
	void respond(Event& event, Response* response);
	bool hasResponse();
	Response* getResponse();
	Response* fireEventAndWait(Event& event);

private:
	EventBus& _bus;
	EventListener& _listener;
	const int _id;
	std::vector<Response*> _responses;
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
	/*Event(const Response& r) : type(r.type), target(r.target), r.s
	Event(EventType type) : type(type) {};
	Event(EventType type, EventTarget target) : type(type), target(target) {};*/

	EventType type; // The type of request
	EventTarget target; // Device that is target by the request
	EventTarget sender; // Device that sent the request

	virtual void prepare(EventTarget target)
	{
		this->target = target;
	}

	//size_t payloadSize = 0; // Number of bytes after this variable.
};

// Represents a response that can be sent back after an event was received
struct Response : Event
{
	virtual void prepare(EventTarget target)
	{
		Event::prepare(target);
	}
};

extern EventBus kernelBus;

#endif