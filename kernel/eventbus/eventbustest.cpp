#include "eventbustest.h"

#include "cdefs.h"

EventBusTest::EventBusTest() : _bus(kernelBus.registerListener(*this))
{
}

void EventBusTest::sendTestEvent(int type)
{
	Event event;
	event.target = TARGET_ALL;
	event.type = type;

	_bus.fireEvent(event);
}

bool EventBusTest::handleEvent(Event& event)
{
	LOG_INFO("Received event");

	if (event.type == 1337)
	{
		LOG_INFO("Type was 1337, accepting");
		return true;
	}
	else
	{
		LOG_INFO("Type was not 1337, not accepting");
		return false;
	}
}