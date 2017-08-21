//
// Created by Olivier on 30/09/16.
//

#ifndef OLLIOS_GIT_ATAPIODEVICE_H
#define OLLIOS_GIT_ATAPIODEVICE_H

#include "devices/ata/atadevice.h"
#include "cdefs.h"

class AtaPioDevice: public AtaDevice {
public:
    AtaPioDevice(u16 port, unsigned short* data, int device);
    ~AtaPioDevice();

    virtual DeviceType getDeviceType() const;
	virtual void getDeviceInfo(void* deviceinfo) const;

    size_t write(const void* data, size_t amount);
    size_t write(const void* data);
    size_t write(char data);
    size_t read(void* data, size_t amount);
    size_t seek(i32 offset, int position);

private:
	u8 getStatus();
	void flush();
	void prepareRW(size_t lba, size_t sectors);
	void waitBSYUnsetPoll();
	void waitDRQSetPoll();
	void waitDRQUnsetPoll();
	// Return value: true on success, false on fail
	bool readPIO(size_t lba, size_t sectors, char* buf);
	// Return valua: true on success, false on fail
	bool writePIO(size_t lba, size_t sectors, const char* buf);
	// Read a certain number of bytes. Is completely block/sector independent
	bool read(size_t start, size_t bytes, char* data);
	// Read a certain number of bytes. Is completely block/sector independent
	bool write(size_t start, size_t bytes, const char* data);

	// Turn a byte address to a LBA address
	size_t toLBA(size_t address);

	size_t _pointer = 0;
};

#endif //OLLIOS_GIT_ATAPIODEVICE_H
