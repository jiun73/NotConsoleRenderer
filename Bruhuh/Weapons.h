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

struct WeaponSettings 
{
	int maxTemp = 10;
	double cooling = 1.4;
	double heating = 0.7;
	int storage = 10; 
	uint32_t bootSpeed = 100;
	uint32_t clockSpeed = 100;
};

struct WeaponComputer
{
	vector<WeaponInstruction> instructions;
	vector<WeaponRegister> registers;
	vector<string>* source = nullptr;
	WeaponSettings settings;
	WeaponBullet bulletType;

	double temperature = 0;
	bool booting = false;
	bool overheat = false;

	uint32_t clockLast = 0;
	uint32_t delay = 0;

	size_t programCounter = 0;
	size_t emptyCnt = 0;
	size_t skipCnt = 0;

	bool skipCounterIncr = false;

	size_t entity_id = size_t() - 1;

	void shoot() { bulletType.create(entity_id); }
	bool next_instr();
	void set_register_values();
	void skip();
	void cycle();
	void tick(size_t entityid);
	void end();
	void jumpto(size_t dest) { programCounter = dest; skipCounterIncr = true; }
};