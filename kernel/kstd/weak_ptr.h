#ifndef WEAK_PTR
#define WEAK_PTR

#include "shared_ptr.h"

namespace std {

template<class T>
class weak_ptr {
public:
    using value_type = T;
    using info_type = typename shared_ptr<T>::info_type;

    weak_ptr() {
        _data = NULL;
        _count = NULL;
    }

    weak_ptr(const weak_ptr<T>& s) {
        _data = s._data;
        _count = s._count;
        if (_data != nullptr) {
            __atomic_add_fetch(&_count->weakCount, 1, __ATOMIC_SEQ_CST);
        }
    }

    weak_ptr(weak_ptr<T>&& s) {
        _data = s._data;
        _count = s._count;
        s._data = nullptr;
        s._count = nullptr;
    }

    weak_ptr(const shared_ptr<T>& s) {
        _data = s._data;
        _count = s._count;
        if (_data != nullptr) {
            __atomic_add_fetch(&_count->weakCount, 1, __ATOMIC_SEQ_CST);
        }
    }

    ~weak_ptr() {
        reset();
    }

    void reset() {
        if (_data != nullptr) {
            _data = nullptr;
            if (__atomic_sub_fetch(&_count->weakCount, 1, __ATOMIC_SEQ_CST) == 0) {
                delete _count;
            }
            _count = nullptr;
        }
    }

    weak_ptr<T>& operator=(const weak_ptr<T>& s) {
        if (_data == s._data) {
            return *this;
        }

        reset();

        _data = s._data;
        _count = s._count;
        if (_data != nullptr) {
            __atomic_add_fetch(&_count->weakCount, 1, __ATOMIC_SEQ_CST);
        }

        return *this;
    }

    weak_ptr<T>& operator=(const shared_ptr<T>& s) {
        if (_data == s._data) {
            return *this;
        }

        reset();

        _data = s._data;
        _count = s._count;
        if (_data != nullptr) {
            __atomic_add_fetch(&_count->weakCount, 1, __ATOMIC_SEQ_CST);
        }

        return *this;
    }

    weak_ptr<T>& operator=(weak_ptr&& s) {
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

    shared_ptr<T> lock() const { 
        if (use_count() == 0) {
            return shared_ptr<T>();
        } else {
            shared_ptr<T> res;
            res._data = _data;
            res._count = _count;
            __atomic_add_fetch(&_count->weakCount, 1, __ATOMIC_SEQ_CST);
            __atomic_add_fetch(&_count->strongCount, 1, __ATOMIC_SEQ_CST);
            return res;
        }
    }

    long use_count() {
        long usecount = 0;
        if (_count != nullptr) {
            __atomic_load(&_count->strongCount, &usecount, __ATOMIC_SEQ_CST);
        }
        return usecount;
    }

private:
    T* _data;
    info_type* _count;
};
}

#endif