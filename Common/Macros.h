#pragma once
#include "Core/Log/Logging.h"

#ifndef _DEBUG
#define NCL_DEBUG 0
#else
#define NCL_DEBUG 1
#endif // NDEBUG

#ifndef NCL_ASSERT
#if NCL_DEBUG
#define NCL_ASSERT(x) \
        do { \
            if(!(x)) { \
                NCL_FATAL("Assertion: {}\n\t{}:{}", XSTR(x), __FILE__, __LINE__); \
            } \
        } while(0)
#else
#define NCL_ASSERT(x) (void)(x)
#endif
#endif // NCL_ASSERT

#ifdef _MSC_VER
#define NCL_DEBUG_BREAK() \
	do { \
		if (IsDebuggerPresent()) { \
			__debugbreak(); \
		} else { \
			::std::abort(); \
		} \
	} while (0)
#elif __GNUC__
#define NCL_DEBUG_BREAK() __builtin_trap()
#elif __clang__
#define NCL_DEBUG_BREAK() __builtin_debugtrap()
#else
#define NCL_DEBUG_BREAK() ::std::abort()
#endif // _MSC_VER

#define NCL_ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))