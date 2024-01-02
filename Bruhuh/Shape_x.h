#pragma once
#include "EntityX.h"
#include "Vec2D.h"
#include "TileRenderer.h"

#include <set>

namespace AP22
{
	//get max vector in list
	V2d_d max_vec(std::vector<V2d_d>& list, V2d_d start);

	//get max vector in list
	V2d_d min_vec(std::vector<V2d_d>& list, V2d_d start);
}

inline double getAngleTowardPoint(V2d_d origin, V2d_d target)
{
	return (-atan2(double(origin.y - target.y), double(origin.x - target.x)) - (M_PI / 2));
}

inline V2d_d tripleCrossProduct(const V2d_d& c, const V2d_d& b, const V2d_d& a)
{
	return
	{
		a.y * (b.x * c.y - b.y * c.x),
		a.x * (b.y * c.x - b.x * c.y)
	};
}

extern function<V2d_d(V2d_d)> global_shape_transform;
extern uint32_t global_shape_transform_dt;

struct Shape_x
{
private:
	vector<V2d_d> points;
	map<double, V2d_d> support_regions;
	bool needs_recalc = true;

public:
	V2d_d position = 0;
	double angle = 0;
	double scale = 1;

	Shape_x() {}
	Shape_x(const vector<V2d_d>& points) : points(points) {}
	~Shape_x() {}

	vector<V2d_d>& get_points() 
	{
		needs_recalc = true;
		return points;
	}

	const vector<V2d_d>& read_points() { return points; }
	vector<V2d_d>& get_points_no_recalc() { return points; }

	V2d_d find_point(V2d_d p);

	void draw();

	V2d_d mean();

	Shape_x get_mincowski(Shape_x& other);

	void calc_support_points();
	void calc_support_edge(const V2d_d& p1, const V2d_d& p2);

	V2d_d support_op(double angl);
	V2d_d support(double support_angle);
};



struct GJK 
{
	vector<V2d_d> simplex;

	bool lineCase(V2d_d& dir)
	{
		V2d_d B = simplex[0];
		V2d_d A = simplex[1];

		V2d_d AB = B - A;
		V2d_d AO = -A;
		V2d_d ABperp = tripleCrossProduct(AB, AO, AB);
		dir = ABperp;
		return false;
	}

	bool triangleCase(V2d_d& dir)
	{
		V2d_d C = simplex[0];
		V2d_d B = simplex[1];
		V2d_d A = simplex[2];

		V2d_d AB = B - A;
		V2d_d AC = C - A;
		V2d_d AO = -A;
		V2d_d ABperp = tripleCrossProduct(AC, AB, AB);
		V2d_d ACperp = tripleCrossProduct(AB, AC, AC);
		if (ABperp.dot(AO) >= 0)
		{
			simplex.erase(simplex.begin());
			dir = ABperp;
			return false;
		}
		else if (ACperp.dot(AO) >= 0)
		{
			simplex.erase(simplex.begin() + 1);
			dir = ACperp;
			return false;
		}
		return true;
	}

	bool handleSimplex(V2d_d& direction)
	{
		if (simplex.size() == 2)
			return lineCase(direction);
		return triangleCase(direction);
	}

	template<class S1, class S2>
	V2d_d get_minkowski_support( S1& shape1,  S2& shape2, double angle)
	{
		return shape1.support(angle) - shape2.support(angle + M_PI);
	}

	template<class S1, class S2>
	bool find( S1& shape1,  S2& shape2)
	{
		simplex.clear();
		V2d_d direction;
		double angle;

		simplex.push_back(get_minkowski_support(shape1, shape2, 0.3)); //start with direction 0
		direction = -simplex.at(0);

		while (true)
		{
			angle = direction.orientation();
			V2d_d new_point = get_minkowski_support(shape1, shape2, angle);
			if (new_point.dot(direction) <= 0.0)
				return false;

			simplex.push_back(new_point);

			if (handleSimplex(direction))
				return true;
		}
	}

	void visualize(Shape_x& shape1, Shape_x& shape2)
	{
		Shape_x simplex_shape(simplex);
		simplex_shape.position += 250;

		pencil(COLOR_PINK);
		draw_line({ 250,0 }, { 250,500 });
		draw_line({ 0,250 }, { 500,250 });
		
		pencil(COLOR_GREEN);
		Shape_x	mincowski_diff = shape1.get_mincowski(shape2);
		mincowski_diff.position = 250;
		mincowski_diff.draw();
		pencil(COLOR_WHITE);
		simplex_shape.draw();
		pencil(COLOR_CYAN);
		draw_line(edge.first + 250, edge.second + 250);
		pencil(COLOR_PINK);
		draw_line(250, point + 250);
		
	}

	double get_edge_minimum_distance_from_origin(const V2d_d& point1, const V2d_d& point2, V2d_d& point)
	{
		if (abs(point2.x - point1.x) < 0.1) //edge case
		{
			V2d_d ABperp = { 1,0 };
			point = { point2.x,0 };
			return abs(ABperp.dot(point));
		}

		double A = (point2.y - point1.y) / (point2.x - point1.x);
		double B = point1.y - A * point1.x;
		double X = -(A * B) / (A * A + 1);
		double Y = A * X + B; //closest point

		V2d_d AB = point2 - point1; //get a vector perpendicular to the edge
		V2d_d AO = -point1;
		V2d_d ABperp = tripleCrossProduct(AB, AO, AB);
		ABperp.normalize();

		point = { X,Y };

		return abs(ABperp.dot(point)); //not the actual distance lol
	}

	pair<V2d_d, V2d_d> find_edge_closest_origin(double& min, std::vector<V2d_d>::const_iterator& ret, V2d_d& point)
	{
		pair<V2d_d, V2d_d> edge;
		V2d_d buf;

		min = std::numeric_limits<double>::infinity();

		for (auto it = simplex.cbegin() + 1; it != simplex.cend(); it++)
		{
			
			double distance = get_edge_minimum_distance_from_origin(*(it - 1), *it, buf);
			if (distance < min)
			{
				ret = it;
				min = distance;
				edge = { *(it - 1), *it };
				point = buf;
			}
		}

		double distance = get_edge_minimum_distance_from_origin(simplex.front(), simplex.back(), buf);

		if (distance < min)
		{
			min = distance;
			ret = simplex.end();
			point = buf;
			return {simplex.front(), simplex.back()};
		}
		else
		{
			return edge;
		}
	}

	double distance;
	V2d_d point;
	pair<V2d_d, V2d_d> edge;

	V2d_d EPA(Shape_x& shape1, Shape_x& shape2)
	{
		double min = std::numeric_limits<double>::infinity();

		while (true)
		{
			std::vector<V2d_d>::const_iterator it;
			edge = find_edge_closest_origin(distance, it, point);

			V2d_d B = edge.first;
			V2d_d A = edge.second;

			V2d_d AB = B - A;
			V2d_d AO = -A;
			V2d_d ABperp = -tripleCrossProduct(AB, AO, AB);

			V2d_d new_point = get_minkowski_support(shape1, shape2, ABperp.orientation());
			//*(it - 1) == new_point || (it != simplex.end()) && *(it) == new_point ???
			//std::count(simplex.begin(), simplex.end(), new_point)
			if (std::count(simplex.begin(), simplex.end(), new_point)) //we're on the edge of the md, return
			{
				if (distance < min)
				{
					min = distance;
				}
				else
				{
					return point;
				}
			}
			else
				simplex.insert(it, new_point); //still not on the edge, keep looking

			
		}
	}
};