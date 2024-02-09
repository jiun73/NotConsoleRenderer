#pragma once

#include "TileRenderer.h"

const int X_CONSOLE = 700;
const int Y_CONSOLE = 700;

const int BEG_X_MAP = 100;
const int BEG_Y_MAP = 100;
const int END_X_MAP = 600;
const int END_Y_MAP = 600;

const int squaresPerRow = 6;
const int squaresPerColumn = 7;

const int xy = (END_X_MAP - BEG_X_MAP) / squaresPerColumn;
const int yx = (END_Y_MAP - BEG_Y_MAP) / squaresPerRow;

const int connexionsAFaire = 4;

const int side_difference = 1;
const int up_difference = squaresPerColumn;
const int up_diag_difference = squaresPerColumn - 1;
const int down_diag_difference = squaresPerColumn + 1;

struct player
{
public:
	bool has_turn = false;
	vector<int> squaresPossessed;
	string name;
};
