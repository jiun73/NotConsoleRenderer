#pragma once
#include <functional>
#include <array>
#include <map>

using std::function;
using std::array;
using std::map;

enum ComputerInstructionSet
{
	LOAD, JUMP, SHOOT, WAIT,
	JEZ, JNZ, ADD, SUB, GOTO,
	SWAP, VALUE, WRITE,
	READ, HALT, INVALID
};

struct WeaponRegister
{
	bool _signed_ = true;
	bool readonly = false;
	int bit = 5;
	int value = 0;

	void clamp_value()
	{
		int bits = bit - _signed_;

		int max = 0;
		for (size_t i = 0; i < bits; i++) max |= 1 << i;

		if (!_signed_)
		{
			if (value < 0) value = max - (-value % (max + 1));
			if (value > max) value = value % (max + 1);
		}
		else
		{
			if (value > max) value = -max + (value % (max + 1));
			if (value < -max) value = max - (-value % (max + 1));
		}
	}
};

struct WeaponBullet
{
	map<string, WeaponRegister> registers;

	function<void(WeaponBullet&, size_t)> bulletCreationFunction;

	void create(size_t entityid) { bulletCreationFunction(*this, entityid); }
};

struct WeaponComputer;

struct WeaponInstruction
{
	function<void(WeaponComputer&, WeaponInstruction&)> instructionFunction;

	array<int, 3> inputs = { -1,-1,-1 };
	array<int, 3> values = { -1,-1,-1 };
	array<bool, 3> isRegisterValue = { false, false, false };
	array<string, 3> specialInputs = { "", "", ""};

	bool data = false;
	bool empty = true;

	int& operator[](size_t i) { return values.at(i - 1); }
	string& special(size_t i) { return specialInputs.at(i - 1); }
};