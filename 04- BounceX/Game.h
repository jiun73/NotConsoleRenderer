#pragma once

#include "TileRenderer.h"
#include "Strings.h"
#include <ctime>

const int X_CONSOLE = 1000;
const int Y_CONSOLE = 750;

const double frame_rate = 120.0;

struct physics
{
	V2d_d acceleration = { 0,0.01 };
	V2d_d velocite = { 0 };
};

struct chrono 
{
private:
	double time = 0;
	double aug = 1.0 / frame_rate;
	bool stop = false;

	double increase()
	{
		if (!stop)
		{
			time += aug;
			return time;
		}
	}
public:
	void start()
	{
		increase();
	}
	void reset()
	{
		time = 0;
	}
	void pause()
	{
		stop = true;
	}
	void restart()
	{
		stop = false;
		increase();
	}
	double actual()
	{
		return time;
	}
};

struct square
{
private:
	Rect rect(V2d_i& pos, V2d_i xy, V2d_i velocite)
	{
		pencil(COLOR_GREEN);
		if (is_main)
		{
			pencil(COLOR_WHITE);
		}
		draw_rect(Rect({ pos, xy }));
		pos += velocite;
		return { pos,xy };
	}
	int choose()
	{
		int x = random().range(0, 1);
		if (x == 0)
		{
			return -1;
		}
		return 1;
	}
	V2d_i pos = { random().range(xy.x * 2,X_CONSOLE - xy.x),random().range(xy.y * 2,Y_CONSOLE - xy.y) };
	V2d_i xy = { 20,20 };
	vector<V2d_i> points;

	void change_velocity(V2d_i pos, V2d_i xy, V2d_i& velocite)
	{
		string son = "doing.wav"; //"../TileRenderer/Sounds/rizz.wav";
		if (pos.y == 0)
		{
			velocite.y = 1;
			sound().playSound(son);
		}
		else if (pos.y + xy.y == Y_CONSOLE)
		{
			sound().playSound(son);
			velocite.y = -1;
		}
		else if (pos.x == 0)
		{
			sound().playSound(son);
			velocite.x = 1;
		}
		else if (pos.x + xy.x == X_CONSOLE)
		{
			sound().playSound(son);
			velocite.x = -1;
		}
	}
	
	void move_main(V2d_i& pos, V2d_i xy, V2d_i velocite)
	{
		if (key_held(SDL_SCANCODE_W) && pos.y > 0)
		{
			velocite = { 0,-1 };
		}
		else if (key_held(SDL_SCANCODE_S) && pos.y + xy.y < Y_CONSOLE)
		{
			velocite = { 0,1 };
		}
		if (key_held(SDL_SCANCODE_A) && pos.x > 0)
		{
			velocite = { -1,0 };
		}
		else if (key_held(SDL_SCANCODE_D) && pos.x + xy.x < X_CONSOLE)
		{
			velocite = { 1,0 };
		}
		pos.x += velocite.x * acceleration;
		pos.y += velocite.y * acceleration;
	}
public:
	int vi = 0;
	bool touchGround = false;
	bool begin = false;
	V2d_i velocite = { choose(),choose() };
	int acceleration = 5;
	bool is_main = false;
	physics physique;
	V2d_i create()
	{
		rect(pos, xy, velocite);
		if (!is_main)
		{
			change_velocity(pos, xy, velocite);
		}
		else
		{
			move_main(pos, xy, velocite);
		}
		return pos;
	}
	bool main_touched = false;
	void show_coordinates()
	{
		pencil(COLOR_WHITE);
		setlocale(LC_ALL, "");
		string texte = "Vos coordonnees sont: {";
		string x = entier_en_chaine(pos.x);
		string y = entier_en_chaine(pos.y);
		texte += x;
		texte += ":" + y + "}";
		draw_simple_text(texte, { 100,10 }, get_font(0));
	}
	V2d_i& get_pos()
	{
		return pos;
	}
	V2d_i get_xy()
	{
		return xy;
	}
	vector<V2d_i>& get_points()
	{
		for (int n = pos.x; n <= pos.x + xy.x; n++)
		{
			for (int i = pos.y; i <= pos.y + xy.y; i++)
			{
				points.push_back({ n,i });
			}
		}
		return points;
	}
};

struct shape
{
private:
	V2d_i xy = { 200,200 };
	V2d_i pos = {X_CONSOLE / 2 - xy.x / 2, Y_CONSOLE / 2 - xy.y / 2};
	V2d_i topright = { pos.x + xy.x,pos.y };
	V2d_i bottomleft = { pos.x, pos.y + xy.y};
	V2d_i bottomright = {pos.x + xy.x, pos.y + xy.y};
	V2d_i centre = { pos.x + (topright.x - pos.x) / 2, pos.y + (bottomleft.y - pos.y) / 2 };
	int angle = 0;
	double convertir_en_radians(int angle)
	{
		double newAngle = static_cast<double>(angle);
		double angleInRad = newAngle * 3.14159265 / 180.0000000;
		return angleInRad;
	}

public:
	void create()
	{
		pencil(COLOR_PINK);
		draw_line(pos, topright);
		draw_line(pos, bottomleft);
		draw_line(bottomleft, bottomright);
		draw_line(topright, bottomright);
	}
	void move()
	{
		if (key_held(SDL_SCANCODE_W))
		{
			pos.y--;
			topright.y--;
			bottomright.y--;
			bottomleft.y--;
		}
		if (key_held(SDL_SCANCODE_S))
		{
			pos.y++;
			topright.y++;
			bottomright.y++;
			bottomleft.y++;
		}
		if (key_held(SDL_SCANCODE_A))
		{
			pos.x--;
			topright.x--;
			bottomright.x--;
			bottomleft.x--;
		}
		if (key_held(SDL_SCANCODE_D))
		{
			pos.x++;
			topright.x++;
			bottomright.x++;
			bottomleft.x++;
		}
		centre = { pos.x + (topright.x - pos.x) / 2, pos.y + (bottomleft.y - pos.y) / 2 };
	}
	void rotate_on_pos()
	{
		if (key_held(SDL_SCANCODE_SPACE))
		{
			/*int x = cos(angleEnRadians) * rayon + centreDeRotation.x;
			  int y = sin(angleEnRadians) * rayon + centreDeRotation.y;*/
			topright.x = xy.x * cos(convertir_en_radians(angle)) + pos.x;
			topright.y = xy.y * sin(convertir_en_radians(angle)) + pos.y;
			bottomright.x = sqrt(xy.x * xy.x + xy.y * xy.y) * cos(convertir_en_radians(angle - 315)) + pos.x;
			bottomright.y = sqrt(xy.x * xy.x + xy.y * xy.y) * sin(convertir_en_radians(angle - 315)) + pos.y;
			bottomleft.x = xy.x * cos(convertir_en_radians(angle - 270)) + pos.x;
			bottomleft.y = xy.y * sin(convertir_en_radians(angle - 270)) + pos.y;
			angle++;
		}
	}
	void rotate_on_center()
	{
		// centre du rectangle = {pos.x + (topright.x - pos.x) / 2, pos.y + (bottomleft.y - pos.y) / 2}
		if (key_held(SDL_SCANCODE_Q))
		{
			if (key_held(SDL_SCANCODE_J))
			{
				std::cout << "allo";
			}
			double rayon = sqrt(20000); //sqrt(pow((xy.x / 2), 2) + pow((xy.y / 2), 2));
			pos.x = rayon * cos(convertir_en_radians(angle - 135)) + centre.x;
			pos.y = rayon * sin(convertir_en_radians(angle - 135)) + centre.y;
			topright.x = rayon * cos(convertir_en_radians(angle - 45)) + centre.x;
			topright.y = rayon * sin(convertir_en_radians(angle - 45)) + centre.y;
			bottomright.x = rayon * cos(convertir_en_radians(angle - 315)) + centre.x;
			bottomright.y = rayon * sin(convertir_en_radians(angle - 315)) + centre.y;
			bottomleft.x = rayon * cos(convertir_en_radians(angle - 225)) + centre.x;
			bottomleft.y = rayon * sin(convertir_en_radians(angle - 225)) + centre.y;
			angle++;
		}
	}
	void bounce_on_walls()
	{
		pos.x++;
		pos.y = sin(convertir_en_radians(angle + 90));
		
		bottomleft.x++;
		pos.y = sin(convertir_en_radians(angle + 270));

		bottomright.x++;
		bottomright.y = sin(convertir_en_radians(angle + 270));

		topright.x++;
		topright.y = sin(convertir_en_radians(angle + 90));
	}
	void show_coordinates()
	{
		pencil(COLOR_WHITE);
		setlocale(LC_ALL, "");
		string texte = "Vos coordonnees sont: {";
		string x = entier_en_chaine(pos.x);
		string y = entier_en_chaine(pos.y);
		texte += x;
		texte += ":" + y + "}";
		draw_simple_text(texte, { 100,10 }, get_font(0));
	}
};

class ell
{
	int a = 100;
	int b = 300;
	V2d_i pos = { X_CONSOLE / 2, Y_CONSOLE / 2 };

};


