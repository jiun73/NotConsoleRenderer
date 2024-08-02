#pragma once
#include "RAS.h"
#include <array>

namespace FF
{
	enum FMods 
	{
		STATIC,
		LINEAR,
		QUAD
	};

	using std::array;

	template<typename T>
	inline void static_func(RAS::Time time, T& i, array<double, 1> arr)
	{
		i = arr.at(0);
	}

	template<typename T>
	inline void const_func(RAS::Time time, T& x, const T& value)
	{
		x = value;
	}

	inline void linear_func(RAS::Time time, int& i, array<double, 2> arr)
	{
		double start = arr.at(0);
		double speed = arr.at(1);
		i = ((time / 1000.0) * speed) + start;
	}

	inline RAS::Time linear_inverse(int y, array<double, 2> arr)
	{
		double start = arr.at(0);
		double speed = arr.at(1);
		return ((y - start) / speed) * 1000.0;
	}
}