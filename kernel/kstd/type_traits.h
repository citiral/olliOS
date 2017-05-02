//
// Created by Olivier on 27/09/16.
//

#ifndef OLLIOS_GIT_TYPE_TRAITS_H
#define OLLIOS_GIT_TYPE_TRAITS_H

namespace std {
    template<class T>
    struct remove_reference {
        typedef T type;
    };
    template<class T>
    struct remove_reference<T&> {
        typedef T type;
    };
    template<class T>
    struct remove_reference<T&&> {
        typedef T type;
    };
}

#endif //OLLIOS_GIT_TYPE_TRAITS_H
