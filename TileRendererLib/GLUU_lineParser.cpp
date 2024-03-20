#include "pch.h"
#include "GLUU_lineParser.h"

void GLUU::LineParser::parse_function_keyword(string_ranges kw, vector<Expression>& constants, vector<Expression>& functions, bool rev)
{
	string flat = kw.flat();

	parser->output_seq("func " + flat + (rev ? "(reversed)" : ""));

	Expression func(return_flags->back());
	func.root = false;
	func.reversed = rev;
	shared_generic var = variable_dictionnary()->get(flat);
	if (var == nullptr) { parser->add_error(GLUU_ERROR_INVALID_FUNCTION_NAME, "function '" + flat + "' doesn't exist in this scope", current_keyword->begin()); return; } // error GLUU_ERROR_INVALID_FUNCTION_NAME
	if (var->identity() == typeid(GenericFunction))
	{
		func.func = true;
		func.function = std::reinterpret_pointer_cast<GenericFunction>(var);
		func.func_name = flat;
		if (func.function->arg_count() > 0)
			functions.push_back(func);
		else
			constants.push_back(func);
	}
	else if (var->type() == typeid(Expression))
	{
		Expression& star = *(Expression*)var->raw_bytes();
		func.constant = var;
		func.user_func = true;

		func.arg_count = star.args_name.size();

		if (star.args_name.size() == 0)
			constants.push_back(func);
		else
			functions.push_back(func);
	}
	else
	{
		parser->add_error(GLUU_ERROR_INVALID_FUNCTION_NAME, "'" + flat + "' isn't a valid function type!", kw.begin()); return;
	}

	move_const_to_arg();
}

