#pragma once
#include <assert.h>

#ifndef NCL_DEBUG || NDEBUG
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
