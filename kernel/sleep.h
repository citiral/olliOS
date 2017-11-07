#ifndef __OLLIOS_SLEEP_H
#define __OLLIOS_SLEEP_H

#include "stdint.h"

// Tries to sleep for at least the given amount of milliseconds. It will not be exactly precise, and usually overshoot because of overhead, unless the timer is calibrated wrong.
void sleep(uint32_t millisecond);

#endif