#include "pch.h"
#include "Shape.h"

function<V2d_d(V2d_d)> global_shape_transform = nullptr;
uint32_t global_shape_transform_dt = 0;

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

V2d_d Shape_x::find_point(V2d_d p)
{
	double norm = p.norm();
	double orientation = p.orientation() + angle;

	if (global_shape_transform)
	{
		return global_shape_transform(p.polar(norm, orientation) * scale + position);
	}

	return (p.polar(norm, orientation) * scale + position);
}

void Shape_x::draw(Camera& cam)
{
	if (points.empty()) return;
	V2d_i p1, p2;

	for (auto it = points.begin() + 1; it != points.end(); it++)
	{
		p1 = cam.find(find_point(*std::prev(it)).floor());
		p2 = cam.find(find_point(*it).floor());
		draw_line(p1, p2);
	}

	p1 = cam.find(find_point(points.front()).floor());
	p2 = cam.find(find_point(points.back()).floor());

	draw_line(p1, p2);

	//draw_circle(support(-getAngleTowardPoint(position, mouse_position()) + (0.5 * M_PI)), 10);
	//draw_circle(support_op(-getAngleTowardPoint(position, mouse_position()) + (0.5 * M_PI)), 10);
}

V2d_d Shape_x::mean()
{
	V2d_d sum = 0;
	for (auto p : points)
		sum += p;

	return sum / points.size();
}

Shape_x Shape_x::get_mincowski(Shape_x& other)
{
	Shape_x md;

	std::vector<V2d_d> allp;
	std::set<double> angles;
	std::set<V2d_d> maxp;

	for (int i = 0; i < 1000; i++)
	{
		double angle = 2.0 * M_PI * ((double)i / 1000);
		md.points.push_back(support(angle) - other.support(angle + M_PI));
	}

	return md;
}

void Shape_x::calc_support_points()
{
	support_regions.clear();
	V2d_d _mean = mean();

	for (auto it = points.begin() + 1; it != points.end(); it++)
	{
		calc_support_edge(*(it - 1) - _mean, (*it) - _mean);
	}
	calc_support_edge(points.back() - _mean, points.front() - _mean);
}

void Shape_x::calc_support_edge(const V2d_d& point1, const V2d_d& point2)
{
	double region;

	if (abs(point2.x - point1.x) < 0.1) //edge case
	{
		if (point2.x >= 0)
			region = M_PI;
		else
			region = 0;
	}
	else
	{
		double A = (point2.y - point1.y) / (point2.x - point1.x);
		double B = point1.y - A * point1.x;
		double X = -(A * B) / (A * A + 1);
		double Y = A * X + B; //gets point closest to origin (perpendicular to edge)
		region = V2d_d(X, Y).orientation() + M_PI;
	}

	support_regions.emplace(region, point2);
}

V2d_d Shape_x::support_op(double support_angle)
{
	V2d_d _mean = mean();
	support_angle += M_PI;
	support_angle -= angle;
	support_angle -= (2 * M_PI) * std::floor(support_angle * (1 / (2 * M_PI)));

	if (needs_recalc)
	{
		calc_support_points();
		needs_recalc = false;
	}

	auto it = support_regions.lower_bound(support_angle);

	if (it == support_regions.end())
		return find_point(support_regions.begin()->second + _mean);

	return find_point(it->second + _mean);
}

V2d_d Shape_x::support(double support_angle)
{
	support_angle -= (2 * M_PI) * std::floor(support_angle * (1 / (2 * M_PI)));
	V2d_d _mean = mean();
	V2d_d* support = nullptr;
	V2d_d direction;

	direction.polar(1.0, support_angle - angle); //TODO: yikes! this calls sin and cos

	double max = 0;
	for (auto& p : points)
	{
		V2d_d relative = p - _mean;
		double dot = direction.dot(relative);
		if (dot > max)
		{
			support = &p;
			max = dot;
		}
	}

	if (support == nullptr) return 0;

	return find_point(*support);
}