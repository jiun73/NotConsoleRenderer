#pragma once
#include "WeaponIntructions.h"

struct IntDefaultZero 
{
	int val = 0;
	operator int() { return val; }
};

struct WeaponInventory 
{
	map<ComputerInstructionSet, IntDefaultZero> available_intructions;
	map<int, IntDefaultZero> available_constants;
	bool infinite = false;
};