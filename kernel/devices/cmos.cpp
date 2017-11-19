#include "cmos.h"

#include "io.h"
#include "cdefs.h"

CMOS::CMOS() : _bus(kernelBus.registerListener(*this))
{
}

bool CMOS::handleEvent(Event& event)
{
	if (event.type == GETTIME)
	{
		respondTime(event);
		return true;
	}
	else
	{
		return false;
	}
}

void CMOS::respondTime(Event& event)
{
	TimeResponse* response = new TimeResponse();

	response->seconds = readRTC(0);
	response->minutes = readRTC(2);
	response->hours = readRTC(4);
	response->day = readRTC(7);
	response->month = readRTC(8);
	response->year = ((int) readRTC(9)) + ((int) readRTC(0x32)) * 100; 

	_bus.respond(event, response);
}

char CMOS::bcdToBinary(char bcd)
{
	return ( (bcd & 0xF0) >> 1) + ( (bcd & 0xF0) >> 3) + (bcd & 0xf);
}

char CMOS::read(char r)
{
	selectRegister(r);
	return inb(0x71);
}

char CMOS::readBCD(char r)
{
	return bcdToBinary(read(r));
}

char CMOS::readRTC(char r)
{
	if (rtcIsBCD())
		return readBCD(r);
	else
		return read(r);
}

void CMOS::write(char r, char v)
{
	UNUSED(r);
	UNUSED(v);
	CPU::panic("Not going to write");
}

void CMOS::selectRegister(char r)
{
	// This does disable NMIs
	//outb(0x70, 0x80 | r);
	outb(0x70, r);
}

bool CMOS::rtcIsBCD()
{
	char status = read(0xB);
	return (status & 4) == 0;
}

bool CMOS::rtcIs24()
{
	char status = read(0xB);
	return (status & 2) > 0;
}