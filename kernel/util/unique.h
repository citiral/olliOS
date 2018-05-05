#ifndef __UNIQUE_H_
#define __UNIQUE_H_

#include <types.h>

// a class that enables generating a number in an atomic faction, so that the number is guaranteerd to be unique.
template<class T>
class UniqueGenerator {
public:
    UniqueGenerator(T initial = 0): _value(initial)
    {

    }

    T next()
    {
        volatile register T out = 1;
        __asm__("lock; xadd %1, %0" : "+m" (_value), "+r" (out));
        return out;
    }

private:
    T _value;
};

#endif