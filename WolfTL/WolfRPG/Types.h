#pragma once

#include <string>
#include <vector>
#include <windows.h>

// Check MSVC
#if _WIN32 || _WIN64
#if _WIN64
#define BIT_64
#else
#define BIT_32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define BIT_64
#else
#define BIT_32
#endif
#endif

using Bytes    = std::vector<uint8_t>;
using uInts    = std::vector<uint32_t>;
using tString  = std::wstring;
using tStrings = std::vector<tString>;

#define DISABLE_COPY_MOVE(T)             \
	T(T const &)               = delete; \
	void operator=(T const &t) = delete; \
	T(T &&)                    = delete;
