#pragma once

#include "NotConsoleRenderer.h"
#include <map>
#include <vector>
#include "Strings.h"
using namespace std;

// taille de la console
const int X_CONSOLE = 900;
const int Y_CONSOLE = 900;
V2d_i window = { X_CONSOLE, Y_CONSOLE };

// Contour du board
const int bcd = 50; // board contour difference: distance en pixels entre la console et et le contour du board
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

class pion
{
private:
public:
	int rayon = xy / 2;
	V2d_i pos;
};
	 

// Chacun des 4 joueurs
class player
{
public:
	string name;
	Color couleur;
};

player rouge;
player bleu;
player jaune;
player vert;

// chaque taille du plancher
class tile
{
private:
	
	int x = (END_X_MAP - BEG_X_MAP) / squaresPerColumn;
	int y = (END_Y_MAP - BEG_Y_MAP) / squaresPerRow;
	V2d_i xy = { x,y };

public:
	V2d_i pos;
	map<bool, player> occupation;
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
		draw_simple_text(entier_en_chaine(numero), {pos.x + 10, pos.y + 10}, get_font(0));
	}
};
