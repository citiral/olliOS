#include "stdio.h"
#include "streams/stream.h"

int fseek(FILE* stream, long int offset, int origin)
{
    return stream->seek(offset, origin);
}
