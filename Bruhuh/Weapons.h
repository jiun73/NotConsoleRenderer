#pragma once
#include "SDL.h"
#include <vector>
#include <map>
#include <array>
#include <string>
#include "StringManip.h"
#include "WeaponErrors.h"
#include "WeaponHelpers.h"

using std::vector;
using std::pair;
using std::string;
using std::array;

struct WeaponComputer
{
	vector<WeaponRegister> registers;
	vector<string>* source;

	int temperature = 0;
	int maxTemp = 0;
	
	int storage = 0;

	WeaponBullet bulletType;

	uint32_t clockSpeed = 100;
	uint32_t clockLast = 0;

	size_t programCounter = 0;
	size_t emptyCnt = 0;
	size_t skipCnt = 0;
	vector<WeaponInstruction> instructions; 

	bool skipCounterIncr = false;

	void shoot() { bulletType.bulletCreationFunction(); }
	bool next_instr();
	void set_register_values();
	void skip();
	void cycle();
	void tick();
	void jumpto(size_t dest) { programCounter = dest; skipCounterIncr = true; }
};