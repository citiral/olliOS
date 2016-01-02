#pragma once

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
