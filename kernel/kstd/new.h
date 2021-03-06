//
// Created by Olivier on 25/09/16.
//

#ifndef OLLIOS_GIT_NEW_H
#define OLLIOS_GIT_NEW_H

#include "kstd/cstddef.h"

void* operator new(std::size_t size);
//void* operator new (std::size_t size, const std::nothrow_t& nothrow_value) noexcept;
void* operator new(std::size_t size, void* ptr) noexcept;

void* operator new[](std::size_t size);
//void* operator new[] (std::size_t size, const std::nothrow_t& nothrow_value) noexcept;
void* operator new[](std::size_t size, void* ptr) noexcept;

void operator delete(void* ptr) noexcept;
//void operator delete (void* ptr, const std::nothrow_t& nothrow_constant) noexcept;
void operator delete(void* ptr, void* voidptr2) noexcept;

void operator delete[](void* ptr) noexcept;
//void operator delete[] (void* ptr, const std::nothrow_t& nothrow_constant) noexcept;
void operator delete[](void* ptr, void* voidptr2) noexcept;

#endif //OLLIOS_GIT_NEW_H
