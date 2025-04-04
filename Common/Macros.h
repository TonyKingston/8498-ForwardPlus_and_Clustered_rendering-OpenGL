#pragma once

#ifndef _DEBUG
#define NCL_DEBUG 0
#else
#define NCL_DEBUG 1
#endif // NDEBUG

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

#define NCL_SAFE_FREE(x) if(x != nullptr) free(x); x = nullptr;

#ifdef NCL_NO_FORCE_INLINE // Forcing inlining is the default behaviour
	#define NCL_INLINE inline
#elif defined(_MSC_VER)
#define NCL_INLINE __forceinline
#elif defined(__clang__) || defined(__GNUC__)
#define NCL_INLINE __inline__ __attribute__((always_inline))
#else
#define NCL_INLINE inline // Fallback to regular inline, might be best to notify with an error instead
#endif

#ifndef XSTR
	#define XSTR(x) STR_IMPL(x)
	#define STR_IMPL(x) #x
#endif 

#ifndef XCONCAT
	#define XCONCAT(x, y) CONCAT_IMPL(x, y)
	#define CONCAT_IMPL(x, y) x##y
#endif

#define ANON(Name) XCONCAT(Name, __COUNTER__)

#ifndef NCL_NO_UNIQUE_ADDRESS
#ifdef _MSC_VER
	#define NCL_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
	#define NCL_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif // _MSC_VER
#endif // NCL_NO_UNIQUE_ADDRESS