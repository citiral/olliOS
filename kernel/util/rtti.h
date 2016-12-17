#ifndef __RTTI_H
#define __RTTI_H

#include "stddef.h"

typedef size_t type_t;

type_t generateType();

template <typename T>
type_t typeOf()
{
    static const type_t type = generateType();
    return type;
}

template<class T>
class Rtti {
public:
    static type_t getType()
    {
        return typeOf<T>();
    }
};

#endif /* end of include guard: __RTTI_H */
