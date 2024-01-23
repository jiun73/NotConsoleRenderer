﻿#pragma once

#include "TileRenderer.h"
#include "Strings.h"
#include <ctime>

const int X_CONSOLE = 1000;
const int Y_CONSOLE = 750;

const double frame_rate = 120.0;

struct chrono 
{
private:
	double time = 0;
	double aug = 1.0 / 120.0;
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
	bool begin = false;
	V2d_i velocite = { choose(),choose() };
	int acceleration = 5;
	bool is_main = false;
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
	V2d_i pos = {X_CONSOLE / 2, Y_CONSOLE / 2};
	V2d_i topright = { pos.x + xy.x,pos.y };
	V2d_i bottomleft = { pos.x, pos.y + xy.y};
	V2d_i bottomright = {pos.x + xy.x, pos.y + xy.y};
	V2d_i centre = { pos.x + (topright.x - pos.x) / 2, pos.y + (bottomright.y - pos.y) / 2 };
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
	}
	void rotate_on_pos(int& angle)
	{
		if (key_held(SDL_SCANCODE_SPACE))
		{
			double angleEnRadians = convertir_en_radians(angle);
			double x = cos(angleEnRadians);
			double y = sin(angleEnRadians);
			/*int x = cosinus * radius + axisPosition.x;
			  int y = sinus * radius + axisPosition.y;*/
			//xcosθ−ysinθ, xsinθ + ycosθ;
			/*pos.x = pos.x * x - pos.y * y;
			pos.y = pos.x * y - pos.y * x;*/
			topright.x = xy.x * cos(convertir_en_radians(angle)) + pos.x;
			topright.y = xy.y * sin(convertir_en_radians(angle)) + pos.y;
			bottomright.x = sqrt(xy.x * xy.x + xy.y * xy.y) * cos(convertir_en_radians(angle - 315)) + pos.x;
			bottomright.y = sqrt(xy.x * xy.x + xy.y * xy.y) * sin(convertir_en_radians(angle - 315)) + pos.y;
			bottomleft.x = xy.x * cos(convertir_en_radians(angle - 270)) + pos.x;
			bottomleft.y = xy.y * sin(convertir_en_radians(angle - 270)) + pos.y;
			angle++;
		}
	}
	void rotate_on_center(int& angle)
	{
		// centre du rectangle = {pos.x + (topright.x - pos.x) / 2, pos.y + (bottomright.y - pos.y) / 2}
		if (key_held(SDL_SCANCODE_Q))
		{
			pos.x = sqrt(pow((xy.x / 2),2) + pow((xy.y / 2),2)) * cos(convertir_en_radians(angle - 135)) + centre.x;
			pos.y = sqrt(pow((xy.x / 2), 2) + pow((xy.y / 2), 2)) * sin(convertir_en_radians(angle - 135)) + centre.y;
			topright.x = sqrt(pow((xy.x / 2), 2) + pow((xy.y / 2), 2)) * cos(convertir_en_radians(angle - 45)) + centre.x;
			topright.y = sqrt(pow((xy.x / 2), 2) + pow((xy.y / 2), 2)) * sin(convertir_en_radians(angle - 45)) + centre.y;
			bottomright.x = sqrt(pow((xy.x / 2), 2) + pow((xy.y / 2), 2)) * cos(convertir_en_radians(angle - 315)) + centre.x;
			bottomright.y = sqrt(pow((xy.x / 2), 2) + pow((xy.y / 2), 2)) * sin(convertir_en_radians(angle - 315)) + centre.y;
			bottomleft.x = sqrt(pow((xy.x / 2), 2) + pow((xy.y / 2), 2)) * cos(convertir_en_radians(angle - 225)) + centre.x;
			bottomleft.y = sqrt(pow((xy.x / 2), 2) + pow((xy.y / 2), 2)) * sin(convertir_en_radians(angle - 225)) + centre.y;
			angle++;
		}
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
