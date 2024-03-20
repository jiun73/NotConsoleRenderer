#pragma once
#include "GLUU_exprParser.h"

namespace GLUU {
	class LineParser
	{
		Parser* parser = nullptr;
		ExpressionParser* expression_parser = nullptr;

		queue<shared_ptr<bool>>* return_flags = nullptr;

		vector<string_ranges> keywords;
		vector<string_ranges>::iterator current_keyword;
		vector<Expression> constants;
		vector<Expression> functions;

		string_ranges full;
		string dummy = "";

		Expression* ret_val = nullptr;
		bool is_first = false;
		bool make_return = false;

	public:
		LineParser(Parser* parser, ExpressionParser* expression_parser, queue<shared_ptr<bool>>* return_flags) : parser(parser), return_flags(return_flags), expression_parser(expression_parser) , full(dummy){}
		~LineParser() {};

		void move_const_to_arg()
		{
			if (functions.empty()) return;

			if (functions.back().all_args_set())
			{
				constants.push_back(functions.back());
				functions.pop_back();
			}

			while (!functions.empty() && !constants.empty())
			{
				bool full = functions.back().set_next_arg(constants.back());
				constants.pop_back();

				if (full)
				{
					constants.push_back(functions.back());
					functions.pop_back();
				}
			}
		}

		void parse_function_keyword(string_ranges kw, vector<Expression>& constants, vector<Expression>& functions, bool rev = false);

		void add_variable_constant(shared_generic variable)
		{
			Expression constant(return_flags->back());
			constant.root = false;
			constant.constant = variable;
			add_constant(constant);
		}

		bool parse_static_keywords(string flat_keyword, bool& do_break)
		{
			if (!is_first) return false;

			

			if (flat_keyword == "new")
			{
				parser->get_declaractions(full);
				do_break = true;
				return true;
			}
			else if (ClassFactory::get()->has(flat_keyword))
			{
				
				if (keywords.size() < 2) { parser->add_error(GLUU_ERROR_INVALID_DECLARATION, "not enough params in arg expression -> (arg type name)", current_keyword->begin()); return true; }
				string type = flat_keyword;
				string name = next(current_keyword)->flat();

				if (!parser->current_scope->make(name, type)) { parser->add_error(GLUU_ERROR_INVALID_TYPE, "type '" + keywords.at(0).flat() + "' doesn't exist or is not registered", next(current_keyword)->begin()); do_break = true; return true;} //error GLUU_ERROR_INVALID_TYPE

				parser->output_seq("declared variable '" + name + "' as " + type + " in scope " + parser->current_scope->name);

				shared_generic var = parser->get_variable_from_scope(name);
				if (var == nullptr) { do_break = true; return true; }
				add_variable_constant(var);

				current_keyword += 1;

				do_break = false;
				return true;
			}
			else if (flat_keyword == "return")
			{
				parser->output_seq("return");
				make_return = true;
				do_break = false;
				return true;
			}
			else if (flat_keyword == "arg" || flat_keyword == "arg*")
			{
				if (keywords.size() < 3) { parser->add_error(GLUU_ERROR_INVALID_ARG_FORMAT, "not enough params in arg expression -> (arg type name)", current_keyword->begin()); return true; } //error GLUU_ERROR_INVALID_ARG_FORMAT
				string type = next(current_keyword)->flat();
				string name = next(current_keyword, 2)->flat();

				if (!ClassFactory::get()->has(type)) { parser->add_error(GLUU_ERROR_INVALID_TYPE, "type '" + type + "' does not exist or is not registered", current_keyword->begin()); return true; }

				parser->output_seq(flat_keyword + " '" + name + "' as " + type);

				if (flat_keyword == "arg")
					ret_val->add_arg(name, type);
				else if (flat_keyword == "arg*")
					ret_val->add_arg_ref(name, type);

				do_break = true;
				return true;
			}

			return false;
		}

		bool parse_keyword(string_ranges keyword)
		{
			string flat = keyword.flat();

			if (keyword.empty()) return false;

			bool do_break = false;
			if (parse_static_keywords(flat, do_break))
			{
				return do_break;
			}
			else if (flat == "false")
			{
				parser->output_seq("bool " + flat);
				add_constant(make_const<bool>(false));
			}
			else if (flat == "true")
			{
				parser->output_seq("bool " + flat);
				add_constant(make_const<bool>(true));
			}
			else if (is_char(flat))
			{
				//TODO: GLUU_ERROR_INVALID_CHAR_DECLARATION

				parser->output_seq("character " + flat.at(0));
				add_constant(make_const<char>(range_shave(keyword).flat().at(0)));
			}
			else if (is_string(flat))
			{
				parser->output_seq("string '" + flat + "'");
				add_constant(make_const<string>(range_shave(keyword).flat()));
			}
			else if (is_num(flat))
			{
				Expression constant(return_flags->back());
				constant.constant = make_generic<int>();
				constant.constant->destringify(flat);
				parser->output_seq("number '" + flat + "'");
				add_constant(constant);
			}
			else if (is_var_name(flat))
			{
				parser->output_seq("variable '" + flat + "'");
				shared_generic var = parser->get_variable_from_scope(keyword);
				if (var == nullptr) return true;
				add_variable_constant(var);
			}
			else if (flat.front() == '.')
			{
				parser->output_seq("member '" + flat + "'");

				if (constants.empty())
				{
					parser->add_error(GLUU_ERROR_INVALID_MEMBER_EXPRESSION, "Cannot call " + flat + " on nothing", keyword.begin());
					return false;
				}

				Expression copy = constants.back();

				if (copy.get_type() != typeid(shared_generic))
				{
					if (!parser->inspectors.count(copy.get_type()))
					{
						parser->add_error(GLUU_ERROR_INVALID_VARIABLE_NAME, "'" + string(copy.get_type().name()) + "' has no members",  keyword.begin());
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
				add_constant(inspector_expr);
			}
			else if (flat.front() == '`' && flat.size() > 1 && is_func_name({ keyword.begin() + 1,  keyword.end() }))
			{
				parse_function_keyword({ keyword.begin() + 1,  keyword.end() }, constants, functions, true);
			}
			else if (is_func_name(keyword))
			{
				parse_function_keyword(flat, constants, functions);
			}
			else
			{
				parser->add_error(GLUU_ERROR_INVALID_EXPRESSION_KEYWORD, "could not deduce type of keyword from '" + flat + "' ", keyword.begin()); // error GLUU_ERROR_INVALID_EXPRESSION_KEYWORD
			}

			is_first = false;
			return false;
		}

		void add_constant(const Expression& constant)
		{
			constants.push_back(constant);
			move_const_to_arg();
		}

		void parse_expression(string_ranges str, Expression& ret_val) 
		{
			str = range_trim(str, ' ');

			parser->output_seq(str.flat());
			parser->next_level();

			vector<string_ranges> ignore = range_delimiter(str, parser->expr_open, parser->expr_close);

			is_first = true;
			make_return = false;

			for (auto it = ignore.begin(); it != ignore.end(); it += 2)
			{
				vector<string_ranges> ignore2 = range_delimiter(*it, "\"", "\"");
				for (auto it2 = ignore2.begin(); it2 != ignore2.end(); it2 += 2)
				{
					keywords = subchain(chain(*it2, range_until, " "), range_until, ",");
					for (current_keyword = keywords.begin(); current_keyword != keywords.end(); current_keyword++)
					{
						if (parse_keyword(*current_keyword)) return;
					}

					if (!next(it2)->empty())
					{
						std::cout << next(it2)->flat() << std::endl;
						string_ranges r2 = *next(it2);
						parser->output_seq(r2.flat());
						add_constant(make_const<string>(range_shave(r2).flat()));
					}
				}

				if (!next(it)->empty())
				{
					Expression sub = expression_parser->parse_sequence_next(*next(it));
					add_constant(sub);

				}
			}

			move_const_to_arg();

			/*for (auto it = functions.rbegin(); it != functions.rend(); it++)
			{
				size_t count = it->get_args_count();

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

			}*/
			if(!functions.empty())
			{
				parser->add_error(GLUU_ERROR_NOT_ENOUGH_ARGS, "ran out of arguments for function '" + functions.back().func_name + "' ", str.begin());
				return;
			}

			for (auto& c : constants)
				ret_val.recursive.push_back(c);

			if (make_return)
			{
				if (ret_val.recursive.size() > 0)
				{
					Expression copy = ret_val;
					ret_val = Expression(return_flags->back(), true);
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

		template<typename T>
		Expression make_const(const T& obj)
		{
			Expression constant(return_flags->back());
			constant.root = false;
			constant.constant = make_generic<T>(obj);
			return constant;
		}

		void parse(string_ranges str, Expression& ret_val)
		{
			this->ret_val = &ret_val;
			full = str;
			parse_expression(str, ret_val);
		}
	};
}