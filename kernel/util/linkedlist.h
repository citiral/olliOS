//
// Created by Olivier on 09/10/16.
//

#ifndef OLLIOS_GIT_LINKEDLIST_H
#define OLLIOS_GIT_LINKEDLIST_H

template<class T>
class LinkedList<T> {
public:
    LinkedList(T elem, LinkedList<T>* next) {
        _data = elem;
        _next = next;
    }

    LinkedList(const T& elem, LinkedList<T>* next) {
        _data = elem;
        _next = next;
    }

    LinkedList(T&& elem, LinkedList<T>* next) {
        _data = std::move(elem);
        _next = next;
    }

    ~LinkedList() {
        if (_next)
            delete _next;
    }

    T& get() {
        return _data;
    }

    const T& get() const {
        return _data;
    }

    void push(T elem) {
        _next = LinkedList(_data, _next);
    }

    void push(const T& elem) {
        _next = LinkedList(_data, _next);
    }

    void push(T&& elem) {
        _next = LinkedList(std::move(_data), _next);
    }
private:
    T _data;
    LinkedList<T> _next;
};

#endif //OLLIOS_GIT_LINKEDLIST_H
