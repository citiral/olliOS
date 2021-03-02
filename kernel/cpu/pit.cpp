#include "pit.h"
#include "io.h"

#define CHANNEL_0_DATA_PORT 0x40
#define CHANNEL_1_DATA_PORT 0x41
#define CHANNEL_2_DATA_PORT 0x42
#define COMMAND_REGISTER    0x43

using namespace pit;

void pit::init()
{
    // configure all 3 channels in one shot mode, because the bios might have configured them differently
    set_one_shot(Channel::channel_0, 0);
    set_one_shot(Channel::channel_1, 0);
    set_one_shot(Channel::channel_2, 0);
}

void pit::set_one_shot(Channel channel, u16 count)
{
    // convert the channel number to an index
    u8 channelVal = (u8) channel;

    // command the channel to be oneshot
    outb(COMMAND_REGISTER, 0b00110000 | (channelVal << 6));

    // write the high and low counter values to the timer channel
    outb(CHANNEL_0_DATA_PORT + channelVal, count & 0xFF);
    outb(CHANNEL_0_DATA_PORT + channelVal, (count >> 8) & 0xFF);

}