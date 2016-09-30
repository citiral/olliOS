#include "stdio.h"
#include "stream.h"

int fseek(FILE* stream, long int offset, int origin)
{
    return stream->seek(offset, origin);
}
