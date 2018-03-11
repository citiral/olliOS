#ifndef __OLLIOS_EVENTBUSTEST_H
#define __OLLIOS_EVENTBUSTEST_H

#include "eventbus.h"

class EventBusTest : EventListener
{
public:
	EventBusTest();
	void sendTestEvent(int type);
	bool handleEvent(Event& event);

private:
	BusDevice& _bus;
};

#endif