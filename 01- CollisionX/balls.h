#pragma once

#include "TileRenderer.h"
#include "Strings.h"

const int X_CONSOLE = 1000;
const int Y_CONSOLE = 750;


struct square
{
private:
	Rect rect(V2d_i& pos, V2d_i xy,V2d_i velocite)
	{
		pencil(COLOR_GREEN);
		if (is_main)
		{
			pencil(COLOR_WHITE);
			if (!begin)
			{
				pos = { 30,30 };
				begin = true;
			}
		}
		draw_rect(Rect({ pos, xy }));
		pos += velocite;
		return {pos,xy};
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

	void change_velocity(V2d_i pos, V2d_i xy, V2d_i& velocite)
	{
		string son = "doing.wav"; //../TileRenderer/Sounds/rizz.wav
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
	bool in_perimeter(square carre)
	{
		int perimetre = 5;
		if (pos.x + xy.x + perimetre >= carre.get_pos().x && abs(pos.y - carre.get_pos().y) < xy.x * 2 && pos.x < carre.get_pos().x ||
			pos.x <= carre.get_pos().x + carre.get_xy().x + perimetre && abs(pos.y - carre.get_pos().y) < xy.x * 2 && pos.x > carre.get_pos().x ||
			pos.y + xy.y + perimetre >= carre.get_pos().y && abs(pos.x - carre.get_pos().x) < xy.x * 2 && pos.y < carre.get_pos().y ||
			pos.y <= carre.get_pos().y + carre.get_xy().y + perimetre && abs(pos.x - carre.get_pos().x) < xy.x * 2 && pos.y > carre.get_pos().y  )
		{
			return true;
		}
		return false;
	}
	int line_touch(vector<V2d_i> line1, vector<V2d_i> line2)
	{
		vector<V2d_i> points;
		for (int n = 0; n < line1.size(); n++)
		{
			for (int i = 0; i < line2.size(); i++)
			{
				if (line1.at(n).x == line2.at(i).x)
				{
					points.push_back(line1.at(n));
				}
			}
		}
		return points.size();
	}
	string touched(square carre)
	{
		int nombre = line_touch(get_top_coordinates(), carre.get_top_coordinates());
		if (nombre > 3)
		{
			return "horizontal";
		}
		else
		{
			return "vertical";
		}
		/*if (line_touch(get_top_coordinates(), carre.get_bottom_coordinates()) || line_touch(get_bottom_coordinates(), carre.get_top_coordinates()))
		{
			return "horizontal";
		}
		else if (line_touch(get_left_coordinates(), carre.get_right_coordinates()) || line_touch(get_right_coordinates(), carre.get_left_coordinates()))
		{
			return "vertical";
		}*/
	}
	vector<V2d_i> get_bottom_coordinates()
	{
		vector<V2d_i> points;
		for (int n = pos.x; n <= pos.x + xy.x; n++)
		{
			points.push_back({ n,pos.y + xy.y });
		}
		return points;
	}
	vector<V2d_i> get_left_coordinates()
	{
		vector<V2d_i> points;
		for (int n = pos.y; n <= pos.y + xy.y; n++)
		{
			points.push_back({ n,pos.x });
		}
		return points;
	}
	vector<V2d_i> get_right_coordinates()
	{
		vector<V2d_i> points;
		for (int n = pos.y; n <= pos.y + xy.y; n++)
		{
			points.push_back({ n,pos.x + xy.x });
		}
		return points;
	}
	vector<V2d_i> get_top_coordinates()
	{
		vector<V2d_i> points;
		for (int n = pos.x; n <= pos.x + xy.x; n++)
		{
			points.push_back({ n,pos.y });
		}
		return points;
	}
	vector<V2d_i> get_points()
	{
		vector<V2d_i> points;
		for (int n = pos.x; n <= pos.x + xy.x; n++)
		{
			for (int i = pos.y; i <= pos.y + xy.y; i++)
			{
				points.push_back({ n,i });
			}
		}
		return points;
	}
	void move_main(V2d_i& pos, V2d_i xy, V2d_i velocite)
	{
		velocite = 0;
		if (key_held(SDL_SCANCODE_A) && pos.x > 0)
		{
			velocite.x = -1;
		}
		if (key_held(SDL_SCANCODE_D) && pos.x + xy.x < X_CONSOLE)
		{
			velocite.x = 1;
		}
		if (key_held(SDL_SCANCODE_W) && pos.y > 0)
		{
			velocite.y = -1 ;
		}
		if (key_held(SDL_SCANCODE_S) && pos.y + xy.y < Y_CONSOLE)
		{
			velocite.y = 1 ;
		}
		pos.x += velocite.x * acceleration;
		pos.y += velocite.y * acceleration;
	}
public:
	bool begin = false;
	V2d_i velocite = { choose(),choose() };
	bool is_main = false;
	int acceleration = 2;
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
	void boink(vector<square>& vect)
	{
		for (int i = 0; i < vect.size(); i++)
		{
			if (pos == vect.at(i).get_pos())
			{
				continue;
			}
			if (in_perimeter(vect.at(i)))
			{
				square carre = vect.at(i);
				if (pos.x < carre.get_pos().x + carre.get_xy().x &&
					pos.x + xy.x > carre.get_pos().x &&
					pos.y < carre.get_pos().y + carre.get_xy().y &&
					pos.y + xy.y > carre.get_pos().y)
				{
					if (is_main)
					{
						vect.clear();
						pencil(COLOR_BLACK);
						draw_clear();
						main_touched = true;
						sound().stopMusic();
						sound().playSound("lost.wav");
						break;
					}
					string zone = touched(vect.at(i));
					if (zone == "horizontal")
					{
						velocite.y *= -1;
					}
					if (zone == "vertical")
					{
						velocite.x *= -1;
					}
				}
			}
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
		draw_simple_text(texte, { 10,10 }, get_font(0));
	}
	V2d_i get_pos()
	{
		return pos;
	}
	V2d_i get_xy()
	{
		return xy;
	}
};



