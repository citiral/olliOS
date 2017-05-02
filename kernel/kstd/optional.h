//
// Created by Olivier on 01/10/16.
//

#ifndef OLLIOS_GIT_OPTIONAL_H
#define OLLIOS_GIT_OPTIONAL_H

namespace std {
    /*template<class T>
    class optional {
    public:
        using value_type = T;

        optional(): _some(false) {};

        optional(const optional& other) {
            if (other._some) {
                _some = true;
                new (_data) T(*other);
            } else {
                _some = false;
            }
        }

        optional(optional&& other) {
            if (other._some) {
                _some = true;
                new (_data) T(std::move(*other));
            } else {
                _some = false;
            }
        }

        constexpr optional(const T& value) {
            _some = true;
            new (_data) T(value);
        }

        constexpr optional(T&& value) {
            _some = true;
            new (_data) T(std::move(value));
        }

        ~optional() {
            if (_some) {
                ((T*)_data).~T();
            }
        }

        optional& operator=( std::nullopt_t ) {
            if (_some) {
                ((T*)_data).~T();
                _some = false;
            }
        }

        optional& operator=(const optional& other) {

        }

        optional& operator=( optional&& other );

        template< class U >
        optional& operator=( U&& value );


    private:
        // container for the data of T
        unsigned char _data[sizeof(T)];

        // whether or not this contains a value
        bool _some;
    };*/
}

#endif //OLLIOS_GIT_OPTIONAL_H
