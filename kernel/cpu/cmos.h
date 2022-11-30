#ifndef __OLLIOS_CMOS_H_
#define __OLLIOS_CMOS_H_

#include <stdint.h>

namespace cmos {
    struct cmos_time {
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
        uint8_t day;
        uint8_t month;
        uint8_t year;
    };

    cmos_time getCurrentTime();
}

#endif