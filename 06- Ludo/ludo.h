#pragma once

#include "NotConsoleRenderer.h"
#include <map>
#include <vector>
using namespace std;

const int X_CONSOLE = 1000;
const int Y_CONSOLE = 750;
V2d_i window = { X_CONSOLE, Y_CONSOLE };

// Chacun des 4 joueurs
class player
{
public:
	string name;
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

	void show(Color couleur = COLOR_WHITE, bool is_full = true)
	{
		if (!is_full)
		{
			pencil(couleur);
			draw_rect(Rect({ pos, xy }));
		}
	}
};
