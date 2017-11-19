#ifndef __OLLIOS_NULLLISTENER_H
#define __OLLIOS_NULLLISTENER_H

#include "eventbus/eventbus.h"

class NullListener : EventListener
{
public:
	NullListener();

	int fireEvent(Event& event);
	bool hasResponse();
	Response* getResponse();
	Response* fireEventAndWait(Event& event);
	int id();

	bool handleEvent(Event& event);
private:
	BusDevice& _bus;
};

#endif