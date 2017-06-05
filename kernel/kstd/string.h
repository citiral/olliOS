#ifndef KSTD_STRING_H
#define KSTD_STRING_H
#include <types.h>

// divergence from standard: missing functions

namespace std {
    class string {
    public:
        string();
        string(const string& str);
        string(const char* s);
        string(const char* s, size_t n);
        string(size_t n, char c);
        string(string&& str);
        ~string();

        string& operator=(const string& str);
        string& operator=(const char* s);
        string& operator=(char c);
        string& operator=(string&& str);
        
        char& operator[](size_t pos);
        const char& operator[](size_t pos) const;

        size_t length() const;
        size_t size() const;

        const char* c_str() const;
        const char* data() const;

        static const size_t npos = -1;
    private:
        char* _data;
    };
}

#endif