#pragma once
#include "RAS.h"
#include <array>
#include "Fields.h"

namespace FF
{
	enum FMods
	{
		STATIC,
		LINEAR,
		QUAD,
		MVT_STATIC,
		MVT_LINEAR,
		MVT_QUAD
	};

	using std::array;

	template<typename T>
	inline size_t static_func(RAS::Time time, T& i, array<double, 1> arr)
	{
		i = arr.at(0);
		return STATIC;
	}

	template<typename T>
	inline void const_func(RAS::Time time, T& x, const T& value)
	{
		x = value;
	}

	inline size_t linear_func(RAS::Time time, double& i, array<double, 2> arr)
	{
		double start = arr.at(0);
		double speed = arr.at(1);
		i = ((time / 1000.0) * speed) + start;
		return LINEAR;
	}

	inline RAS::Time linear_inverse(double y, array<double, 2> arr)
	{
		double start = arr.at(0);
		double speed = arr.at(1);
		return ((y - start) / speed) * 1000.0;
	}

	inline size_t mvt_static(RAS::Time time, Mvt& mvt, array<double,1> arr)
	{ 
		mvt.acc = 0;
		mvt.vel = 0;
		mvt.pos = arr[0];
		return MVT_STATIC;
	}

	inline size_t mvt_linear(RAS::Time time, Mvt& mvt, array<double, 2> arr)
	{
		double x = arr.at(0);
		double v = arr.at(1);
		double dt = (time / 1000.0);
		mvt.acc = 0;
		mvt.vel = 0;
		mvt.pos = (dt * v) + x;
		return MVT_STATIC;
	}

	inline size_t mvt_quad(RAS::Time time, Mvt& mvt, array<double, 3> arr)
	{
		double x = arr.at(0);
		double vi = arr.at(1);
		double a = arr.at(2);
		double dt = (time / 1000.0);
		mvt.acc = 0;
		mvt.vel = (dt * a) + vi;
		mvt.pos = (dt * vi) + (0.5 * a * dt * dt) + x;
		return MVT_QUAD;
	}
}