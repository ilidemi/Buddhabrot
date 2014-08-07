#pragma once

template <typename T>
struct AtomWrapper
{
	std::atomic<T> _a;

	AtomWrapper()
		:_a()
	{}

	AtomWrapper(const std::atomic<T> &a)
		:_a(a.load())
	{}

	AtomWrapper(const AtomWrapper &other)
		:_a(other._a.load())
	{}

	AtomWrapper & operator=(const AtomWrapper &other)
	{
		_a.store(other._a.load());
	}
	
	T operator++()
	{
		return ++_a;
	}
	
	T operator++(int)
	{
		return _a++;
	}
	
	T operator+=(T val)
	{
		return _a.fetch_add(val);
	}
};