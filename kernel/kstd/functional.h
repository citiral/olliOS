#ifndef OLLIOS_GIT_FUNCTIONAL_H
#define OLLIOS_GIT_FUNCTIONAL_H

#include "types.h"
#include "cdefs.h"

namespace std
{
// Not sure what this does exactly
template <typename _Result, typename _Arg>
struct __hash_base
{
	typedef _Result result_type;
	typedef _Arg argument_type;
};

// Define primary hash class template
template <typename _Tp>
struct hash : public __hash_base<size_t, _Tp>
{
	size_t operator()(_Tp __val) const;
};

// Something for pointers
template <typename _Tp>
struct hash<_Tp *> : public __hash_base<size_t, _Tp *>
{
	size_t operator()(_Tp *__val) const
	{
		return reinterpret_cast<size_t>(__val);
	}
};

// Explicit specializations for integer types.
#define _Cxx_hashtable_define_trivial_hash(_Tp) \
	template <>                                 \
	inline size_t                               \
	hash<_Tp>::operator()(_Tp __val) const      \
	{                                           \
		return static_cast<size_t>(__val);      \
	}

/// Explicit specialization for bool.
_Cxx_hashtable_define_trivial_hash(bool);
_Cxx_hashtable_define_trivial_hash(char);
_Cxx_hashtable_define_trivial_hash(int);
_Cxx_hashtable_define_trivial_hash(short);
_Cxx_hashtable_define_trivial_hash(long);
_Cxx_hashtable_define_trivial_hash(unsigned char);
_Cxx_hashtable_define_trivial_hash(unsigned int);
_Cxx_hashtable_define_trivial_hash(unsigned short);
_Cxx_hashtable_define_trivial_hash(unsigned long);
}

#endif