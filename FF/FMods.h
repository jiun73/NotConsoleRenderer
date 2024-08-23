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
		mvt.vel = v;
		mvt.pos = (dt * v) + x;
		return MVT_LINEAR;
	}

	inline size_t mvt_quad(RAS::Time time, Mvt& mvt, array<double, 3> arr)
	{
		double x = arr.at(0);
		double vi = arr.at(1);
		double a = arr.at(2);
		double dt = (time / 1000.0);
		mvt.acc = a;
		mvt.vel = (dt * a) + vi;
		mvt.pos = (dt * vi) + (0.5 * a * dt * dt) + x;
		return MVT_QUAD;
	}

	inline vector<RAS::Time> mvt_linear_inverse(Mvt mvt, array<double, 2> arr)
	{
		double x = arr.at(0);
		double v = arr.at(1);

		if (v == 0 && mvt.pos != x)
			return {};

		RAS::Time dt = ((mvt.pos - x) / v) * 1000.0;
		return { dt };
	}

	inline vector<RAS::Time> quad_root(array<double, 3> arr)
	{
		double c = arr.at(0);
		double b = arr.at(1);
		double a = arr.at(2);

		double r = (b * b) + (2 * a * c);

		if (r < 0)
			return {};

		RAS::Time dt1 = ((-b - sqrt(r)) / (a)) * 1000;

		if (r == 0)
			return { dt1 };

		RAS::Time dt2 = ((-b + sqrt(r)) / (a)) * 1000;

		if (dt1 > dt2)
			return { dt1, dt2 };
		else
			return { dt2, dt1 };
	}

	inline vector<RAS::Time> mvt_quad_inverse(Mvt mvt, array<double, 3> arr)
	{
		return quad_root({ mvt.pos - arr[0], arr[1], arr[2] });
	}

	inline array<double, 3> mvt_mod_to_quad(RAS::Modifier* mod)
	{
		if (!mod->is_reversible()) return {};

		vector<double> vec = mod->reverse_params();

		switch (mod->reverse_type())
		{
		case MVT_QUAD:
			return { vec.at(0), vec.at(1), vec.at(2) };
		case MVT_LINEAR:
			return { vec.at(0), vec.at(1) };
		case MVT_STATIC:
			return { vec.at(0) };
		default:
			return {};
		}
	}

	struct TimeRange 
	{
		RAS::Time time_begin = 0;
		RAS::Time time_end = 0;
	};

	inline vector<TimeRange> find_overlap_range(const array<double, 3>& quad1_top, const array<double, 3>& quad1_bottom , const array<double, 3>& quad2_top, const array<double, 3>& quad2_bottom)
	{
		
	}
}