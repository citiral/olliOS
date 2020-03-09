#ifndef KSTD_STRING_H
#define KSTD_STRING_H

#include <types.h>
#include "functional.h"

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

        friend bool operator==(const string& lhs, const string& rhs);
        friend bool operator==(const string& lhs, const char* rhs);
        friend bool operator==(const char* lhs, const string& rhs);
        friend bool operator!=(const string& lhs, const string& rhs);
        friend bool operator!=(const string& lhs, const char* rhs);
        friend bool operator!=(const char* lhs, const string& rhs);

        char& operator[](size_t pos);
		const char& operator[](size_t pos) const;
		
		friend string operator+(const string& lstr, const char* rstr);
		friend string operator+(const char* lstr, const string& rstr);
		friend string operator+(const string& lstr, const string& rstr);
		friend string operator+(const string& lstr, char rchar);
		friend string operator+(char lchar, const string& rstr);

		string& operator+=(const char* str);
		string& operator+=(const string& str);
		string& operator+=(char c);

        int compare(const string& str) const;
        int compare(const char* str) const;

        size_t length() const;
        size_t size() const;

        const char* c_str() const;
        const char* data() const;

        string substr(size_t pos = 0, size_t len = npos) const;

        static const size_t npos = -1;
    private:
        char* _data;
	};
	
	template<> struct hash<string>
	{
		// Using Jenkin's one-at-the-time hash
		size_t operator()(const string &str) const
		{;
			size_t hash = 0;

			for (size_t i = 0; i < str.length(); i++)
			{
				hash += str[i++];
				hash += hash << 10;
				hash ^= hash >> 6;
			}

			hash += hash << 3;
			hash ^= hash >> 11;
			hash += hash << 15;
			return hash;
		}
	};
}

#endif