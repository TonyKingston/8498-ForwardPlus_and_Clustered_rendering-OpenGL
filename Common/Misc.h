#pragma once
#include <Build.h>

template<class T>
class Singleton {
public:
	static T& Get() { 
		static T* instance = new T();
		NCL_ASSERT(instance);
		return *instance;
	}

	static void Destroy() {
		Reset();
	}

protected:

	Singleton() = default;
	virtual ~Singleton() = default;

private:

	friend T;

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

	void Reset() { Get().~T(); }
};

// Inherit when using the curiously recurring template pattern (CRTP)
template<typename T>
class CRTP
{
public:
	T& Impl() { return static_cast<T&>(*this); }
	T const& Impl() const { return static_cast<T const&>(*this); }
private:
};