#include "cmos.h"
#include "io.h"

using namespace cmos;

#define CMOS_REGISTER_SELECT_PORT    0x70
#define CMOS_DATA_PORT               0x71

#define REGISTER_SECONDS    0x00
#define REGISTER_MINUTES    0x02
#define REGISTER_HOURS      0x04
#define REGISTER_DAY        0x07
#define REGISTER_MONTH      0x08
#define REGISTER_YEAR       0x09
#define REGISTER_STATUS_A   0x0A
#define REGISTER_STATUS_B   0x0B

#define STATUS_B_24H_MODE       0x2
#define STATUS_B_BINARY_MODE    0x4


uint8_t readRegister(uint8_t reg) {
    outb(CMOS_REGISTER_SELECT_PORT, reg);
    return inb(CMOS_DATA_PORT);
}


uint8_t readValue(uint8_t reg, bool isBinary, bool is24Hour)
{
    uint8_t v = readRegister(reg);
    bool isPm = false;

    if (is24Hour) {
        isPm = (v & 0x80) != 0;
        v &= 0x7F;
    }

    if (!isBinary) {
        v = ((v / 16) * 10) + (v & 0xF);
    }

    if (is24Hour) {
        if (v == 12 && isPm) {
            v = 0;
        }
    }

    return v;
}


cmos_time cmos::getCurrentTime() {
    uint8_t status = readRegister(REGISTER_STATUS_B);
    bool is24Hour = (status & STATUS_B_24H_MODE) != 0;
    bool isBinary = (status & STATUS_B_BINARY_MODE) != 0;
    
    cmos_time t;
    t.seconds = readValue(REGISTER_SECONDS, isBinary, false);
    t.minutes = readValue(REGISTER_MINUTES, isBinary, false);
    t.hours = readValue(REGISTER_HOURS, isBinary, is24Hour);
    t.day = readValue(REGISTER_DAY, isBinary, false);
    t.month = readValue(REGISTER_MONTH, isBinary, false);
    t.year = readValue(REGISTER_YEAR, isBinary, false);
    return t;
}