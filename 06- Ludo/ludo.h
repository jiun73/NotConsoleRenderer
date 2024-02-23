#pragma once

#include "NotConsoleRenderer.h"
#include <map>
#include <vector>
using namespace std;

// taille de la console
const int X_CONSOLE = 1000;
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


// BOARD COLORING
	// bleu
	Rect le_bleu = { {BEG_X_MAP, BEG_Y_MAP},{ (END_X_MAP - BEG_X_MAP) * 6 / 15,(END_Y_MAP - BEG_Y_MAP) * 6 / 15 } };
	Rect le_rouge = { {(END_X_MAP - BEG_X_MAP) * 9 / 15, BEG_Y_MAP},{(END_X_MAP - BEG_X_MAP) * 6 / 15,(END_Y_MAP - BEG_Y_MAP) * 6 / 15} };
	Rect le_vert = { {(END_X_MAP - BEG_X_MAP) * 9 / 15, (END_Y_MAP - BEG_Y_MAP) * 9 / 15},{ (END_X_MAP - BEG_X_MAP) * 6 / 15,(END_Y_MAP - BEG_Y_MAP) * 6 / 15 } };
	Rect le_jaune = { {BEG_X_MAP, (END_Y_MAP - BEG_Y_MAP) * 9 / 15},{ (END_X_MAP - BEG_X_MAP) * 6 / 15,(END_Y_MAP - BEG_Y_MAP) * 6 / 15 } };

// Chacun des 4 joueurs
class player
{
public:
	string name;
	Color couleur;
	//vector<tile> tilesOccupied;
};

player rouge;
player bleu;
player jaune;
player vert;

// chaque taille du plancher
class tile
{
private:
	V2d_i pos;
	int x;
	int y;
	V2d_i xy = { x,y };

public:
	map<bool, player> occupation;
	static vector<tile> tiles;
	void show(Color couleur = COLOR_WHITE, bool is_full = true)
	{
		if (!is_full)
		{
			pencil(couleur);
			draw_rect(Rect({ pos, xy }));
		}
	}
};
