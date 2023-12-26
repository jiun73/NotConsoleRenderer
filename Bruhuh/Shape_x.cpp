#include "Shape_x.h"

V2d_d AP22::max_vec(std::vector<V2d_d>& list, V2d_d start)
{
	V2d_d max = start;
	for (auto p : list)
	{
		if (p.x > max.x)
			max.x = p.x;
		if (p.y > max.y)
			max.y = p.y;
	}
	return max;
}

V2d_d AP22::min_vec(std::vector<V2d_d>& list, V2d_d start)
{
	V2d_d min = start;
	for (auto p : list)
	{
		if (p.x < min.x)
			min.x = p.x;
		if (p.y < min.y)
			min.y = p.y;
	}
	return min;
}