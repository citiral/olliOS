#include "eventbustest.h"

#include "cdefs.h"

EventBusTest::EventBusTest() : _bus(kernelBus.registerListener(*this))
{
}

void EventBusTest::sendTestEvent(int type)
{
	Event event;
	event.type = type;
	event.prepare(TARGET_ALL);

	_bus.fireEvent(event);
	Response* response = _bus.getResponse();
	LOG_INFO("Received response");
	delete response;
}

bool EventBusTest::handleEvent(Event& event)
{
	LOG_INFO("Received event");

	if (event.type == 1337)
	{
		LOG_INFO("Type was 1337, accepting");

		Response* response = new Response();
		response->type = 199;
		_bus.respond(event, response);

		return true;
	}
	else
	{
		LOG_INFO("Type was not 1337, not accepting");
		return false;
	}
}