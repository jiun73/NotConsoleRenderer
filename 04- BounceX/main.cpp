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

void projectile_oblique(square& main, double time)
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

void cercle(V2d_i pos, double radius)
{
	pencil(COLOR_WHITE);
	for (int i = 0; i <= 380; i++)
	{
		double sinus = sin(i);
		double cosinus = cos(i);
		int x = cosinus * radius;
		int y = sinus * radius;
		draw_pix({ pos.x + x, pos.y + y});
	}
}

int main()
{
	V2d_i window = { X_CONSOLE, Y_CONSOLE };
	set_window_size(window);
	set_window_resizable();

	square main;
	main.is_main = true;
	main.velocite = 0;
	int Yi = Y_CONSOLE / 2;
	main.get_pos() = { 30,Yi };

	double time = 0;
	int vi = 0;
	bool touchGround = false;
	while (run())
	{
		//time = get_time(time);
		//pencil(COLOR_BLACK);
		//draw_clear();
		////chute_libre(main,time, vi, touchGround);
		////projectile_horizontal(main, time);
		projectile_oblique(main, time);
		main.create();
		main.show_coordinates();
		//show_time(time);
		cercle({750,300},30.0000);
	}
}