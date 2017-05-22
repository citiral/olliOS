#pragma once

#include <types.h>

// divergence from standard: missing functions

namespace std {
    class string {
    public:
        string();
        string(const string& str);
        string(const char* s);
        string(size_t n, char c);
        string(string&& str);
        ~string();

        string& operator=(const string& str);
        string& operator=(const char* s);
        string& operator=(char c);
        string& operator=(string&& str);

        size_t length() const;
        size_t size() const;

        const char* c_str() const;
        const char* data() const;

    private:
        char* _data;
    };
}