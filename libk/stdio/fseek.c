#include "stdio.h"
#include "streams/blockdevice.h"

int fseek(FILE* stream, long int offset, int origin)
{
    return stream->seek(offset, origin);
}
