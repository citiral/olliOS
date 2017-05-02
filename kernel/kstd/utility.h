//
// Created by Olivier on 27/09/16.
//

#ifndef OLLIOS_GIT_UTILITY_H
#define OLLIOS_GIT_UTILITY_H

#include "kstd/type_traits.h"
#include "kstd/algorithm.h"

namespace std {
    template<class T>
    typename remove_reference<T>::type&& move(T&& a) noexcept
    {
        typedef typename remove_reference<T>::type&& RvalRef;
        return static_cast<RvalRef>(a);
    }

    template<class T1, class T2>
    struct pair {
    public:
        using first_type = T1;
        using second_type = T2;

        constexpr pair(): first({}), second({}) {};
        constexpr pair(const T1& x, const T2& y): first(x), second(y) {};
        template< class U1, class U2 >
        constexpr pair(const U1&& x, const U2&& y): first(x), second(y) {};
        template< class U1, class U2 >
        constexpr pair(const pair<U1, U2>& p): first(p.first), second(p.second) {};
        template< class U1, class U2 >
        constexpr pair(const pair<U1, U2>&& p): first(p.first), second(p.second) {};
        pair( const pair& p ) = default;
        pair( pair&& p ) = default;

        pair& operator=(const pair& other) {
            first = other.first;
            second = other.second;
        }

        template< class U1, class U2 >
        pair& operator=(const pair<U1,U2>& other) {
            first = other.first;
            second = other.second;
        }

        pair& operator=(pair&& other) {
            swap(first, other.first);
            swap(second, other.second);
        }

        template< class U1, class U2 >
        pair& operator=(pair<U1,U2>&& other) {
            swap(first, other.first);
            swap(second, other.second);
        }

        T1 first;
        T2 second;
    };
}

#endif //OLLIOS_GIT_UTILITY_H
