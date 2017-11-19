#include "nulllistener.h"

#include "cdefs.h"

NullListener::NullListener() : _bus(kernelBus.registerListener(*this))
{
}

int NullListener::fireEvent(Event& event)
{
	return _bus.fireEvent(event);
}

Response* NullListener::fireEventAndWait(Event& event)
{
	return _bus.fireEventAndWait(event);
}

bool NullListener::hasResponse()
{
	return _bus.hasResponse();
}

Response* NullListener::getResponse()
{
	return _bus.getResponse();
}

int NullListener::id()
{
	return _bus.id();
}

bool NullListener::handleEvent(Event& event)
{
	return false;
}