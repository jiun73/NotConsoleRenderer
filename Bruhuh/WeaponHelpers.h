#pragma once
#include <functional>
#include <array>

using std::function;
using std::array;

enum ComputerInstructionSet
{
	LOAD, JUMP, SHOOT, WAIT,
	JEZ, JNZ, ADD, SUB, GOTO,
	SWAP, VALUE, WRITE,
	READ, HALT, INVALID
};

struct WeaponBullet
{
	function<void()> bulletCreationFunction;

	int damage = 0;
	int speed = 0;
	int size = 0;
};

struct WeaponRegister
{
	bool _signed_ = true;
	bool readonly = false;
	int bit = 5;
	int value;
};

struct WeaponComputer;

struct WeaponInstruction
{
	function<void(WeaponComputer&, WeaponInstruction&)> instructionFunction;

	array<int, 3> inputs = { -1,-1,-1 };
	array<int, 3> values = { -1,-1,-1 };
	array<bool, 3> isRegisterValue = { false, false, false };

	bool data = false;
	bool empty = true;

	int& operator[](size_t i) { return values.at(i - 1); }
};