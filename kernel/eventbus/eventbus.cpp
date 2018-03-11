#include "eventbus.h"

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
	BusDevice* device = new BusDevice(*this, listener, _devices.size());
	_devices.push_back(device);
	return *device;
}

void EventBus::fireEvent(BusDevice& device, Event& event)
{
	event.sender = device.id();

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
				return;
			}
		}

		// If target == TARGET_ANY the function will have already returned if a handler was found
		if (event.target == TARGET_ANY)
		{
			CPU::panic("No device could handle event");
		}
	}
	else
	{
		if (!_devices[event.target]->handleEvent(event))
		{
			CPU::panic("Device could not handle event");
		}
	}
}

BusDevice::BusDevice(EventBus& bus, EventListener& listener, const int id) : _bus(bus), _listener(listener), _id(id)
{
}

BusDevice::~BusDevice()
{
}

void BusDevice::fireEvent(Event& event)
{
	_bus.fireEvent(*this, event);
}

bool BusDevice::handleEvent(Event& event)
{
	return _listener.handleEvent(event);
}

int BusDevice::id()
{
	return _id;
}