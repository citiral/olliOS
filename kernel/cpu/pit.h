#ifndef __PIT_H_
#define __PIT_H_

#include "types.h"

namespace pit {

    enum class Channel {
        channel_0 = 0,
        channel_1 = 1,
        channel_2 = 2,
    };

    void init();

    void set_one_shot(Channel channel, u16 count);
}

#endif