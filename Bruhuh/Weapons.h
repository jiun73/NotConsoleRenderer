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
	array<bool, 3> isRegisterValue;

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
	ERROR_INVALID_INSTRUCTION,
	ERROR_EXPECTING_VALUE,
	ERROR_EXPECTING_REGISTER,
	ERROR_EXPECTING_ADDRESS,
	ERROR_EXPECTING_ARGUMENT
};

struct WeaponError 
{
	size_t offset = 0;
	ComputerErrorTypes type = ERROR_NONE;
	ComputerErrorTypes subtype = ERROR_NONE;
	int line = 0;
	
};

struct WeaponComputer
{
	vector<WeaponRegister> registers;

	int temperature = 0;
	int maxTemp = 0;
	
	int storage = 0;

	WeaponBullet bulletType;

	uint32_t clockSpeed = 100;
	uint32_t clockLast = 0;

	size_t programCounter = 0;
	vector<WeaponInstruction> instructions; 

	bool doNotIncrCounter = false;

	void shoot() { bulletType.bulletCreationFunction(); }

	bool cycle()
	{
		if (instructions.size() == 0) return true;
		if (programCounter > instructions.size() - 1) programCounter = 0;

		WeaponInstruction& instruction = instructions.at(programCounter);

		if (instruction.empty) { programCounter++; return false; }

		for (size_t i = 0; i < instruction.inputs.size(); i++)
		{
			bool isReg = instruction.isRegisterValue.at(i);

			if (isReg)
			{
				if (instruction.inputs.at(i) < 0) continue;

				instruction.values.at(i) = registers.at(instruction.inputs.at(i)).value;
			}
			else
			{
				instruction.values.at(i) = instruction.inputs.at(i);
			}
		}

		instruction.instructionFunction(*this, instruction);

		if (doNotIncrCounter) { doNotIncrCounter = false; return true; }
		programCounter++;
		return true;
	}

	void tick()
	{
		uint32_t timeSinceLast = SDL_GetTicks() - clockLast;

		while (timeSinceLast > clockSpeed)
		{
			
			if(cycle())
			timeSinceLast -= clockSpeed;
		}

		clockLast = SDL_GetTicks() - timeSinceLast;
	}
};

enum InstructionArgTypes 
{
	ARG_REGISTER,
	ARG_VALUE,
	ARG_ADDRESS
};

struct Instruction 
{
	function<void(WeaponComputer&, WeaponInstruction&)> function;
	vector<int> args_types;
};

Instruction get_instruction(ComputerInstructionSet type)
{
	switch (type)
	{
	case LOAD:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				std::cout << data[1] << " " << data[2] << std::endl;
				computer.registers.at(data[2]).value = data[1];
			}, {ARG_VALUE, ARG_REGISTER} };
	case JUMP:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.programCounter += data[1];
				computer.doNotIncrCounter = true;
			}, {ARG_ADDRESS} };
	case SHOOT:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.shoot();
			},{} };
	case WAIT:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.clockLast += data[1];
			}, {ARG_VALUE} };
	case IF:
		break;
	case ADD:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.registers.at(data[3]).value = data[1] + data[2];
			}, {ARG_VALUE, ARG_VALUE, ARG_REGISTER} };
	case SUB:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.registers.at(data[3]).value = data[1] - data[2];
			}, {ARG_VALUE, ARG_VALUE, ARG_REGISTER} };
	case GOTO:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.programCounter = data[1];
				computer.doNotIncrCounter = true;
			}, {ARG_ADDRESS} };
	case SWAP:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				int buffer = computer.registers.at(data[1]).value;
				computer.registers.at(data[1]).value = computer.registers.at(data[2]).value;
				computer.registers.at(data[2]).value = buffer;
			}, {ARG_REGISTER, ARG_REGISTER} };
	case VALUE:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.cycle();
			}, { ARG_VALUE } };
	case WRITE:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{

				WeaponInstruction val;
				val.instructionFunction = get_instruction(VALUE).function;
				val.inputs.at(0) = data[1];
				val[1] = data[1];

				computer.instructions.at(data[2]) = val;
			}, { ARG_VALUE, ARG_ADDRESS} };
	case READ:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				WeaponInstruction& instr = computer.instructions.at(data[1]);
				if (instr.data)
				{
					computer.registers.at(data[2]).value = instr[1];
				}
			}, {ARG_ADDRESS, ARG_REGISTER} };
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

	size_t get_true_offset(const vector<string>& args, size_t offset)
	{
		size_t ret = 0;
		for (size_t i = 0; i < std::min(args.size(), offset); i++)
		{
			ret += args.at(i).size() + 1;
		}
		return ret;
	}

	WeaponError load_intruction(const string& line)
	{
		vector<string> args = split(line, ' ');

		if (args.size() == 0)
		{
			return {};
		}

		ComputerInstructionSet type = get_instruction_type(args.at(0));

		if (type == INVALID) return { 0, ERROR_INVALID_INSTRUCTION };

		Instruction info = get_instruction(type);

		WeaponInstruction new_intruction;
		new_intruction.empty = false;
		for (auto& is : new_intruction.isRegisterValue)
		{
			is = false;
		}

		new_intruction.instructionFunction = info.function;
		for (size_t i = 1; i < args.size(); i++)
		{
			string& arg = args.at(i);

			if (arg.size() == 0) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_ARGUMENT };

			int& input = new_intruction.inputs.at(i - 1);

			switch (info.args_types.at(i - 1))
			{
			case ARG_VALUE:
			{
				bool isRegisterValue = all_of(arg.begin(), arg.end(), isupper);
				bool isConstantValue = all_of(arg.begin(), arg.end(), isdigit);
				if (!(isRegisterValue || isConstantValue)) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_VALUE };
				if (isConstantValue) input = stoi(arg);
				else if (isRegisterValue) 
				{ 	
					if (arg.size() != 1) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_REGISTER };
					input = arg.at(0) - 'A';
					new_intruction.isRegisterValue.at(i - 1) = true;
				}
				break;
			}

			case ARG_REGISTER:
				if(!all_of(arg.begin(), arg.end(), isupper)) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_REGISTER };
				if (arg.size() != 1) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_REGISTER };
				input = arg.at(0) - 'A';
				break;
			case ARG_ADDRESS:
			{
				bool isRegisterValue = all_of(arg.begin(), arg.end(), isupper);

				if (isRegisterValue)
				{
					if (arg.size() != 1) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_REGISTER };
					input = arg.at(0) - 'A';
					new_intruction.isRegisterValue.at(i - 1) = true;
				}
				else
				{
					if (arg.size() <= 1) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_ADDRESS };
					if (arg.at(0) != '$') return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_ADDRESS };

					string address = arg.substr(1);
					input = stoi(address);
				}
			}
				break;
			default:
				return { get_true_offset(args, i), ERROR_INVALID_INSTRUCTION };
			}
		}

		if (info.args_types.size() < args.size() - 1) return { get_true_offset(args, args.size()), ERROR_TOO_MANY_ARGS };
		if (info.args_types.size() > args.size() - 1) return { get_true_offset(args, args.size()), ERROR_NOT_ENOUGH_ARGS };

		computer.instructions.push_back(new_intruction);

		return {};
	}

	vector<WeaponError> load(const string& code)
	{
		computer.instructions.clear();
		computer.instructions.resize(computer.storage);
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