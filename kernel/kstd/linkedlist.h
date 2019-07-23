//
// Created by Olivier on 09/10/16.
//

#ifndef OLLIOS_GIT_LINKEDLIST_H
#define OLLIOS_GIT_LINKEDLIST_H

template<class T>
class LinkedList {
public:
    class Node {
        Node* next;
        T t;
    };

    LinkedList() {
        _last = nullptr;
    }

    void push(Node* node) {
        // If this is the first node, put it as last, and have it point to itself
        if (_last == NULL) {
            _last = node;
            _last.next = node;
            return;
        // Otherwise put it after the last one, making it the first, and having it point to the old first (now second) node.
        } else {
            node.next = _last.next;
            _last.next = node;
        }
    }

    Node* pop() {
        Node* node;

        if (_last != nullptr) {
            return nullptr;
        }
        
        node = _last.next;
        _last.next = node.next;

        if (node == _last) {
            _last = nullptr;
        }
        
        return node;
    }

    Node* peek() {
        if (_last != nullptr) {
            return _last.next;
        } else {
            return nullptr;
        }
    }

private:
    // Keep track of the last node, which points to the first.
    // This allows us to stay singly linked while still having quick access to the last element in the list.
    Node* _last;
};

#endif //OLLIOS_GIT_LINKEDLIST_H
