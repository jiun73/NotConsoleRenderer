#include "TileRenderer.h"

struct Shape 
{
	std::vector<V2d_i> points;
	V2d_d position = 0;
	double angle = 0;

	V2d_d find_point(V2d_d p)
	{
		double norm = p.norm();
		double orientation = p.orientation() + angle;

		return p.polar(norm, orientation) + position;
	}

	void draw() 
	{
		for (auto it = points.begin() + 1; it != points.end(); it++)
		{
			draw_line((V2d_i)find_point(*(it - 1)), (V2d_i)find_point(*it));
		}

		draw_line((V2d_i)find_point(points.front()), (V2d_i)find_point(points.back()));
	}
};

double getAngleTowardPoint(V2d_d origin, V2d_d target)
{
	return (-atan2(double(origin.y - target.y), double(origin.x - target.x)) - (M_PI / 2));
}

struct Object
{
	Shape shape;
	int life = 100;
	V2d_d vel = 0;
	bool circle = false;
	Color color;
};

int main()
{
	set_window_size(300);
	set_window_resizable();

	Shape s;
	s.points = { -5,{10,0},{-5,5} };

	int cntr = 0;

	std::vector<Object> objects;

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();

		pencil(rgb(255, 255, 0));

			V2d_d old = s.position;
			keyboard().quickKeyboardControlWASD(s.position, 1);
			V2d_d dis = old - s.position;

			if (dis != 0 && random().range(0,3) ==3)
			{
				dis.normalize();
				double ori = dis.orientation() + random().frange(-0.3, 0.3);

				Object o;
				o.shape.position = old;
				o.vel.polar(10, ori);
				o.circle = true;
				o.life = 30;
				objects.push_back(o);
			}

		if (mouse().released(1))
		{
			double ang = -getAngleTowardPoint(s.position, get_true_mouse_pos()) + (0.5 * M_PI);
			Object bullet;
			bullet.shape.points = { -5, 5, {-5,5 } };
			bullet.shape.position = s.position;
			bullet.shape.angle = random().frange(-2 * M_PI, 2 * M_PI);
			bullet.life = 3000;
			bullet.vel.polar(3, ang);

			objects.push_back(bullet);
		}

		s.draw();

		draw_line({ 10,0 }, { 0,10 });
		draw_line({ 10,0 }, { 20,10 });
		draw_line({ 5,5 }, { 15,5 });

		draw_line({ 30, 0 }, { 30,10 });
		draw_line({ 30, 10 }, { 40,10 });
		draw_line({ 50, 0 }, { 50,10 });
		draw_line({ 50, 10 }, { 60,10 });

		draw_circle({ 75,5 }, 5);

		s.angle = -getAngleTowardPoint(s.position, get_true_mouse_pos()) + (0.5 * M_PI);

		for (auto it = objects.begin(); it != objects.end(); it++)
		{
			it->shape.position += it->vel;
			it->life--;
			if (it->circle)
			{
				pencil(rainbow(1000));
				draw_circle(it->shape.position, it->life / 3);
				it->vel /= 2;
			}
			else
			{
				pencil(it->color);
				it->shape.draw();
			}
			

			if (it->life == 0)
			{
				it = objects.erase(it);

				if (it == objects.end())
					break;
			}
		}
	}

	return 0;
}