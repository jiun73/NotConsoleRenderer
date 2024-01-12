#pragma once

#include "TileRenderer.h"

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
		V2d_i tailleEcran = get_window_size();
		draw_rect(Rect({ pos, xy }));
		nettoyer(pos, xy,velocite);
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

	V2d_i tailleEcran = get_window_size();
	V2d_i pos = { random().range(0,X_CONSOLE),random().range(0,Y_CONSOLE) };
	V2d_i velocite = { choose(),choose() };
	V2d_i xy = { 20,20 };
	void change_velocity(V2d_i pos, V2d_i xy, V2d_i& velocite)
	{
		string son = "../TileRenderer/Sounds/rizz.wav";
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
public:
	V2d_i create()
	{
		pencil(COLOR_GREEN);
		rect(pos, xy, velocite);
		/*string txt = "Vos coordonnées: " + pos.x;
		txt += ", " + pos.y;
		draw_text(txt, X_CONSOLE, { X_CONSOLE / 2 - 100 }, get_font(1));*/
		change_velocity(pos, xy, velocite);
		return pos;
	}
	bool boink(vector<square> vect)
	{
		for (int i = 0; i < vect.size(); i++)
		{
			if (rect(pos,xy,velocite) == vect.at(i).get_rect())
			{
				return true;
			}
		}
		return false;
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

};