
#include "pch.h"
#include "GLUU_parser.h"

namespace GLUU
{
	Parser::Parser()
	{
		GLUU_scope = variable_dictionnary()->make_new_scope("GLUU");

		keywords_func.emplace("SIZE", make_pair(1, [](Parser& parser, Element& gfx, vector<string_ranges> s)
			{
				gfx.size.set(s.at(0), parser);
			}));

		keywords_func.emplace("IF", make_pair(1, [](Parser& parser, Element& gfx, vector<string_ranges> s)
			{
				gfx.condition.set(s.at(0), parser);
			}));

		keywords_func.emplace("FIT", make_pair(1, [](Parser& parser, Element& gfx, vector<string_ranges> s)
			{
				gfx.fit.set(s.at(0), parser);
			}));

		keywords_func.emplace("CFIT", make_pair(0, [](Parser& parser, Element& gfx, vector<string_ranges> s)
			{
				gfx.fit() = true;
			}));
	};

	void Parser::add_error(Errors code, const string& message, string::iterator it)
	{
		Errorinfo info;
		info.tot_ch = std::distance(source_begin, it);
		auto line_it = lines.lower_bound(info.tot_ch);
		auto prev_line_it = prev(lines.lower_bound(info.tot_ch));
		info.line = line_it->second;
		info.ch = info.tot_ch - prev_line_it->first;
		info.message = message;
		info.code = code;
		errors.push_back(info);
	}

	void Parser::register_class(shared_ptr <Widget> c)
	{
		widgets.emplace(c->fetch_keyword().second, c);
	}

	bool Parser::parse_keyword(vector<string_ranges>::iterator kw, vector<Expression>& constants, vector<Expression>& functions, bool& f, string_ranges full, size_t size, Expression& ret_val)
	{
		string flat = kw->flat();

		if (f && flat == "new")
		{
			get_declaractions(full);
			return true;
		}
		if (f && flat == "arg")
		{
			if (size < 3) {add_error(GLUU_ERROR_INVALID_ARG_FORMAT, "not enough params in arg expression -> (arg type name)", kw->begin()); return true; } //error GLUU_ERROR_INVALID_ARG_FORMAT
			string type = next(kw)->flat();
			string name = next(kw, 2)->flat();
			ret_val.add_arg(name, type);
			output_seq("arg '" + name + "' as " + type);
			return true;
		}
		if (f && flat == "arg*")
		{
			if (size < 3) { add_error(GLUU_ERROR_INVALID_ARG_FORMAT, "not enough params in arg ptr expression -> (arg* type name)", kw->begin()); return true; } //error GLUU_ERROR_INVALID_ARG_FORMAT
			string type = next(kw)->flat();
			string name = next(kw, 2)->flat();
			ret_val.add_arg_ref(name, type);
			output_seq("arg* '" + name + "' as " + type);
			return true;
		}
		else if (flat == "this")
		{
			Expression constant;
			constant.root = false;
			constant.constant = std::make_shared<GenericRef<Expression>>(ret_val);
			constants.push_back(constant);
		}
		else if (is_char(*kw))
		{
			output_seq("ch " + kw->flat());
			constants.push_back(make_const<char>(range_shave(*kw).flat().at(0)));
		}
		else if (is_string(*kw))
		{
			output_seq("str " + kw->flat());
			constants.push_back(make_const<string>(range_shave(*kw).flat()));
		}
		else if (is_num(*kw))
		{
			Expression constant;
			constant.root = false;
			constant.constant = make_generic<int>();
			constant.constant->destringify(kw->flat());
			output_seq("num " + kw->flat());

			constants.push_back(constant);
		}
		else if (is_var_name(*kw))
		{
			output_seq("var " + kw->flat());

			Expression constant;
			constant.root = false;
			shared_generic var = variable_dictionnary()->get(flat);
			if (var == nullptr) { add_error(GLUU_ERROR_INVALID_VARIABLE_NAME, "variable '" + flat + "' doesn't exist in this scope", kw->begin()); return true; }
			constant.constant = var;
			constants.push_back(constant);
		}
		else if (is_func_name(*kw))
		{
			parse_function_keyword(*kw, constants, functions);
		}
		else
		{
			add_error(GLUU_ERROR_INVALID_EXPRESSION_KEYWORD, "could not deduce type of keyword from '" + kw->flat() + "' ", kw->begin()); // error GLUU_ERROR_INVALID_EXPRESSION_KEYWORD
		}

		f = false;
		return false;
	}

	void Parser::parse_expression(string_ranges str, Expression& ret_val)
	{
		str = range_trim(str, ' ');

		output_seq(str.flat());
		next_level();
		
		vector<string_ranges> ignore = range_delimiter(str, expr_open, expr_close);

		vector<Expression> constants;
		vector<Expression> functions;

		bool f = true;
		for (auto it = ignore.begin(); it != ignore.end(); it += 2)
		{
			vector<string_ranges> ignore2 = range_delimiter(*it, '"', '"');
			for (auto it2 = ignore2.begin(); it2 != ignore2.end(); it2 += 2)
			{
				vector<string_ranges> keywords = subchain(chain(*it2, range_until, " "), range_until, ",");
				for (auto kw = keywords.begin(); kw != keywords.end(); kw++)
				{
					if (parse_keyword(kw, constants, functions, f, str, keywords.size(), ret_val)) return;
				}

				if (!next(it2)->empty())
				{
					string_ranges r2 = *next(it2);
					output_seq(r2.flat());
					constants.push_back(make_const<string>(range_shave(r2).flat()));
				}
			}

			if (!next(it)->empty())
			{
				//output_seq(next(it)->flat());
				Expression sub = parse_sequence_next(*next(it));
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
					add_error(GLUU_ERROR_NOT_ENOUGH_ARGS, "ran out of arguments for function '" + it->func_name + "' ", str.begin());
				}

				it->recursive.push_back(constants.back());
				constants.pop_back();
			}

			constants.push_back(*it);

		}

		for (auto& c : constants)
			ret_val.recursive.push_back(c);

		prev_level();
	}
}