#pragma once
#include <assert.h>

#ifndef _DEBUG
	#define NCL_DEBUG 0
#else
	#define NCL_DEBUG 1
#endif // NDEBUG

#ifndef NCL_ASSERT
	#if NCL_DEBUG
		#define NCL_ASSERT(x) assert(x)
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
#define ANKER_DEBUG_BREAK() __builtin_trap()
#elif __clang__
#define ANKER_DEBUG_BREAK() __builtin_debugtrap()
#else
#define ANKER_DEBUG_BREAK() ::std::abort()
#endif // _MSC_VER
