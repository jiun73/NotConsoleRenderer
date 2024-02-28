#pragma once

#include "NotConsoleRenderer.h"
#include <map>
#include <vector>
#include "Strings.h"
using namespace std;

// taille de la console
const int X_CONSOLE = 800;
const int Y_CONSOLE = 800;
V2d_i window = { X_CONSOLE, Y_CONSOLE };

// Contour du board
const int bcd = 100; // board contour difference: distance en pixels entre la console et et le contour du board
const Rect board_contour = { {bcd, bcd},{X_CONSOLE - bcd * 2, Y_CONSOLE - bcd * 2} };


// Dessin de lignes
const int BEG_X_MAP = bcd;
const int BEG_Y_MAP = bcd;
const int END_X_MAP = X_CONSOLE - bcd;
const int END_Y_MAP = Y_CONSOLE - bcd;

const int squaresPerRow = 15;
const int squaresPerColumn = 15;

const int xy = (END_X_MAP - BEG_X_MAP) / squaresPerColumn;
const int yx = (END_Y_MAP - BEG_Y_MAP) / squaresPerRow;
 

void draw_full_circle(V2d_i pos, int rayon)
{
	for (int i = pos.x - rayon; i <= pos.x + rayon; i++)
	{
		for (int n = pos.y - rayon; n <= pos.y + rayon; n++)
		{
			if (pow(i - pos.x, 2) + pow(n - pos.y, 2) < pow(rayon, 2))
			{
				draw_pix({ i,n });
			}
		}
	}
}

struct de
{
	static int shuffle()
	{
		return Random().frange(1, 7);
	}
};


class tile
{
private:

	int x = (END_X_MAP - BEG_X_MAP) / squaresPerColumn;
	int y = (END_Y_MAP - BEG_Y_MAP) / squaresPerRow;
	V2d_i xy = { x,y };

public:
	V2d_i pos;
	static vector<tile> tiles;
	int numero;

	void light()
	{
		draw_full_rect({ pos,xy });
		Color actual = get_pencil();
		pencil(COLOR_BLACK);
		draw_rect({ pos,xy });
		pencil(actual);
	}

	void show_num()
	{
		pencil(COLOR_CYAN);
		draw_simple_text(entier_en_chaine(numero), { pos.x + 10, pos.y + 10 }, get_font(0));
	}

	int index_of(vector<tile> chemin)
	{
		for (int i = 0; i < chemin.size(); i++)
		{
			if (chemin.at(i).pos == pos)
			{
				return i;
			}
		}
	}
};

class pion
{
private:
public:
	int rayon = xy / 2 - 5;
	int numero;
	int spawn;
	int caseActuelle;
	bool can_be_played = true;
	V2d_i pos;

	pion(int n, int s)
	{
		numero = n;
		spawn = s;
	}

	void show_number()
	{
		pencil(COLOR_WHITE);
		draw_simple_text(entier_en_chaine(numero), pos + xy / 2 - 5 , get_font(0));
	}

	void display(vector<tile> carreaux)
	{
		pos = carreaux.at(caseActuelle).pos;
		draw_full_circle(pos + xy / 2, rayon);
		show_number();
	}

	void move(vector<tile> carreaux, int num)
	{
		caseActuelle += num;
		for (int i = 0; i < carreaux.size(); i++)
		{
			de::shuffle();
		}
	}
};


class player
{
private:
	int n1;
	int n2;
	int n3;
	int n4;

public:
	string name;
	Color couleur;
	int spawnTile;

	pion* token1 = new pion(1, 6);
	pion* token2 = new pion(2, 6);
	pion* token3 = new pion(3, 6);
	pion* token4 = new pion(4, 6);

	void init_tokens()
	{
		token1->spawn = n1;
		token1->caseActuelle = token1->spawn;
		token2->spawn = n2;
		token2->caseActuelle = token2->spawn;
		token3->spawn = n3;
		token3->caseActuelle = token3->spawn;
		token4->spawn = n4;
		token4->caseActuelle = token4->spawn;
	}

	player(Color coul, string n, int s1, int s2, int s3, int s4, int st)
	{
		couleur = coul;
		name = n;
		n1 = s1;
		n2 = s2;
		n3 = s3;
		n4 = s4;
		spawnTile = st;
		init_tokens();
	}

	void display_tokens(vector<tile> carreaux)
	{
		pencil(rgb(51,62,32));
		token1->display(carreaux);
		pencil(rgb(51, 62, 32));
		token2->display(carreaux);
		pencil(rgb(51, 62, 32));
		token3->display(carreaux);
		pencil(rgb(51, 62, 32));
		token4->display(carreaux);
	}
};


player* rouge = new player(rgb(255, 0, 0), "rouge", 16, 19, 61, 64, 91);
player* bleu = new player(rgb(0, 0, 255), "bleu", 151, 154, 196, 199, 201);
player* jaune = new player(rgb(255, 255, 0), "jaune", 25, 28, 70, 73, 23);
player* vert = new player(rgb(0, 255, 0), "vert", 160, 163, 205, 208, 133);
