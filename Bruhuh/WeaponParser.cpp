#include "WeaponParser.h"

inline size_t WeaponParser::get_true_offset(const vector<string>& args, size_t offset)
{
	size_t ret = 0;
	for (size_t i = 0; i < std::min(args.size(), offset); i++)
	{
		ret += args.at(i).size() + 1;
	}
	return ret;
}

inline WeaponError WeaponParser::load_intruction(const string& line, size_t i)
{
	vector<string> args = split(line, ' ');

	if (args.size() == 0)
	{
		return {};
	}

	ComputerInstructionSet type = get_instruction_type(args.at(0));

	if (type == INVALID) return { 0, ERROR_INVALID_INSTRUCTION };
	if (!inventory_copy.infinite)
	{
		if (inventory_copy.available_intructions.count(type) && inventory_copy.available_intructions.at(type) > 0)
		{
			inventory_copy.available_intructions.at(type).val--;
		}
		else return  { 0, ERROR_NOT_IN_INVENTORY };
	}

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
			bool isConstantValue = isNum(arg);
			if (!(isRegisterValue || isConstantValue)) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_VALUE };
			if (isConstantValue) {
				if (arg.size() == 1 && arg.at(0) == '-') return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_VALUE };
				input = stoi(arg);
				if (!inventory_copy.infinite)
				{
					if (inventory_copy.available_constants.count(input) && inventory_copy.available_constants.at(input) > 0)
					{
						inventory_copy.available_constants.at(input).val--;
					}
					else return  { get_true_offset(args, i), ERROR_NOT_IN_INVENTORY };
				}
			}
			else if (isRegisterValue)
			{
				if (arg.size() != 1) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_REGISTER };
				input = arg.at(0) - 'A';
				new_intruction.isRegisterValue.at(i - 1) = true;
			}
			break;
		}
		case ARG_SPECIAL_REGISTER:
			if (arg.size() > 1 && arg.at(0) == '#')
			{
				string sp_reg_name = arg.substr(1);
				if (!computer->bulletType.registers.count(sp_reg_name)) return { get_true_offset(args, i) , ERROR_NOT_A_REGISTER, ERROR_NONE };
				string& sp_input = new_intruction.specialInputs.at(i - 1);
				sp_input = sp_reg_name;
				break;
			}
			if (arg.size() == 1 && arg.at(0) == '#') return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_SPECIAL_REGISTER };
			[[fallthrough]];
		case ARG_REGISTER:
			if (!all_of(arg.begin(), arg.end(), isupper)) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_REGISTER };
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
				if(!isNum(address)) return { get_true_offset(args, i), ERROR_INVALID_ARGUMENT, ERROR_EXPECTING_ADDRESS };
				input = stoi(address);
			}
		}
		break;
		default:
			return { get_true_offset(args, i), ERROR_INVALID_INSTRUCTION };
		}
	}

	if (info.args_types.size() > args.size() - 1) return { get_true_offset(args, args.size()), ERROR_NOT_ENOUGH_ARGS };

	computer->instructions.at(i) = new_intruction;

	return {};
}

vector<WeaponError> WeaponParser::load(WeaponComputer& computer, WeaponInventory& inventory)
{
	computer.instructions.clear();
	computer.instructions.resize(computer.settings.storage);
	source.resize(computer.settings.storage);
	computer.source = &source;
	inventory_copy = inventory;

	this->inventory = &inventory;
	this->computer = &computer;

	vector<WeaponError> errors;

	size_t i = 0;
	for (auto& line : source)
	{
		WeaponError error = load_intruction(line, i);
		error.line = (int)i;
		if (error.type != ERROR_NONE)
			errors.push_back(error);
		i++;
	}

	return errors;
}
