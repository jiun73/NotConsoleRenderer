#include "Game.h"
#include <iomanip>

double get_time(double& time)
{
	double aug = 1.0 / frame_rate;
	time += aug;
	return time;
}

void show_time(double time)
{
	pencil(COLOR_WHITE);
	string temps = reel_en_chaine(time);
	draw_simple_text(temps, { 500,500 }, get_font(0));
}

void chute_libre(square& main, double& time, int& vi, bool& touchGound)
{
	int energyLoss = 0.5;
	if (main.get_pos().y + main.get_xy().y < Y_CONSOLE && !touchGound)
	{
		main.get_pos().y = (9.8 * time * time) / 2;
	}
	else if (touchGound)
	{
		if (-119.7 / 2 * time + (-9.8 * time * time) / 2 + 730 > 0)
		{
			main.get_pos().y = -119.7 / 2 * time + -9.8 * time * time / 2 + 730;
		}
	}
	else
	{
		touchGound = true;
		time = 0;
	}
}

void projectile_horizontal(square& main, double time)
{
	double Vi = 100;
	int Xi = 30;
	int coefficient = 10;
	if (main.get_pos().y + 20 < Y_CONSOLE)
	{
		main.get_pos().x = Vi * time + Xi;
		main.get_pos().y = coefficient * time * time * 9.8 / 2;
	}
}

void projectile_oblique(square& main, double time, int& angle)
{
	double Vi = 50.0;
	int Xi = 30;
	int coefficient = 10;
	if (main.get_pos().y + 20 < Y_CONSOLE)
	{
		main.get_pos().x = Vi * time + 30;
		double trajectoire = Vi * sin(angle);
		main.get_pos().y = trajectoire + Y_CONSOLE / 2;
		if (angle == 360)
		{
			angle = 0;
		}
		else
		{
			angle++;
		}
	}
}

double convertir_en_radians(int angle)
{
	double newAngle = static_cast<double>(angle);
	double angleInRad = newAngle * 3.14159265 / 180.0000000;
	return angleInRad;
}

void circle_on_axis(square& main, V2d_i axisPosition, double radius, int& angle)
{
	double newAngle = static_cast<double>(angle);
	double angleInRad = newAngle * 3.14159265 / 180.0000000;
	double sinus = sin(angleInRad);
	double cosinus = cos(angleInRad);
	int x = cosinus * radius + axisPosition.x;
	int y = sinus * radius + axisPosition.y;
	main.get_pos().x = x;
	main.get_pos().y = y;
	angle++;
}

void make_main_rotate(V2d_i& pos, int& angle)
{
	double newAngle = static_cast<double>(angle);
	double angleInRad = newAngle * 3.14159265 / 180.0000000;
	double sinus = sin(angleInRad);
	double cosinus = cos(angleInRad);
	pos.x = cosinus * pos.x;
	pos.y = sinus * pos.y;
	pencil(COLOR_WHITE);
	draw_pix(pos);
}

void rotate(vector<V2d_i>& vect, int& angle)
{
	for (int i = 0; i < vect.size(); i++)
	{
		make_main_rotate(vect.at(i), angle);
	}
	angle++;
}

int main()
{
	V2d_i window = { X_CONSOLE, Y_CONSOLE };
	set_window_size(window);
	set_window_resizable();

	square main;
	main.get_pos() = { X_CONSOLE  / 2,Y_CONSOLE / 2 };
	main.is_main = true;
	main.velocite = 0;

	shape carre;

	double time = 0;
	int vi = 0;
	bool touchGround = false;
	int angle = 0;
	V2d_i axis = { X_CONSOLE / 2, Y_CONSOLE / 2 };
	int allow = 0;
	int max = 3;
	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
		allow++;
		time = get_time(time);
		//chute_libre(main,time, vi, touchGround);
		//projectile_horizontal(main, time);
		//projectile_oblique(main, time, angle);
		/*if (allow == max)
		{
			circle_on_axis(main, axis, 350, angle);
			allow = 0;
		}*/
		//rotate(main.get_points(), angle);
		carre.create();
		carre.move();
		carre.rotate_on_pos(angle);
		carre.rotate_on_center(angle); // bug: quand on se déplace puis on spin, la carré revient à sa position initiale. Quand j'essaie de fixer cela, la carré sort complètement de l'écran
		carre.show_coordinates();
		main.create();
		//main.show_coordinates();
		show_time(time);
	}
}