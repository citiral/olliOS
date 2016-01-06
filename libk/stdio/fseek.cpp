#include "stdio.h"
#include "stream.h"

int fseek(FILE* stream, long int offset, int origin)
{
    stream->seek(offset, origin);
    return 0;
}
