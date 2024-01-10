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
	JEZ,
	JNZ,
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
	int bit = 5;
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
	vector<string>* source;

	int temperature = 0;
	int maxTemp = 0;
	
	int storage = 0;

	WeaponBullet bulletType;

	uint32_t clockSpeed = 100;
	uint32_t clockLast = 0;

	size_t programCounter = 0;
	size_t emptyCnt = 0;
	vector<WeaponInstruction> instructions; 

	bool doNotIncrCounter = false;

	void shoot() { bulletType.bulletCreationFunction(); }

	bool next_instr() 
	{
		if (instructions.size() == 0) return false;

		while (instructions.at(programCounter).empty)
		{
			programCounter++; 

			if (programCounter > instructions.size() - 1) programCounter = 0;
			
			emptyCnt++;
			if (emptyCnt == storage) { emptyCnt = 0; return false; }
		}
		emptyCnt = 0;
		return true;
	}

	void set_register_values() 
	{
		for (auto& reg : registers)
		{
			int bits = reg.bit - reg._signed_;

			int max = 0;
			for (size_t i = 0; i < bits; i++) max |= 1 << i;

			if (!reg._signed_)
			{
				if (reg.value < 0) reg.value = max - (-reg.value % (max + 1));
				if (reg.value > max) reg.value = reg.value % (max + 1);
			}
			else
			{
				if (reg.value > max) reg.value = -max + (reg.value % (max + 1));
				if (reg.value < -max) reg.value = max - (-reg.value % (max + 1));
			}
		}
	}

	void cycle()
	{
		WeaponInstruction& instruction = instructions.at(programCounter);

		if (instruction.empty) return;

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

		if (!doNotIncrCounter)  
			programCounter++;
		else
			doNotIncrCounter = false;
	}

	void tick()
	{
		uint32_t timeSinceLast = SDL_GetTicks() - clockLast;
		
		if (timeSinceLast > clockSpeed)
		{
			cycle();
			set_register_values();
			next_instr();
			clockLast = SDL_GetTicks();
		}
	}
};

enum InstructionArgTypes 
{
	ARG_REGISTER,
	ARG_VALUE,
	ARG_ADDRESS,
	ARG_DATA
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
				computer.registers.at(data[2]).value = data[1];
			}, {ARG_VALUE, ARG_REGISTER} };
	case JUMP:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				computer.programCounter += data[1];
				computer.doNotIncrCounter = true;
			}, {ARG_VALUE} };
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
	case JEZ:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				if (data[1] == 0)
				{
					computer.programCounter = data[2];
					computer.doNotIncrCounter = true;
				}
			}, {ARG_VALUE, ARG_ADDRESS} };
	case JNZ:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				if (data[1] != 0)
				{
					computer.programCounter = data[2];
					computer.doNotIncrCounter = true;
				}
			}, {ARG_VALUE, ARG_ADDRESS} };
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
			{}, { ARG_DATA } };
	case WRITE:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{

				WeaponInstruction val;
				val.instructionFunction = get_instruction(VALUE).function;
				val.inputs.at(0) = data[1];
				val[1] = data[1];
				val.data = true;

				computer.instructions.at(data[2]) = val;
				computer.source->at(data[2]) = "VALUE " + std::to_string(data[1]);
			}, { ARG_VALUE, ARG_ADDRESS} };
	case READ:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				WeaponInstruction& instr = computer.instructions.at(data[1]);
				if (!instr.empty && instr.data)
				{
					computer.registers.at(data[2]).value = instr.inputs.at(0);
				}
				else
					computer.registers.at(data[2]).value = 0;
			}, {ARG_ADDRESS, ARG_REGISTER} };
	case HALT:
		break;
	default:
		break;
	}
}

ComputerInstructionSet get_instruction_type(string type)
{
	if (type == "LOAD")		return LOAD;
	if (type == "JUMP")		return JUMP;
	if (type == "SHOOT")	return SHOOT;
	if (type == "WAIT")		return WAIT;
	if (type == "JEZ")		return JEZ;
	if (type == "JNZ")		return JNZ;
	if (type == "ADD")		return ADD;
	if (type == "SUB")		return SUB;
	if (type == "GOTO")		return GOTO;
	if (type == "SWAP")		return SWAP;
	if (type == "VALUE")	return VALUE;
	if (type == "WRITE")	return WRITE;
	if (type == "READ")		return READ;
	if (type == "HALT")		return HALT;
	return INVALID;
}

#include <algorithm>
using std::all_of;

struct WeaponParser 
{
	WeaponComputer computer;
	vector<string> source;

	size_t get_true_offset(const vector<string>& args, size_t offset)
	{
		size_t ret = 0;
		for (size_t i = 0; i < std::min(args.size(), offset); i++)
		{
			ret += args.at(i).size() + 1;
		}
		return ret;
	}

	WeaponError load_intruction(const string& line, size_t i )
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

		if (info.args_types.size() < args.size() - 1) return { get_true_offset(args, args.size()), ERROR_TOO_MANY_ARGS };

		new_intruction.instructionFunction = info.function;
		for (size_t i = 1; i < args.size(); i++)
		{
			string& arg = args.at(i);

			if (arg.size() == 0) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_ARGUMENT };

			int& input = new_intruction.inputs.at(i - 1);

			switch (info.args_types.at(i - 1))
			{
			case ARG_DATA:
				new_intruction.data = true;
				[[fallthrough]];
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

		if (info.args_types.size() > args.size() - 1) return { get_true_offset(args, args.size()), ERROR_NOT_ENOUGH_ARGS };

		computer.instructions.at(i) = new_intruction;

		return {};
	}

	vector<WeaponError> load(const string& code)
	{
		computer.instructions.clear();
		computer.instructions.resize(computer.storage);
		computer.source = &source;
		//vector<string> lines = split(code, '\n');
		vector<WeaponError> errors;

		size_t i = 0;
		for (auto& line : source)
		{
			WeaponError error = load_intruction(line, i);
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

inline std::string get_error_message(const WeaponError& error)
{
	switch (error.type)
	{
	case ERROR_INVALID_ARGUMENT:
		switch (error.subtype) {
		case ERROR_EXPECTING_VALUE:		return "Argument invalid! expected: constant or register";
		case ERROR_EXPECTING_REGISTER:	return "Argument invalid! expected: register";
		case ERROR_EXPECTING_ADDRESS:	return "Argument invalid! expected: address or register";
		case ERROR_EXPECTING_ARGUMENT:	return "Argument invalid! expected: non-empty argument";
		default: return "Argument invalid!";
		}

	case ERROR_NOT_A_REGISTER:		return "Register not available!";
	case ERROR_NOT_IN_INVENTORY:	return "Not enough tokens in inventory!";
	case ERROR_TOO_MANY_ARGS:		return "Too many arguments!";
	case ERROR_NOT_ENOUGH_ARGS:		return "Not enough arguments!";
	case ERROR_INVALID_INSTRUCTION:	return "Instruction doesn't exist!";
	default:						return "Something went wrong!";
	}
}