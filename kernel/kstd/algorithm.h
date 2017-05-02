//
// Created by Olivier on 27/09/16.
//

#ifndef OLLIOS_GIT_ALGORITHM_H
#define OLLIOS_GIT_ALGORITHM_H

namespace std {
    template<typename T>
    void swap(T& t1, T& t2) {
        T temp = move(t1); // or T temp(std::move(t1));
        t1 = move(t2);
        t2 = move(temp);
    }
}

#endif //OLLIOS_GIT_ALGORITHM_H
