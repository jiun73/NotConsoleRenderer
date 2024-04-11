#pragma once

#include "NotConsoleRenderer.h"

const int X_CONSOLE = 600;
const int Y_CONSOLE = 600;

const int BEG_X_MAP = 100;
const int BEG_Y_MAP = 100;
const int END_X_MAP = 500;
const int END_Y_MAP = 500;

struct player
{
public:
	bool has_turn;
	vector<int> squaresPossessed;
	string name;
};
