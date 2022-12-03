#ifndef SHARED_PTR
#define SHARED_PTR

#include "cdefs.h"
#include <stdint.h>

namespace std {

template<class T>
class weak_ptr;

template<class T>
class shared_ptr {
public:
    using value_type = T;
    using info_type = struct InfoBlock {
        long weakCount = 1;
        long strongCount = 1;
    };    

    shared_ptr() {
        _data = NULL;
        _count = NULL;
    }

    shared_ptr(T* data) {
        _data = data;
        _count = nullptr;
        if (data != nullptr) {
            _count = new InfoBlock;
        }
    }

    shared_ptr(const shared_ptr& s) {
        _data = s._data;
        _count = s._count;
        if (_count != nullptr) {
            __atomic_add_fetch(&_count->strongCount, 1, __ATOMIC_SEQ_CST);
            __atomic_add_fetch(&_count->weakCount, 1, __ATOMIC_SEQ_CST);
        }
    }

    shared_ptr(shared_ptr&& s) {
        _data = s._data;
        _count = s._count;
        s._data = nullptr;
        s._count = nullptr;
    }

    ~shared_ptr() {
        reset();
    }

    void reset() {
        if (_data != nullptr) {
            if (__atomic_sub_fetch(&_count->strongCount, 1, __ATOMIC_SEQ_CST) == 0) {
                delete _data;
            }
            _data = nullptr;
            if (__atomic_sub_fetch(&_count->weakCount, 1, __ATOMIC_SEQ_CST) == 0) {
                delete _count;
            }
            _count = nullptr;
        }
    }

    void reset(T* data) {
        reset();
        _data = data;
        _count = nullptr;
        if (data != nullptr) {
            _count = new InfoBlock;
        }
    }

    shared_ptr<T>& operator=(const shared_ptr& s) {
        if (_data == s._data) {
            return *this;
        }

        reset();

        _data = s._data;
        _count = s._count;
        if (_data != nullptr) {
            __atomic_add_fetch(&_count->strongCount, 1, __ATOMIC_SEQ_CST);
            __atomic_add_fetch(&_count->weakCount, 1, __ATOMIC_SEQ_CST);
        }

        return *this;
    }

    shared_ptr<T>& operator=(shared_ptr&& s) {
        if (_data == s._data) {
            s._data = NULL;
            s._count = NULL;
            return *this;
        }

        reset();

        _data = s._data;
        _count = s._count;
        s._data = nullptr;
        s._count = nullptr;

        return *this;
    }

    operator bool() const {
        return _data != nullptr;
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
        long usecount = 0;
        if (_count != nullptr) {
            __atomic_load(_count->strongCount, &usecount, __ATOMIC_SEQ_CST);
        }
        return usecount;
    }

private:
    T* _data;
    info_type* _count;

    friend weak_ptr<T>;
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