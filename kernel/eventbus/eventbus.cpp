#include "eventbus.h"

#include "cdefs.h"
#include "cpu.h"

EventBus kernelBus;

EventBus::EventBus()
{
}

EventBus::~EventBus()
{
	size_t size = _devices.size();
	for (size_t i = 0; i < size; i++)
	{
		BusDevice* device = _devices[i];
		delete device;
	}
}

BusDevice& EventBus::registerListener(EventListener& listener)
{
	BusDevice* device = new BusDevice(*this, listener, _devices.size() + 1);
	_devices.push_back(device);
	return *device;
}

int EventBus::fireEvent(BusDevice& device, Event& event)
{
	setHeaders(event, device);

	// Add synchronization when atomic_boolean exists

	// Only loop over all the devices if no specific target was specified
	if (event.target == TARGET_ANY || event.target == TARGET_ALL)
	{
		size_t size = _devices.size();
		for (size_t i = 0; i < size; i++)
		{
			BusDevice* device = _devices[i];
			// If target == TARGET_ANY it should break if the event was processed.
			if (device->handleEvent(event) && event.target == TARGET_ANY)
			{
				return device->id();
			}
		}

		// If target == TARGET_ANY the function will have already returned if a handler was found
		if (event.target == TARGET_ANY)
		{
			return 0;
		}
		else
		{
			return TARGET_ALL;
		}
	}
	else
	{
		if (!_devices[event.target-1]->handleEvent(event))
		{
			return 0;
		}
		else
		{
			return event.target;
		}
	}
}

void EventBus::fireResponse(BusDevice& device, Response* response)
{
	setHeaders(*response, device);
	_devices[response->target-1]->handleResponse(response);
}

void EventBus::setHeaders(Event& event, BusDevice& device)
{
	event.sender = device.id();
}

BusDevice::BusDevice(EventBus& bus, EventListener& listener, const int id) : _bus(bus), _listener(listener), _id(id)
{
}

BusDevice::~BusDevice()
{
	size_t size = _responses.size();
	for (size_t i = 0; i < size; i++)
	{
		delete _responses[i];
	}
}

int BusDevice::fireEvent(Event& event)
{
	return _bus.fireEvent(*this, event);
}

void BusDevice::respond(Event& event, Response* response)
{
	response->target = event.sender;
	_bus.fireResponse(*this, response);
}

bool BusDevice::handleEvent(Event& event)
{
	return _listener.handleEvent(event);
}

void BusDevice::handleResponse(Response* response)
{
	_responses.push_back(response);
}

bool BusDevice::hasResponse()
{
	return _responses.size() > 0;
}

Response* BusDevice::getResponse()
{
	Response* response;
	if (hasResponse())
	{
		response = _responses[0];
		_responses.erase(0);
	}
	else
	{
		CPU::panic("No response");
	}
	return response;
}

Response* BusDevice::fireEventAndWait(Event& event)
{
	fireEvent(event);
	return getResponse();
}

int BusDevice::id()
{
	return _id;
}