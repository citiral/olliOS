//
// Created by Olivier on 30/09/16.
//

#include "ata.h"
#include "atapiodevice.h"
#include "cpu.h"
#include <string.h>

namespace ata {

AtaPioDevice::AtaPioDevice(bindings::Binding* ata, u16 port, unsigned short* data, int device): AtaDevice(ata, port, data, device) {
}

AtaPioDevice::~AtaPioDevice() {
}

size_t AtaPioDevice::write(const void* data, size_t amount)
{
	UNUSED(data);
	UNUSED(amount);
	CPU::panic("Write not implemented");
	return 0;
}

size_t AtaPioDevice::write(const void* data)
{
	UNUSED(data);
	CPU::panic("Write not implemented");
	return 0;
}

size_t AtaPioDevice::write(char data)
{
	UNUSED(data);
	CPU::panic("Write not implemented");
	return 0;
}

size_t AtaPioDevice::read(void* data, size_t amount, size_t offset)
{
	char* cdata = (char*) data;
	read(_pointer, amount, cdata);
	_pointer += amount;
	return 0;
}

size_t AtaPioDevice::seek(i32 offset, int position)
{
	if (position == SEEK_SET)
	{
		if (offset > 0)
			_pointer = offset;
	}
	else if (position == SEEK_CUR)
	{
		if (offset < 0 && -offset > position)
			_pointer = 0;
		else
			_pointer += offset;
	}
	else if (position == SEEK_END)
	{
		CPU::panic("NOT IMPLEMENTED");
	}
	return 0;
}

u8 AtaPioDevice::getStatus()
{
	return inB(PORT_STATUS);
}

void AtaPioDevice::flush()
{
	outB(PORT_COMMAND, 0xE7);
}

void AtaPioDevice::prepareRW(size_t lba, size_t sectors)
{
	// Select device
	u8 select = ((getDrive()==0) ? 0xE0 : 0xF0) | ((lba >> 24) & 0xF);
	outB(PORT_DRIVE, select);

	// Set number of sectors to read/write
	outB(PORT_SECTOR_COUNT, sectors);

	// Set starting address
	outB(PORT_LBA_LOW, (u8) lba);
	outB(PORT_LBA_MID, (u8) (lba >> 8));
	outB(PORT_LBA_HIGH, (u8) (lba >> 16));
}

void AtaPioDevice::waitBSYUnsetPoll()
{
	while((getStatus() & BIT_STATUS_BSY) != 0)
		asm volatile ("pause");
}

void AtaPioDevice::waitDRQSetPoll()
{
	while((getStatus() & (BIT_STATUS_ERR | BIT_STATUS_DRQ | BIT_STATUS_DF)) == 0)
		asm volatile ("pause");
}

void AtaPioDevice::waitDRQUnsetPoll()
{
	while((getStatus() & BIT_STATUS_DRQ) != 0) ;
}

bool AtaPioDevice::readPIO(size_t lba, size_t sectors, char* buf)
{
	waitBSYUnsetPoll();
	// Prepare the drive
	prepareRW(lba, sectors);

	// Send the read command and wait until it's ready to send data
	outB(PORT_COMMAND, COMMAND_READ);
	waitDRQSetPoll();

	u8 status = getStatus();
	if (status & BIT_STATUS_ERR)
	{
		printf("Error while reading\n");
		return false;
	}
	else if (status & BIT_STATUS_DF)
	{
		printf("Drive fault while reading\n");
		return false;
	}
	else
	{
		for (size_t i = 0; i < sectors*_bytesPerSector; i++)
		{
			u16 recv = inW(PORT_DATA);
			u8 low = (u8) recv;
			u8 high = (u8) (recv >> 8);

			buf[i] = low;
			i++;
			buf[i] = high;
			if (getStatus() & BIT_STATUS_ERR)
				return false;
		}

		waitDRQUnsetPoll();
		return true;
	}
}

bool AtaPioDevice::writePIO(size_t lba, size_t sectors, const char* buf)
{
	waitBSYUnsetPoll();
	prepareRW(lba, sectors);

	outB(PORT_COMMAND, COMMAND_WRITE);
	waitDRQSetPoll();

	if (getStatus() & BIT_STATUS_ERR)
	{
		printf("Error while reading\n");
		return false;
	}
	else
	{
		for (size_t i = 0; i < sectors*_bytesPerSector; i++)
		{
			u16 send = ((u16) buf[i]) | (((u16)buf[i+1]) << 8);
			i++;
			outW(PORT_DATA, send);
		}

		waitDRQUnsetPoll();
		flush();
		waitBSYUnsetPoll();

		return true;
	}
}

bool AtaPioDevice::read(size_t start, size_t bytes, char* data)
{
	size_t startLBA = toLBA(start);
	size_t endLBA = toLBA(start+bytes);
	size_t numBlocks = endLBA - startLBA + 1;

	char* buf = new char[numBlocks * _bytesPerSector];
	bool success = readPIO(startLBA, numBlocks, buf);
	
	if (success)
	{
		// At which byte inside the block does our data start
		size_t offset = start - (startLBA * _bytesPerSector);

		// Copy the data
		memcpy(data, buf+offset, bytes);
	}

	delete[] buf;
	return success;
}

bool AtaPioDevice::write(size_t start, size_t bytes, const char* data)
{
	size_t startLBA = toLBA(start);
	size_t endLBA = toLBA(start+bytes);
	size_t numBlocks = endLBA - startLBA + 1;

	char* buf = new char[numBlocks * _bytesPerSector];
	int success = read(startLBA, numBlocks, buf);
	
	if (success)
	{
		// At which byte inside the block does our data start
		size_t offset = start - (startLBA * _bytesPerSector);

		// Copy the data
		memcpy(buf+offset, data, bytes);

		// And write it back
		success = writePIO(startLBA, numBlocks, buf);
	}

	delete[] buf;
	return success;
}

size_t AtaPioDevice::toLBA(size_t address)
{
	return address / _bytesPerSector;
}

}