#pragma once
#include "Weapons.h"

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

inline Instruction get_instruction(ComputerInstructionSet type)
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
				computer.jumpto(computer.programCounter + data[1]);
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
				if (data[1] == 0) computer.jumpto(data[2]);
			}, {ARG_VALUE, ARG_ADDRESS} };
	case JNZ:
		return { [](WeaponComputer& computer, WeaponInstruction& data)
			{
				if (data[1] != 0) computer.jumpto(data[2]);
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
				computer.jumpto(data[1]);
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
				computer.skip();
			}, { ARG_DATA } };
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

//translate intruction strings to enums
inline ComputerInstructionSet get_instruction_type(string type)
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
