#pragma once

#include "TileRenderer.h"
#include "Strings.h"

const int X_CONSOLE = 1000;
const int Y_CONSOLE = 750;

const V2d_i window = { X_CONSOLE, Y_CONSOLE };

double convertir_en_radians(int angle)
{
	double newAngle = static_cast<double>(angle);
	double angleInRad = newAngle * 3.14159265 / 180.0000000;
	return angleInRad;
}

//void draw_full_circle(V2d_i pos, int radius)
//{
//	draw_circle(pos, radius);
//	for (int i = pos.x - radius; i <= pos.x + radius; i++)
//	{
//		for (int n = pos.y - radius; n <= pos.y + radius; n++)
//		{
//			if (n * n + i * i  (pos.x + radius) * (pos.x + radius))
//			{
//				draw_pix({ i,n });
//			}
//		}
//	}
//}

class player
{
private:
	V2d_i xy = { 25,80 };
	V2d_i pos = { X_CONSOLE / 2 - xy.x / 2, Y_CONSOLE - xy.y };
	Color couleur = COLOR_GREEN;
	int rayon = 15;
	V2d_i centerLeftCircle;
	V2d_i centerRightCircle;
	V2d_i posRectangle;

public:
	void show()
	{
		pencil(couleur);
		draw_full_rect(Rect({ pos,xy })); // rectangle
		draw_circle({pos.x - rayon, pos.y + xy.y - rayon}, rayon); // left circle
		draw_circle({pos.x + xy.x + rayon, pos.y + xy.y - rayon}, rayon); // right circle
	}
	void move()
	{
		if (key_held(SDL_SCANCODE_D) && pos.x + xy.x + rayon * 2 < X_CONSOLE)
		{
			pos.x++;
		}
		if (key_held(SDL_SCANCODE_A) && pos.x - rayon * 2 > 0)
		{
			pos.x--;
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

class bullets
{
private:
	V2d_i pos = {50,50};
	int longueur = 15;
	V2d_i xy = {3, longueur};
	bool firstShot = true;
	vector<V2d_i> points;

	void show()
	{
		pencil(COLOR_GREEN);
		draw_full_rect(Rect({ pos, xy }));
	}
public:
	void shoot(V2d_i posInitiale)
	{
		if (firstShot)
		{
			pos = posInitiale;
			firstShot = false;
		}
		pencil(rgb(255, 0, 0));
		draw_full_rect(Rect({ pos,xy }));
		pos.y--;
	}
	V2d_i get_xy()
	{
		return xy;
	}
	bool is_on_screen()
	{
		if (pos.y > 0 && pos.y < Y_CONSOLE)
		{
			return true;
		}
		return false;
	}
	vector<V2d_i> get_points()
	{
		for (int i = pos.x; i <= pos.x + xy.x; i++)
		{
			for (int n = pos.y; n <= pos.y + xy.y; n++)
			{
				points.push_back({i,n});
			}
		}
		return points;
	}
	V2d_i get_pos()
	{
		return pos;
	}
};


class balls
{
private:
	V2d_i pos = { 500,500 };
	V2d_i velocite = { 0,0 };
	vector<V2d_i> points;
	int rayon = 25;
	int acceleration = 2;
	int numero = rayon;

	void destroy()
	{
		pos = { 100000000,100000000 };
	}

public:
	V2d_i get_pos()
	{
		return pos;
	}
	void show()
	{
		pencil(rgb(255, 0, 255));
		draw_circle(pos, rayon);
	}
	void show_number()
	{
		pencil(COLOR_WHITE);
		string x = entier_en_chaine(numero);
		draw_simple_text(x, pos, get_font(0));
	}
	void move()
	{
		velocite = 0;
		if (key_held(SDL_SCANCODE_A) && pos.x - rayon > 0)
		{
			velocite.x = -1;
		}
		if (key_held(SDL_SCANCODE_D) && pos.x + rayon < X_CONSOLE)
		{
			velocite.x = 1;
		}
		if (key_held(SDL_SCANCODE_W) && pos.y - rayon > 0)
		{
			velocite.y = -1;
		}
		if (key_held(SDL_SCANCODE_S) && pos.y + rayon < Y_CONSOLE)
		{
			velocite.y = 1;
		}
		pos.x += velocite.x * acceleration;
		pos.y += velocite.y * acceleration;
	}
	vector<V2d_i> get_points()
	{
		for (int i = 0; i < 360; i++)
		{
			int x = rayon * cos(convertir_en_radians(i)) + pos.x;
			int y = rayon * sin(convertir_en_radians(i)) + pos.y;
			points.push_back({ x,y });
		}
		return points;
	}
	void boom(vector<bullets>& balles)
	{
		for (int i = 0; i < balles.size(); i++)
		{
			int x = balles.at(i).get_pos().x;
			int y = balles.at(i).get_pos().y;
			if ( pow(x - pos.x,2) + pow(y - pos.y,2) <= rayon * rayon)
			{
				balles.erase(balles.begin() + i);
				numero--;
				if (numero == 0)
				{
					sound().playSound("boom.wav");
					destroy();
				}
				else
				{
					sound().playSound("bop.wav");
				}
				break;
			}
		}
	}
};