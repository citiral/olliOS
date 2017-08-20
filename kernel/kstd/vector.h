//
// Created by Olivier on 25/09/16.
//

#ifndef OLLIOS_GIT_VECTOR_H
#define OLLIOS_GIT_VECTOR_H

#include "kstd/cstddef.h"
#include "kstd/utility.h"
#include "kstd/new.h"
#include <stdlib.h>

// divergence of spec from vector: No iterators, no allocator, missing functions

namespace std {
    template<class T>
    class vector {
    public:
        // typedefs
        using value_type = T;
        using size_type = std::size_t;

        vector() {
            _length = 0;
            _maxLength = 0;
            _data = nullptr;
        }

        vector(const vector<T>& ref) {
            _length = ref._length;
            _maxLength = ref._maxLength;

            if (_maxLength > 0) {
                _data = static_cast<T*>(malloc(_maxLength * sizeof(T)));
            
                if (ref._data != nullptr) {
                    for (size_t i = 0; i < _length; i++) {
                        new (&_data[i]) T(ref._data[i]);
                    }
                }
            } else {
                _data = nullptr;
            }
        }

        vector(vector<T>&& ref) {
            _length = ref._length;
            _maxLength = ref._maxLength;
            _data = ref._data;

            ref._data = nullptr;
            ref._length = 0;
            ref._maxLength = 0;
        }

        ~vector() {
            // clean up all remaining elements
            if (_data != nullptr) {
                for (size_t i = 0; i < _length; i++) {
                    _data[i].~T();
                }

                free(_data);
                _data = nullptr;
            }
        }

        vector<T>& operator=(const vector<T>& ref) {
            // clean up all remaining elements
            if (_data != nullptr) {
                for (size_t i = 0; i < _length; i++) {
                    _data[i].~T();
                }

                free(_data);
                _data = nullptr;
            }

            _length = ref._length;
            _maxLength = ref._maxLength;
            if (_maxLength > 0) {
                _data = static_cast<T*>(malloc(_maxLength * sizeof(T)));

                if (ref._data != nullptr) {
                    for (size_t i = 0; i < _length; i++) {
                        new (&_data[i]) T(ref._data[i]);
                    }
                }
            } else {
                _data = nullptr;
            }

            return *this;
        }

        vector<T>& operator=(vector<T>&& ref) {
            // clean up all remaining elements
            if (_data != nullptr) {
                for (size_t i = 0; i < _length; i++) {
                    _data[i].~T();
                }

                free(_data);
            }

            _length = ref._length;
            _maxLength = ref._maxLength;
            _data = ref._data;

            ref._data = nullptr;
            ref._length = 0;
            ref._maxLength = 0;

            return *this;
        }

        size_type size() const noexcept {
            return _length;
        }

        size_type max_size() const noexcept {
            return _maxLength;
        }

        size_t find(const T& value) const {
            for (size_t i = 0 ; i < _length ; i++) {
                if (_data[i] == value)
                    return i;
            }
            return (size_t) -1;
        }

        void erase(size_t position) {
            // bubble all elements folowing the position one down
            for (size_t i = position ; i + 1 < _length ; i++) {
                _data[i] = std::move(_data[i+1]);
            }

            // and run the destructor of the last element, then shrink the array
            if (_length > 0) {
                _data[_length - 1].~T();
                _length--;
            }
		}
		
		void clear() {
			if (_data != nullptr) {
                for (size_t i = 0; i < _length; i++) {
                    _data[i].~T();
                }

                free(_data);
			}
			
			_length = 0;
			_maxLength = 0;
			_data = nullptr;
		}

        T& at(size_type n) {
            return _data[n];
        }

        const T& at(size_type n) const {
            return _data[n];
        }

        T& operator[] (size_type n) {
            return _data[n];
        }

        const T& operator[] (size_type n) const {
            return _data[n];
		}
		
		// DEVIATES FROM SPECIFICATION
		// position is a size_t instead of an iterator
		void insert(size_t position, const value_type& val)
		{
			if (size() > 0)
			{
				push_back(back());
				for (size_t i = size()-2; i > position; i--)
					_data[i] = _data[i-1];
				_data[position] = val;
			}
			else if (position == 0)
			{
				push_back(val);
			}
			else
			{
				// Um, we shouldn't be able to reach this unless position was invalid
			}
		}

        void push_back(const value_type& val) {
            // optionally expand the array
            testExpand();
            // use placement new to construct the element at the end of the array
            new (&_data[_length]) T(val);
            _length++;
        }

        void push_back(value_type&& val) {
            // optionally expand the array
            testExpand();
            // use placement new to construct the element at the end of the array
            new (&_data[_length]) T(val);
            _length++;
        }

        void pop_back() {
            // first call the last elements constructor and then remove it
            if (_length > 0) {
                _data[_length-1].~T();
                _length--;
            }
        }

        T& front() {
            return _data[0];
        }

        T&& front() const {
            return _data[0];
        }

        T& back() {
            return _data[_length-1];
        }

        T&& back() const {
            return _data[_length-1];
        }

        T* data() {
            return _data;
        }

        const T* data() const {
            return _data;
        }

    private:
        void testExpand() {
            if (_length == _maxLength) {
                // get the new size and (re)allocate the array
                _maxLength = _maxLength*2 + 1;

                if (_data == nullptr) {
                    _data = static_cast<T*>(malloc(_maxLength * sizeof(T)));
                } else {
                    _data = static_cast<T*>(realloc(_data, _maxLength * sizeof(T)));
                }
            }
        }

        T* _data;
        size_t _length;
        size_t _maxLength;
    };

    template<class T>
    bool operator==( const vector<T>& lhs,
                     const vector<T>& rhs ) {
        if (lhs.size() != rhs.size())
           return false;

        for (size_t i = 0; i < lhs.size(); i++)
            if (!(lhs[i] == rhs[i]))
                return false;

        return true;
    };
}

#endif //OLLIOS_GIT_VECTOR_H
