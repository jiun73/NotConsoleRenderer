#pragma once
#include "WeaponIntructions.h"

#include <algorithm>
using std::all_of;

struct WeaponParser
{
	WeaponComputer* computer = nullptr;
	vector<string> source;

	vector<WeaponError> load(WeaponComputer& computer);

private:
	size_t				get_true_offset(const vector<string>& args, size_t offset);
	WeaponError			load_intruction(const string& line, size_t i);
};