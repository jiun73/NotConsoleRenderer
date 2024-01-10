#pragma once
#include <string>

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