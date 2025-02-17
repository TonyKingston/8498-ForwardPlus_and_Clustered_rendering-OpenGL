#pragma once
#include <Build.h>

template<class T>
class Singleton
{
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

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

	void Reset() { instance->~T(); }
};