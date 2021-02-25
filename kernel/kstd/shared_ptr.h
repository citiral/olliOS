#ifndef SHARED_PTR
#define SHARED_PTR

#include "cdefs.h"
#include <stdint.h>

namespace std {

template<class T>
class shared_ptr {
public:
    using value_type = T;

    shared_ptr() {
        _data = NULL;
        _count = NULL;
    }

    shared_ptr(T* data) {
        _data = data;
        _count = nullptr;
        if (data != nullptr) {
            _count = new long;
            *_count = 1;
        }
    }

    shared_ptr(const shared_ptr& s) {
        _data = s._data;
        _count = s._count;
        if (_count != nullptr) {
            __atomic_add_fetch(_count, 1, __ATOMIC_RELAXED);
        }
    }

    shared_ptr(shared_ptr&& s) {
        _data = s._data;
        _count = s._count;
        s._data = nullptr;
        s._count = nullptr;
    }

    ~shared_ptr() {
        if (_data != nullptr) {
            if (__atomic_sub_fetch(_count, 1, __ATOMIC_RELAXED) == 0) {
                delete _data;
                delete _count;
                _data = nullptr;
                _count = nullptr;
            }
        }
    }

    shared_ptr<T>& operator=(const shared_ptr& s) {
        if (_data == s._data) {
            return *this;
        }

        if (_data != nullptr) {
            if (__atomic_sub_fetch(_count, 1, __ATOMIC_ACQUIRE) == 0) {
                delete _data;
                delete _count;
                _data = nullptr;
                _count = nullptr;
            }
        }

        _data = s._data;
        _count = s._count;
        if (_data != nullptr) {
            __atomic_add_fetch(_count, 1, __ATOMIC_ACQUIRE);
        }

        return *this;
    }

    shared_ptr<T>& operator=(shared_ptr&& s) {
        if (_data == s._data) {
            s._data = NULL;
            s._count = NULL;
            return *this;
        }
        
        if (_data != nullptr) {
            if (__atomic_sub_fetch(_count, 1, __ATOMIC_ACQUIRE) == 0) {
                delete _data;
                delete _count;
                _data = nullptr;
                _count = nullptr;
            }
        }

        _data = s._data;
        _count = s._count;
        s._data = nullptr;
        s._count = nullptr;

        return *this;
    }

    T& operator*() {
        return *_data;
    }

    T* operator->() {
        return _data;
    }

    T& operator[](int v) {
        return _data[v];
    }

    T* get() const { 
        return _data;
    }

    long use_count() {
        return _count ? *_count : 0;
    }

private:
    T* _data;
    long* _count;
};

template < class T, class U >
bool operator==( const std::shared_ptr<T>& lhs,
                const std::shared_ptr<U>& rhs ) {
    return lhs.get() == rhs.get();
}

template < class T, class U >
bool operator!=( const std::shared_ptr<T>& lhs,
                const std::shared_ptr<U>& rhs ) {
    return lhs.get() != rhs.get();
}

template< class T >
bool operator==( const std::shared_ptr<T>& lhs, nullptr_t ) {
    return lhs.get() == nullptr;
}

template< class T >
bool operator!=( const std::shared_ptr<T>& lhs, nullptr_t ) {
    return lhs.get() != nullptr;
}

}

#endif