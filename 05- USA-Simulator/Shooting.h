#pragma once

#include "TileRenderer.h"
#include "Strings.h"

const int X_CONSOLE = 1000;
const int Y_CONSOLE = 750;

const V2d_i window = { X_CONSOLE, Y_CONSOLE };

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
	V2d_i pos;
	V2d_i posI;
	int longueur = 7;
	V2d_i xy = {2, longueur};

	void show()
	{
		pencil(COLOR_GREEN);
		draw_full_rect(Rect({ pos, xy }));
	}
public:
	void shoot(V2d_i posInitiale)
	{
		pos = posInitiale;
	}
};