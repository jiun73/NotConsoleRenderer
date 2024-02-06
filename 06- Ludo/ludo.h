#pragma once

#include "TileRenderer.h"
#include <map>
#include <vector>
using namespace std;

// taille de la console
const int X_CONSOLE = 1000;
const int Y_CONSOLE = 900;
V2d_i window = { X_CONSOLE, Y_CONSOLE };

// Contour du board
const int bcd = 50; // board contour difference: distance en pixels entre la console et et le contour du board
const Rect board_contour = { {X_CONSOLE + bcd, Y_CONSOLE - bcd},{X_CONSOLE - bcd * 2, Y_CONSOLE - bcd * 2} };

// Contour du gameplay
const int gcd = 50; // gameplay contour difference: distance en pixels entre la console et et le contour du board
const Rect gameplay_contour = { {X_CONSOLE + gcd, Y_CONSOLE - gcd},{X_CONSOLE - gcd * 2, Y_CONSOLE - gcd * 2} };


// Chacun des 4 joueurs
class player
{
public:
	string name;
	Color couleur;
	//vector<tile> tilesOccupied;
};

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
