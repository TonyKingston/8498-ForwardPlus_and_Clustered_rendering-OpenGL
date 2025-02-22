#pragma once
#include <assert.h>

#ifndef _DEBUG
	#define NCL_DEBUG 0
#else
	#define NCL_DEBUG 1
#endif // NDEBUG

#ifndef NCL_ASSERT
	#if NCL_DEBUG
		#define NCL_ASSERT(x) \
        do { \
            if(!(_cond)) { \
                NCL_FATAL("Assertion: {}\n\t{}:{}", XSTR(_cond), __FILE__, __LINE__); \
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

// X-Macros to string
#define XSTR(x) STR_IMPL(x)
#define STR_IMPL(x) #x

#define XCONCAT(x, y) CONCAT_IMPL(x, y)
#define CONCAT_IMPL(x, y) x##y

#define NCL_COUNTER __COUNTER__

#define NCL_NAME "NCL"

#define NCL_VERSION_MAJOR 1
#define NCL_VERSION_MINOR 0
#define NCL_VERSION_REVISION 0
#define NCL_MAKE_VERSION(major, minor, revision) "v" XSTR(major) "." XSTR(minor) "." XSTR(revision)

#define NCL_VERSION NCL_MAKE_VERSION(NCL_VERSION_MAJOR, NCL_VERSION_MINOR, NCL_VERSION_REVISION)