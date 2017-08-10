#include "kstd/string.h"
#include "kstd/algorithm.h"
#include <string.h>

// Empty strings are allocated as real "\0" strings... I could implement it with nullptr being used as the empty string. but that'd be more work and more error prone
namespace std {

string::string() {
    // we allocate an empty string.
    _data = new char[1];
    _data[0] = 0;
}

string::string(const string& str) {
    _data = new char[str.length() + 1];
    strcpy(_data, str._data);
}

string::string(const char* s) {
    _data = new char[strlen(s) + 1];
    strcpy(_data, s);
}

string::string(const char* s, size_t n) {
    _data = new char[n + 1];
    memcpy(_data, s, n);
    _data[n] = 0;
}

string::string(size_t n, char c) {
    _data = new char[n+1];
    for (size_t i = 0 ; i < n ; i++) {
        _data[i] = c;
    }
    _data[n] = 0;
}

string::string(string&& str) {
    _data = str._data;
    str._data = nullptr;
}

string::~string() {
    if (_data != nullptr) {
        delete[] _data;
    } 
}
string& string::operator=(const string& str) {
    if (_data != nullptr) {
        delete[] _data;
    }

    _data = new char[str.size() + 1];
    strcpy(_data, str._data);

    return *this;
}

string& string::operator=(const char* s) {
    if (_data != nullptr) {
        delete[] _data;
    }

    _data = new char[strlen(s) + 1];
    strcpy(_data, s);

    return *this;
}

string& string::operator=(char c) {
    if (_data != nullptr) {
        delete[] _data;
    }

    _data = new char[2];
    _data[0] = c;
    _data[1] = 0;

    return *this;
}

string& string::operator=(string&& str) {
    char* temp = _data;
    _data = str._data;
    str._data = temp;
    return *this;
}

char& string::operator[](size_t pos) {
    return _data[pos];
}

const char& string::operator[](size_t pos) const {
    return _data[pos];
}

bool operator==(const string& lhs, const string& rhs) {
    return lhs.compare(rhs) == 0;
}

bool operator==(const string& lhs, const char* rhs) {
    return lhs.compare(rhs) == 0;
}

bool operator==(const char* lhs, const string& rhs) {
    return rhs.compare(lhs) == 0;
}

bool operator!=(const string& lhs, const string& rhs) {
    return lhs.compare(rhs) != 0;
}

bool operator!=(const string& lhs, const char* rhs) {
    return lhs.compare(rhs) != 0;
}

bool operator!=(const char* lhs, const string& rhs) {
    return rhs.compare(lhs) != 0;
}

int string::compare(const string& str) const {
    return compare(str.c_str());
}

int string::compare(const char* str) const {
    size_t llength = length();
    size_t rlength = strlen(str);

    int result = memcmp(c_str(), str, min(llength, rlength));

    if (result != 0)
        return result;
    if (llength < rlength)
        return -1;
    if (llength > rlength)
        return 1;
    return 0;
}

size_t string::length() const {
    return strlen(_data);
}

size_t string::size() const {
    return strlen(_data);
}

const char* string::c_str() const {
    return _data;
}

const char* string::data() const {
    return _data;
}

}