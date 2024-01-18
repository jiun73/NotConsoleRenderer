#pragma once
#include "WeaponIntructions.h"
#include "WeaponInventory.h"

#include <algorithm>
using std::all_of;

struct WeaponParser
{
	WeaponComputer* computer = nullptr;
	WeaponInventory* inventory = nullptr;
	WeaponInventory inventory_copy;
	vector<string> source;

	vector<WeaponError> load(WeaponComputer& computer, WeaponInventory& inventory);

private:
	size_t				get_true_offset(const vector<string>& args, size_t offset);
	WeaponError			load_intruction(const string& line, size_t i);
};