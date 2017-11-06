//
// Created by Olivier on 25/09/16.
//

#include "cdefs.h"
#include "kstd/new.h"
#include <stdlib.h>

void* operator new (std::size_t size) {
    return malloc(size);
}

/*void* operator new (std::size_t size, const std::nothrow_t& nothrow_value) noexcept {
    return ::operator new(size);
}*/

void* operator new (std::size_t size, void* ptr) noexcept {
	UNUSED(size);
    return ptr;
}

void* operator new[](std::size_t size) {
    return ::operator new(size);
}

/*void* operator new[] (std::size_t size, const std::nothrow_t& nothrow_value) noexcept {
    return ::operator new(size);
}*/

void* operator new[] (std::size_t size, void* ptr) noexcept {
	UNUSED(size);
    return ptr;
}

void operator delete (void* ptr) noexcept {
    free(ptr);
}

/*void operator delete (void* ptr, const std::nothrow_t& nothrow_constant) noexcept {
    ::operator delete(ptr);
}*/

void operator delete (void* ptr, void* voidptr2) noexcept {
	// should do nothing..
	UNUSED(ptr);
	UNUSED(voidptr2);
}

void operator delete[] (void* ptr) noexcept {
    ::operator delete(ptr);
}

/*void operator delete[] (void* ptr, const std::nothrow_t& nothrow_constant) noexcept {
    ::operator delete (ptr);
}*/

void operator delete[] (void* ptr, void* voidptr2) noexcept {
	// also does nothing
	UNUSED(ptr);
	UNUSED(voidptr2);
}