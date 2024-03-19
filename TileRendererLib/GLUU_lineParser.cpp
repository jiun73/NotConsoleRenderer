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
	if (var == nullptr) { parser->add_error(GLUU_ERROR_INVALID_FUNCTION_NAME, "function '" + flat + "' doesn't exist in this scope", kw.begin()); return; } // error GLUU_ERROR_INVALID_FUNCTION_NAME
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
}

bool GLUU::LineParser::parse_keyword(vector<string_ranges>::iterator& kw, vector<Expression>& constants, vector<Expression>& functions, bool& f, string_ranges full, size_t size, Expression& ret_val, vector<string_ranges>::iterator end, bool& make_return)
{
	string flat = kw->flat();

	if (flat.empty()) return false;


	if (f && flat == "new")
	{
		parser->get_declaractions(full);
		return true;
	}
	if (f && flat == "return")
	{
		make_return = true;
		return false;
	}
	if (f && flat == "arg")
	{
		if (size < 3) { parser->add_error(GLUU_ERROR_INVALID_ARG_FORMAT, "not enough params in arg expression -> (arg type name)", kw->begin()); return true; } //error GLUU_ERROR_INVALID_ARG_FORMAT
		string type = next(kw)->flat();
		string name = next(kw, 2)->flat();

		if (!ClassFactory::get()->has(type)) { parser->add_error(GLUU_ERROR_INVALID_TYPE, "type '" + type + "' does not exist or is not registered", kw->begin()); return true; }

		ret_val.add_arg(name, type);
		parser->output_seq("arg '" + name + "' as " + type);
		return true;
	}
	if (f && flat == "arg*")
	{
		if (size < 3) { parser->add_error(GLUU_ERROR_INVALID_ARG_FORMAT, "not enough params in arg ptr expression -> (arg* type name)", kw->begin()); return true; } //error GLUU_ERROR_INVALID_ARG_FORMAT
		string type = next(kw)->flat();
		string name = next(kw, 2)->flat();

		if (!ClassFactory::get()->has(type)) { parser->add_error(GLUU_ERROR_INVALID_TYPE, "type '" + type + "' does not exist or is not registered", kw->begin()); return true; }

		ret_val.add_arg_ref(name, type);
		parser->output_seq("arg* '" + name + "' as " + type);
		return true;
	}
	/*if (is_member(*kw))
	{
		output_seq("member '" + kw->flat() + "'");
		string_ranges member_expr = *kw;

		vector<string_ranges> split = chain(member_expr, range_until, ".");

		shared_generic var;
		bool b = true;
		next_level();
		for (auto& s : split)
		{
			output_seq(s.flat());
			if (b)
			{
				var = variable_dictionnary()->get(s.flat());
				if(var == nullptr) { add_error(GLUU_ERROR_INVALID_VARIABLE_NAME, "variable '" + s.flat() + "' doesn't exist in this scope", kw->begin()); return true; }
			}
			else
			{
				if (!inspectors.count(var->type())) { add_error(GLUU_ERROR_INVALID_VARIABLE_NAME, "'" + string(var->type().name()) + "' has no members", kw->begin()); return true; }
				var = inspectors.at(var->type()).inspect(var, s.flat());
				if (var == nullptr) { add_error(GLUU_ERROR_INVALID_VARIABLE_NAME, "no member '" + s.flat() + "' ", kw->begin()); return true; }
			}
			b = false;
		}
		prev_level();

		Expression constant(return_flags.back());
		constant.root = false;
		constant.constant = var;
		constants.push_back(constant);
	}
	else */
	if (flat == "this")
	{
		Expression constant(return_flags->back());
		constant.root = false;
		constant.constant = std::make_shared<GenericRef<Expression>>(&ret_val);
		constants.push_back(constant);
	}
	else if (is_char(*kw))
	{
		parser->output_seq("ch " + kw->flat());
		constants.push_back(make_const<char>(range_shave(*kw).flat().at(0)));
	}
	else if (is_string(*kw))
	{
		parser->output_seq("str " + kw->flat());
		constants.push_back(make_const<string>(range_shave(*kw).flat()));
	}
	else if (is_num(*kw))
	{
		Expression constant(return_flags->back());
		constant.root = false;
		constant.constant = make_generic<int>();
		constant.constant->destringify(kw->flat());
		parser->output_seq("num " + kw->flat());

		constants.push_back(constant);
	}
	else if (is_var_name(*kw))
	{
		parser->output_seq("var " + kw->flat());

		Expression constant(return_flags->back());
		constant.root = false;
		shared_generic var = variable_dictionnary()->get(flat);

		if (var == nullptr) { parser->add_error(GLUU_ERROR_INVALID_VARIABLE_NAME, "variable '" + flat + "' doesn't exist in this scope", kw->begin()); return true; }
		constant.constant = var;
		constants.push_back(constant);
	}
	else if (flat.front() == '.')
	{
		parser->output_seq("member '" + kw->flat() + "'");

		if (constants.empty())
		{
			parser->add_error(GLUU_ERROR_INVALID_MEMBER_EXPRESSION, "Cannot call " + flat + " on nothing", kw->begin());
			return false;
		}

		Expression copy = constants.back();

		if (copy.get_type() != typeid(shared_generic))
		{
			if (!parser->inspectors.count(copy.get_type()))
			{
				parser->add_error(GLUU_ERROR_INVALID_VARIABLE_NAME, "'" + string(copy.get_type().name()) + "' has no members", kw->begin());
				return false;
			};
		}

		Expression inspector_expr(return_flags->back());
		inspector_expr.root = false;
		inspector_expr.is_inspector = true;
		inspector_expr.inspectors = &parser->inspectors;
		inspector_expr.recursive.push_back(copy);
		inspector_expr.member = flat.substr(1);
		constants.pop_back();
		constants.push_back(inspector_expr);
	}
	else if (flat.front() == '`' && flat.size() > 1 && is_func_name({ kw->begin() + 1, kw->end() }))
	{
		parse_function_keyword({ kw->begin() + 1, kw->end() }, constants, functions, true);
	}
	else if (is_func_name(*kw))
	{
		parse_function_keyword(*kw, constants, functions);
	}
	else
	{
		parser->add_error(GLUU_ERROR_INVALID_EXPRESSION_KEYWORD, "could not deduce type of keyword from '" + kw->flat() + "' ", kw->begin()); // error GLUU_ERROR_INVALID_EXPRESSION_KEYWORD
	}

	f = false;
	return false;
}

void GLUU::LineParser::add_constant(vector<Expression>& constants, const Expression& constant)
{
}

void GLUU::LineParser::parse_expression(string_ranges str, Expression& ret_val)
{
	str = range_trim(str, ' ');

	parser->output_seq(str.flat());
	parser->next_level();

	vector<string_ranges> ignore = range_delimiter(str, parser->expr_open, parser->expr_close);

	vector<Expression> constants;
	vector<Expression> functions;

	bool f = true;
	bool make_return = false;

	for (auto it = ignore.begin(); it != ignore.end(); it += 2)
	{
		vector<string_ranges> ignore2 = range_delimiter(*it, '"', '"');
		for (auto it2 = ignore2.begin(); it2 != ignore2.end(); it2 += 2)
		{
			vector<string_ranges> keywords = subchain(chain(*it2, range_until, " "), range_until, ",");
			for (auto kw = keywords.begin(); kw != keywords.end(); kw++)
			{
				if (parse_keyword(kw, constants, functions, f, str, keywords.size(), ret_val, keywords.end(), make_return)) return;
			}

			if (!next(it2)->empty())
			{
				std::cout << next(it2)->flat() << std::endl;
				string_ranges r2 = *next(it2);
				parser->output_seq(r2.flat());
				constants.push_back(make_const<string>(range_shave(r2).flat()));
			}
		}

		if (!next(it)->empty())
		{
			Expression sub = expression_parser->parse_sequence_next(*next(it));
			constants.push_back(sub);
		}
	}

	for (auto it = functions.rbegin(); it != functions.rend(); it++)
	{
		size_t count = 0;

		if (it->func) count = it->function->arg_count();
		else count = it->arg_count;

		for (size_t i = 0; i < count; i++)
		{
			if (constants.empty())
			{
				parser->add_error(GLUU_ERROR_NOT_ENOUGH_ARGS, "ran out of arguments for function '" + it->func_name + "' ", str.begin());
				return;
			}

			it->recursive.push_back(constants.back());
			constants.pop_back();
		}

		constants.push_back(*it);

	}

	for (auto& c : constants)
		ret_val.recursive.push_back(c);

	if (make_return)
	{
		if (ret_val.recursive.size() > 0)
		{
			Expression copy = ret_val;
			ret_val = Expression(return_flags->back());
			ret_val.return_call = true;
			ret_val.recursive.push_back(copy);
			ret_val.scope = copy.scope;
		}
		else
		{
			ret_val.return_call = true;
			ret_val.ret_flag = return_flags->back();
		}
	}

	parser->prev_level();
}
