#include "rtti.h"

type_t generateType()
{
    static type_t count = 0;
    count++;
    return count;
}
