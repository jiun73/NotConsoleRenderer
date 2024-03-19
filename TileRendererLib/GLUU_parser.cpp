#include "GLUU_exprParser.h"
#include "pch.h"


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

		keywords_func.emplace("AS", make_pair(1, [&](Parser& parser, Element& gfx, vector<string_ranges> s)
			{
				current_scope->add(make_generic_ref(gfx), s.at(0).flat());
			}));
	};

	void Parser::add_error(Errors code, const string& message, string::iterator it)
	{
		Errorinfo info;
		info.tot_ch = std::distance(source_begin, it);
		auto line_it = lines.lower_bound(info.tot_ch);
		auto prev_line_it = prev(line_it);

		if (line_it == lines.end())
		{
			info.line = prev(line_it)->second;
		}
		else
		{
			info.line = line_it->second;
		}

		if (prev_line_it == lines.end())
		{
			info.ch = info.tot_ch;
		}
		else
		{
			info.ch = info.tot_ch - prev_line_it->first;
		}

		
		info.message = message;
		info.code = code;
		errors.push_back(info);
	}

	void Parser::register_class(shared_ptr <Widget> c)
	{
		widgets.emplace(c->fetch_keyword().second, c);
	}

	void Parser::parse_declaration(string_ranges dec)
	{
		vector<string_ranges> keywords = split_and_trim(dec);

		if (keywords.size() < 1) { add_error(GLUU_ERROR_INVALID_DECLARATION, "'new' found with no declaration after", dec.begin()); return; } //error GLUU_ERROR_INVALID_DECLARATION
		if (keywords.size() < 1) { add_error(GLUU_ERROR_INVALID_DECLARATION, "'new' with no name for declaration", dec.begin()); return; } //error GLUU_ERROR_INVALID_DECLARATION

		if (keywords.at(0).flat() == "function" || keywords.at(0).flat() == "init" || keywords.at(0).flat() == "callback")
		{
			if (keywords.size() < 4) { add_error(GLUU_ERROR_INVALID_FUNCTION_DECLARATION, "Invalid function declaraction (new function = [expr])", dec.begin()); return; } //error GLUU_ERROR_INVALID_FUNCTION_DECLARATION

			string name = keywords.at(1).flat();
			size_t eq = dec.flat().find('=');
			string_ranges func = { dec.begin() + eq + 1, dec.end() };
			ExpressionParser expression_parser(this);
			Expression expr = expression_parser.parse(func);
			current_scope->add(make_shared<GenericType<Expression>>(expr), name);

			if (keywords.at(0).flat() == "init")
			{
				expr.evaluate();
			}

			if (keywords.at(0).flat() == "callback")
			{
				graphics->callbacks.push_back(expr);
			}

			std::cout << "Added new func as '" << name << "' in scope " << current_scope->name << std::endl;
		}
		else
		{
			if (!current_scope->make(keywords.at(1).flat(), keywords.at(0).flat())) { add_error(GLUU_ERROR_INVALID_TYPE, "type '" + keywords.at(0).flat() + "' doesn't exist or is not registered", dec.begin()); return; } //error GLUU_ERROR_INVALID_TYPE
			std::cout << "made new variable " << keywords.at(0).flat() << " " << keywords.at(1).flat() << " in scope " << current_scope->name << std::endl;
			if (keywords.size() > 3 && keywords.at(2).flat() == "=")
			{
				string type = keywords.at(0).flat();
				string name = keywords.at(1).flat();
				string str = keywords.at(3).flat();
				int i = current_scope->get(name)->destringify(str);
				if (i == -1)
				{
					add_error(GLUU_ERROR_INVALID_STRING_TRANSLATION, "could not convert '" + str + "' to type '" + type + "' (no translation)", dec.begin()); return;
				}
				else if (i == 0)
				{
					add_error(GLUU_ERROR_INVALID_STRING_TRANSLATION, "could not convert '" + str + "' to type '" + type + "' (invalid string)", dec.begin()); return;
				}
			}
		}
	}

	Expression Parser::parse_expression(string_ranges expression)
	{
		ExpressionParser expression_parser(this);
		return expression_parser.parse(expression);
	}
}