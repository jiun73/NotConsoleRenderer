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

vector<int> chemin = { 6,7,8,23,38,53,68, 83,99,100,101,102,103,104,119,134,133,132,131,130,129,143,158,173,188,203,218,217,216,
		   201,186,171,156,141,125,124,123,122,121,120,105,90,91,92,93,94,95,81,66,51,36,21 };
 

int at(int index)
{
	int a =  index % chemin.size();
	return chemin.at(a);
}

template<typename T>
T index_of(T el, vector<T> vect)
{
	for (int i = 0; i < vect.size(); i++)
	{
		if (el == vect.at(i))
		{
			return i;
		}
	}
}


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
		return -1;
	}
};

vector<tile> carreaux;
vector<tile> get_tiles()
{
	vector<tile> tiles;
	for (int i = 0; i < 15 * 15; i++)
	{
		tiles.push_back(tile());
	}
	int index = 0;
	for (int i = BEG_Y_MAP; i <= END_Y_MAP - yx; i += yx)
	{
		for (int n = BEG_X_MAP; n <= END_X_MAP - xy; n += xy)
		{
			tiles.at(index).pos = { n,i };
			tiles.at(index).numero = index;
			index++;
		}
	}
	return tiles;
}

struct de
{
	static int shuffle()
	{
		return Random().frange(1, 7);
	}
};


class pion
{
private:
	int index_of()
	{
		for (int i = 0; i < chemin.size(); i++)
		{
			if (caseActuelle == at(i)) 
			{
				return i;
			}
		}
		return 0;
	}
public:
	int rayon = xy / 2 - 5;
	int numero;
	int spawn;
	int caseActuelle; // index du pion dans carreaux
	bool outOfHome = false;

	pion(int n, int s)
	{
		numero = n;
		spawn = s;
	}

	void show_number()
	{
		pencil(COLOR_WHITE);
		//draw_simple_text(entier_en_chaine(numero), carreaux.at(caseActuelle % 225).pos + xy / 2 - 5 , get_font(0));
	}

	void display()
	{
		if (!outOfHome)
		{
			Color col = get_pencil();
			draw_full_circle(carreaux.at(caseActuelle).pos + xy / 2, rayon);
			pencil(COLOR_BLACK);
			draw_circle(carreaux.at(caseActuelle).pos + xy / 2, rayon);
			pencil(col);
		}
		else
		{
			Color col = get_pencil();
			draw_full_circle(carreaux.at(caseActuelle % 225).pos + xy / 2, rayon);
			pencil(COLOR_BLACK);
			draw_circle(carreaux.at(caseActuelle % 225).pos + xy / 2, rayon);
			pencil(col);
		}
		//show_number();
	}

	void move(int num)
	{
		if (outOfHome)
		{
			caseActuelle += num;
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
	bool is_playing = false;
	int pionsEnMaison = 4;
	bool pionJoue = false;

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

	void display_tokens()
	{
		pencil(couleur);
		token1->display();
		token2->display();
		token3->display();
		token4->display();
	}

	// si le joueur a 3 pions en maison, cette fonction retourne le pion qui est sorti
	pion& pion_sorti()
	{
		if (token1->outOfHome)
		{
			return *token1;
		}
		if (token2->outOfHome)
		{
			return *token2;
		}
		if (token3->outOfHome)
		{
			return *token3;
		}
		if (token4->outOfHome)
		{
			return *token4;
		}
	}
};


player* rouge = new player(rgb(255, 0, 0), "rouge", 16, 19, 61, 64, 91);
player* bleu = new player(rgb(0, 0, 255), "bleu", 151, 154, 196, 199, 201);
player* jaune = new player(rgb(255, 255, 0), "jaune", 25, 28, 70, 73, 23);
player* vert = new player(rgb(0, 255, 0), "vert", 160, 163, 205, 208, 133);
player* mirane = new player(rgb(255, 192, 203), "mirane", 1, 1, 1, 1, 1);

player& actual()
{
	if (rouge->is_playing)
	{
		return *rouge;
	}
	if (bleu->is_playing)
	{
		return *bleu;
	}
	if (vert->is_playing)
	{
		return *vert;
	}
	if (jaune->is_playing)
	{
		return *jaune;
	}
}

struct environment
{
	//bool pionMovement = true;

	bool de_obtenu = false;
};