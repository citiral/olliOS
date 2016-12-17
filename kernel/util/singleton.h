#ifndef __SINGLETON_H
#define __SINGLETON_H

template<class T>
class Singleton
{
private:
    Singleton() {};
    ~Singleton() {};

public:
    static T& getInstance()
    {
        return instance;
    }

private:
    static T instance;
};

#endif /* end of include guard: __SINGLETON_H */
