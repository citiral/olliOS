#ifndef __HPET_H_
#define __HPET_H_

#include "types.h"

namespace hpet {
    typedef void (*HpetCallback)(void* context);

    bool init();

    u64 get_elapsed_ns();

    void wait(u64 duration, HpetCallback cb);
    void interval(u64 interval, HpetCallback cb);
}

#endif