#pragma once
#include "SDL.h"
#include <vector>
#include <map>
#include <functional>
#include <array>
#include <string>
#include "StringManip.h"

using std::vector;
using std::function;
using std::pair;
using std::string;
using std::array;

enum ComputerInstructionSet
{
	LOAD,
	JUMP,
	SHOOT,
	WAIT,
	IF,
	ADD,
	SUB,
	GOTO,
	SWAP,
	VALUE,
	WRITE,
	READ,
	HALT,
	INVALID
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
	int bit = 0;
	int value;
};

class WeaponComputer;

struct WeaponInstruction
{
	function<void(WeaponComputer&, WeaponInstruction&)> instructionFunction;

	array<int, 3> inputs;
	array<int, 3> values;
	array<bool, 3> isReg;

	bool data = false;
	bool empty = true;

	int& operator[](size_t i)
	{
		return values.at(i - 1);
	}
};

enum ComputerErrorTypes 
{
	ERROR_NONE,
	ERROR_NOT_A_REGISTER,
	ERROR_NOT_IN_INVENTORY,
	ERROR_INVALID_ARGUMENT,
	ERROR_TOO_MANY_ARGS,
	ERROR_NOT_ENOUGH_ARGS,
	ERROR_INVALID_INSTRUCTION
};

struct WeaponError 
{
	ComputerErrorTypes type = ERROR_NONE;
	int line = 0;
	int offset = 0;
};

struct WeaponComputer
{
	vector<WeaponRegister> registers;

	int temperature = 0;
	int maxTemp = 0;
	
	int storage = 0;

	WeaponBullet bulletType;

	uint32_t clockSpeed = 0;
	uint32_t clockLast = 0;

	size_t programCounter = 0;
	vector<WeaponInstruction> instructions; 

	void shoot() { bulletType.bulletCreationFunction(); }

	void cycle()
	{
		WeaponInstruction& instruction = instructions.at(programCounter);
		instruction.instructionFunction(*this, instruction);
		programCounter++;
	}

	void tick()
	{
		uint32_t timeSinceLast = SDL_GetTicks() - clockLast;

		while (timeSinceLast > clockSpeed)
		{
			cycle();
			timeSinceLast -= clockSpeed;
		}

		clockLast = SDL_GetTicks() + timeSinceLast;
	}
};

pair<function<void(WeaponComputer&, WeaponInstruction&)>, int> get_instruction(ComputerInstructionSet type)
{
	switch (type)
	{
	case LOAD:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.registers.at(data[2]).value = data[1];
			}, 2};
	case JUMP:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.programCounter += data[1];
			}, 1 };
	case SHOOT:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.shoot();
			},0 };
	case WAIT:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.clockLast += data[1];
			}, 1};
	case IF:
		break;
	case ADD:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.registers.at(data[3]).value = data[1] + data[2];
			}, 3 };
	case SUB:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.registers.at(data[3]).value = data[1] - data[2];
			}, 3};
	case GOTO:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.programCounter = data[1];
			},3 };
	case SWAP:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				int buffer = computer.registers.at(data[1]).value;
				computer.registers.at(data[1]).value = computer.registers.at(data[2]).value;
				computer.registers.at(data[2]).value = buffer;
			}, 2 };
	case VALUE:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.cycle();
			}, 0 };
	case WRITE:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{

				WeaponInstruction val;
				val.instructionFunction = get_instruction(VALUE).first;
				val.inputs.at(0) = data[1];
				val[1] = data[1];

				computer.instructions.at(data[2]) = val;
			}, 2 };
	case READ:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				WeaponInstruction& instr = computer.instructions.at(data[1]);
				if (instr.data)
				{
					computer.registers.at(data[2]).value = instr[1];
				}
			}, 2 };
	case HALT:
		break;
	default:
		break;
	}
}

ComputerInstructionSet get_instruction_type(string type)
{
	if (type == "LOAD") return LOAD;
	if (type == "JUMP") return JUMP;
	if (type == "SHOOT") return SHOOT;
	if (type == "WAIT") return WAIT;
	if (type == "IF") return IF;
	if (type == "ADD") return ADD;
	if (type == "SUB") return SUB;
	if (type == "GOTO") return GOTO;
	if (type == "SWAP") return SWAP;
	if (type == "VALUE") return VALUE;
	if (type == "WRITE") return WRITE;
	if (type == "READ") return READ;
	if (type == "HALT") return HALT;
	return INVALID;
}

#include <algorithm>
using std::all_of;

struct WeaponParser 
{
	WeaponComputer computer;

	WeaponError load_intruction(const string& line)
	{
		vector<string> args = split(line, ' ');
		if (args.size() == 0)
		{
			WeaponInstruction empty;
			empty.empty = true;
			computer.instructions.push_back(empty);
			return {};
		}

		ComputerInstructionSet type = get_instruction_type(args.at(0));

		if (type == INVALID) return { ERROR_INVALID_INSTRUCTION };

		auto pair = get_instruction(type);

		if (pair.second > args.size() - 1) return { ERROR_TOO_MANY_ARGS };
		if (pair.second < args.size() - 1) return { ERROR_NOT_ENOUGH_ARGS };

		WeaponInstruction new_intruction;
		new_intruction.instructionFunction = pair.first;
		for (size_t i = 1; i < args.size(); i++)
		{
			string& arg = args.at(i);
			if (all_of(arg.begin(), arg.end(), isupper))
			{
				if (arg.size() == 1) //register
				{
					char c = arg.at(0);
					new_intruction[i] = c - 'A';
				}
				else //special register
				{
					return { ERROR_INVALID_ARGUMENT };
				}
			}
			else if (all_of(arg.begin(), arg.end(), isdigit))
			{
				new_intruction[i] = stoi(args.at(i));
			}
			else
			{
				return { ERROR_INVALID_ARGUMENT };
			}
		}
	}

	vector<WeaponError> load(const string& code)
	{
		vector<string> lines = split(code, '\n');
		vector<WeaponError> errors;

		size_t i = 0;
		for (auto& line : lines)
		{
			WeaponError error = load_intruction(line);
			error.line = i;
			if (error.type != ERROR_NONE)
			{
				errors.push_back(error);
			}
			i++;
		}

		return errors;
	}
};