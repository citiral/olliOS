#ifndef __OLLIOS_CMOS_H
#define __OLLIOS_CMOS_H

#include "eventbus/eventbus.h"

class CMOS : EventListener
{
public:
	CMOS();
	bool handleEvent(Event& event);

private:
	void selectRegister(char r);
	char read(char r);
	char readBCD(char r);
	char readRTC(char r);
	void write(char r, char v);

	void respondTime(Event& event);

	char bcdToBinary(char bcd);

	bool rtcIsBCD();
	bool rtcIs24();

	BusDevice& _bus;
};

struct GetTimeEvent : Event
{
	virtual void prepare(EventTarget target)
	{
		type = GETTIME;
		Event::prepare(target);
	}
};

struct TimeResponse : Response
{
	int seconds;
	int minutes;
	int hours;
	int day;
	int month;
	int year;

	virtual void prepare(EventTarget target)
	{
		type = GETTIME;
		Response::prepare(target);
	}
};

#endif