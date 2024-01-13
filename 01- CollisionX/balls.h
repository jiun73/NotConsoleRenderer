#pragma once

#include "TileRenderer.h"
#include <cmath>

const int X_CONSOLE = 1000;
const int Y_CONSOLE = 750;

struct circle
{
	V2d_i create(V2d_i pos, int radius)
	{
		draw_circle(pos, radius);
		return pos;
	}
	void move(V2d_i& cercle, V2d_i velocite)
	{
		cercle += velocite;
	}
	V2d_i get_pos(V2d_i cercle)
	{
		return cercle;
	}
};


struct square
{
private:
	Rect rect(V2d_i& pos, V2d_i xy,V2d_i velocite)
	{
		pencil(COLOR_GREEN);
		draw_rect(Rect({ pos, xy }));
		//nettoyer(pos, xy,velocite);
		pencil(COLOR_GREEN);
		pos += velocite;
		return {pos,xy};
	}

	void nettoyer(V2d_i pos, V2d_i xy,V2d_i velocite)
	{
		pencil(COLOR_BLACK);
		draw_rect({ pos - velocite,xy });
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

	vector<V2d_i> get_perimeter(int spacing)
	{
		V2d_i sop = pos - spacing;
		V2d_i yx = xy + spacing;
	}

	V2d_i tailleEcran = get_window_size();
	V2d_i pos = { random().range(0,X_CONSOLE),random().range(0,Y_CONSOLE) };
	
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
		else if (pos.x + xy.x == X_CONSOLE)// get_window_size().x)
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
		int nombre = line_touch(get_top_coordinates(), carre.get_bottom_coordinates());
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
public:
	V2d_i velocite = { choose(),choose() };
	V2d_i create()
	{
		pencil(COLOR_GREEN);
		rect(pos, xy, velocite);
		change_velocity(pos, xy, velocite);
		return pos;
	}
	
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
					pos.y + xy.y > carre.get_pos().y) //(touched(vect.at(i), zone)) 
				{
					string zone = touched(vect.at(i));
					if (zone == "horizontal")
					{
						velocite.y *= -1;
						//vect.at(i).velocite.y *= -1;
					}
					if (zone == "vertical")
					{
						velocite.x *= -1;
						//vect.at(i).velocite.x *= -1;
					}
				}
			}
		}
	}
	V2d_i get_pos()
	{
		return pos;
	}
	V2d_i get_xy()
	{
		return xy;
	}
	V2d_i get_velo()
	{
		return velocite;
	}
	Rect get_rect()
	{
		return rect(pos, xy, velocite);
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
};



