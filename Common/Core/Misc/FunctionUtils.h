#pragma once

namespace NCL {
	/* Inherit from to delete copy functions */
	class NonCopyable
	{
	public:
		NonCopyable() = default;
		NonCopyable(const NonCopyable& copy) = delete;
		NonCopyable& operator=(const NonCopyable& copy) = delete;
	};

	/* Inherit from to delete move functions */
	class NonMoveable
	{
	public:
		NonMoveable() = default;
		NonMoveable(NonMoveable&& move) = delete;
		NonMoveable& operator=(NonMoveable&& move) = delete;
	};

	// Alternative macro to do both
	#define NCL_NONCOPYABLE(TypeName) \
		TypeName(TypeName&&) = delete; \
		TypeName(const TypeName&) = delete; \
		TypeName& operator=(const TypeName&) = delete; \
		TypeName& operator=(TypeName&&) = delete;
}

