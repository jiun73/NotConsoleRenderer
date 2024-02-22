#pragma once
#include "Generics.h"
#include "FunctionGenerics.h"
#include "StringRanges.h"
#include "CommandDictionnary.h"
#include "NotConsoleRenderer.h"

#include "GLUU_expr.h"
#include "GLUU_seqvar.h"
#include "GLUU_wint.h"


namespace GLUU {
	enum Errors 
	{
		GLUU_ERROR_INVALID_EXPRESSION_KEYWORD,
		GLUU_ERROR_
	};

	struct Element
	{
		vector<Element> nested;
		shared_ptr<VariableRegistry> scope;
		SeqVar<size_t> size = 10;
		SeqVar<bool> condition;
		SeqVar<bool> fit = false;
		Rect_d last_dest;

		bool is_row = false;

		int get_size_max()
		{
			int i = 0;
			for (auto& n : nested)
			{
				i += n.size();
			}

			return i;
		}

		double map_size(double sz, double max)
		{
			return sz * (size() / max);
		}

		void render(Rect_d dest)
		{
			if (is_row)
			{
				double pencil = dest.pos.x;
				for (auto& col : nested)
				{
					double sz;

					if (fit)
					{
						sz = col.map_size(dest.sz.x, get_size_max());
					}
					else
					{
						sz = col.size();
					}

					Rect_d sub = { {pencil, dest.pos.y},{sz, dest.sz.y} };
					draw_rect(sub);
					col.render(sub);
					pencil += sz;
				}
			}
			else
			{
				double pencil = dest.pos.y;
				for (auto& row : nested)
				{
					double sz;

					if (fit)
					{
						sz = row.map_size(dest.sz.x, get_size_max());
					}
					else
					{
						sz = row.size();
					}

					Rect_d sub = { {dest.pos.x, pencil},{dest.sz.x, sz} };
					draw_rect(sub);
					row.render(sub);
					pencil += sz;
				}
			}
			last_dest = dest;
		}

		shared_ptr < Widget> widget = nullptr;

		void update()
		{
			if (widget != nullptr)
				widget->update(*this);

			for (auto& n : nested)
			{
				n.update();
			}
		}
	};

#include <map>
	using std::map;
	using std::make_pair;

	struct Compiled
	{
		shared_ptr<VariableRegistry> compiled_scope;
		Element* current_row = nullptr;
		Element base_row;

		void render(Rect_d window) { base_row.render(window); base_row.update(); }
		void update()
		{
			//base_row.update();
		}
	};

	class Parser
	{
	private:
		shared_ptr<VariableRegistry> GLUU_scope;

		shared_ptr<VariableRegistry> current_scope;
		map <string, pair<size_t, function<void(Parser&, Element&, vector<string>)>>> keywords_func;
		map <string, shared_ptr<Widget>> widgets;

		shared_ptr<Compiled> graphics;

	public:
		const char row_open = '<';
		const char row_close = '>';
		const char expr_open = '{';
		const char expr_close = '}';

		shared_ptr<VariableRegistry> get_scope() { return GLUU_scope; }

		Parser()
		{
			GLUU_scope = variable_dictionnary()->make_new_scope("GLUU");

			keywords_func.emplace("SIZE", make_pair(1, [](Parser& parser, Element& gfx, vector<string> s)
				{
					std::cout << strings::stringify(s) << std::endl;
					gfx.size.set(s.at(0), parser);
					std::cout << gfx.size() << std::endl;
				}));

			keywords_func.emplace("IF", make_pair(1, [](Parser& parser, Element& gfx, vector<string> s)
				{
					std::cout << strings::stringify(s) << std::endl;
					gfx.condition.set(s.at(0), parser);
				}));

			keywords_func.emplace("FIT", make_pair(1, [](Parser& parser, Element& gfx, vector<string> s)
				{
					std::cout << strings::stringify(s) << std::endl;
					gfx.fit.set(s.at(0), parser);
					std::cout << gfx.fit() << std::endl;
				}));

			keywords_func.emplace("CFIT", make_pair(0, [](Parser& parser, Element& gfx, vector<string> s)
				{
					gfx.fit() = true;
				}));
		}
		~Parser() {}

		void register_class(shared_ptr <Widget> c)
		{
			widgets.emplace(c->fetch_keyword().second, c);
		}

		vector<string> split_and_trim(string_ranges str)
		{
			str = range_trim(str, ' ');

			vector<string_ranges> keywords_range = chain(str, range_until, " ");
			vector<string> keywords;

			for (auto kw : keywords_range)
			{
				kw = range_trim(kw, ' ');
				string flat = kw.flat();
				keywords.push_back(flat);
			}

			return keywords;
		}

		void parse_expression(string_ranges str, Expression& ret_val)
		{
			str = range_trim(str, ' ');
			std::cout << "> " << str.flat() << std::endl;
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
						if (f && kw->flat() == "new")
						{
							get_declaractions(str);
							return;
						}
						if (f && kw->flat() == "arg")
						{
							if (keywords.size() < 3) return; //error GLUU_ERROR_INVALID_ARG_FORMAT
							string type = next(kw)->flat();
							string name = next(kw, 2)->flat();
							ret_val.add_arg(name, type);
							std::cout << "'" << name << "' as " << type << std::endl;
							return;
						}
						if (f && kw->flat() == "arg*")
						{
							if (keywords.size() < 3) return; //error GLUU_ERROR_INVALID_ARG_FORMAT
							string type = next(kw)->flat();
							string name = next(kw, 2)->flat();
							ret_val.add_arg_ref(name, type);
							std::cout << "'" << name << "' as " << type << std::endl;
							return;
						}
						else if (kw->flat() == "this")
						{
							Expression constant;
							constant.root = false;
							constant.constant = std::make_shared<GenericRef<Expression>>(ret_val);
							constants.push_back(constant);
						}
						else if (is_char(*kw))
						{
							std::cout << "\t> ch " << kw->flat() << std::endl;
							kw->begin() += 1;
							kw->end() -= 1;
							Expression constant;
							constant.root = false;
							constant.constant = make_generic<char>(kw->flat().at(0));
							constants.push_back(constant);
						}
						else if (is_string(*kw))
						{
							std::cout << "\t> str " << kw->flat() << std::endl;
							kw->begin() += 1;
							kw->end() -= 1;
							Expression constant;
							constant.root = false;
							constant.constant = make_generic<string>(kw->flat());
							constants.push_back(constant);
						}
						else if (is_num(*kw))
						{
							Expression constant;
							constant.root = false;
							constant.constant = make_generic<int>();
							constant.constant->destringify(kw->flat());
							std::cout << "\t> num " << kw->flat() << " " << constant.constant->stringify() << std::endl;

							constants.push_back(constant);
						}
						else if (is_var_name(*kw))
						{
							std::cout << "\t> var " << kw->flat() << std::endl;

							Expression constant;
							constant.root = false;
							shared_generic var = variable_dictionnary()->get(kw->flat());
							if (var == nullptr) { std::cout << "Invalid var name!" << std::endl;  return; }
							constant.constant = var;
							constants.push_back(constant);
						}
						else if (is_func_name(*kw))
						{
							std::cout << "\t> func " << kw->flat() << std::endl;

							Expression func;
							func.root = false;
							shared_generic var = variable_dictionnary()->get(kw->flat());
							if (var == nullptr) { std::cout << "Invalid func name!" << std::endl; return; } // error GLUU_ERROR_INVALID_FUNCTION_NAME
							if (var->identity() == typeid(GenericFunction))
							{
								func.func = true;
								func.function = std::reinterpret_pointer_cast<GenericFunction>(var);
								func.func_name = kw->flat();
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
								return; // error GLUU_ERROR_INVALID_EXPRESSION_KEYWORD
							}

						}
						f = false;
					}

					if (!next(it2)->empty())
					{
						string_ranges r2 = *next(it2);
						std::cout << "\t> str " << r2.flat() << std::endl;
						r2.begin() += 1;
						r2.end() -= 1;
						Expression constant;
						constant.root = false;
						constant.constant = make_generic<string>(r2.flat());
						constants.push_back(constant);
					}
				}

				if (!next(it)->empty())
				{
					std::cout << "\t> " << next(it)->flat() << std::endl;
					Expression sub = parse_sequence_next(*next(it));
					constants.push_back(sub);
				}
			}

			for (auto it = functions.rbegin(); it != functions.rend(); it++)
			{
				size_t count = 0;



				if (it->func)
				{
					count = it->function->arg_count();
				}
				else
				{
					count = it->arg_count;
				}

				for (size_t i = 0; i < count; i++)
				{
					it->recursive.push_back(constants.back());
					constants.pop_back();
				}

				if (it->func)
				{ 
					//const auto& args = it->generic_to_args(it->get_args_from_recursive(true)); error
					//if (!it->function->args(args)) //check for overloads
					//{
					//	auto fn = variable_dictionnary()->get_fn(it->func_name, args); //error GLUU_ERROR_OVERLOAD_NOT_FOUND
					//	auto fn_type = std::reinterpret_pointer_cast<GenericFunction>(fn);
					//	it->function = fn_type;
					//}
				}

				constants.push_back(*it);

			}

			//for (auto& f : functions)
				//ret_val.recursive.push_back(f);

			for (auto& c : constants)
				ret_val.recursive.push_back(c);
		}

		Expression parse_new_sequence(string& str)
		{
			variable_dictionnary()->enter_scope(GLUU_scope);
			Expression e = parse_sequence(str);
			variable_dictionnary()->exit_scope();
			return e;
		}

		Expression parse_sequence(string& str)
		{
			str += '\n';
			remove_all_range(str, "//", "\n", false);
			remove_all_range(str, "/*", "*/", true);
			change_whitespace_to_space(str);
			return parse_sequence_next(str);
		}

		Expression parse_sequence_next(string_ranges str)
		{
			Expression ret;
			ret.scope = std::make_shared< VariableRegistry>();
			ret.scope->name = "Expr scope";
			shared_ptr<VariableRegistry> old_scope = current_scope;
			current_scope = ret.scope;
			variable_dictionnary()->enter_scope(current_scope);

			if (str.empty())
			{
				assert(false);
				return {};
			}

			str = range_trim(str, ' ');

			if (*str.begin() == expr_open && *(str.end() - 1) == expr_close)
			{
				str.begin() += 1;
				str.end() -= 1;
			}

			vector<string_ranges> subseq = range_delimiter(str, expr_open, expr_close);
			vector<string_ranges> expressions;
			for (auto it = subseq.begin(); it != subseq.end(); it += 2)
			{
				vector<string_ranges> escape_str = split_escape_delim(*it, '"', '"', ";");
				bool f = true;
				for (auto& e : escape_str)
				{
					if (f)
					{
						if (!expressions.empty())
							expressions.back() = { expressions.back().begin(), e.end() };
						else
							expressions.push_back(e);
						f = false;
					}
					else
						expressions.push_back(e);
				}
				if (!expressions.empty())
					expressions.back() = { expressions.back().begin(), next(it)->end() };
			}

			for (auto& e : expressions)
			{
				parse_expression(e, ret);
			}

			variable_dictionnary()->exit_scope();
			current_scope = old_scope;

			return ret;
		}

		void parse_declaration(string_ranges dec)
		{
			vector<string> keywords = split_and_trim(dec);

			if (keywords.size() < 2) return; //error GLUU_ERROR_INVALID_DECLARATION

			if (keywords.at(0) == "function" || keywords.at(0) == "init")
			{
				if (keywords.size() < 4) return;

				string name = keywords.at(1);
				size_t eq = dec.flat().find('=');
				string_ranges func = { dec.begin() + eq + 1, dec.end() };
				Expression expr = parse_sequence_next(func);
				current_scope->add(make_shared<GenericType<Expression>>(expr), name);

				if (keywords.at(0) == "init")
				{
					expr.evaluate();
				}

				std::cout << "Added new func as '" << name << "' in scope " << current_scope->name << std::endl;
			}
			else
			{
				if (!current_scope->make(keywords.at(1), keywords.at(0))) { std::cout << "Invalid variable type!" << std::endl; return; } //error GLUU_ERROR_INVALID_TYPE
				std::cout << "made new variable " << keywords.at(0) << " " << keywords.at(1) << " in scope " << current_scope->name << std::endl;
				if (keywords.size() > 3 && keywords.at(2) == "=")
				{
					current_scope->get(keywords.at(1))->destringify(keywords.at(3));
				}
			}
		}

		void get_declaractions(string_ranges row)
		{
			string new_keyword = "new ";
			vector<string_ranges> declaractions = split_escape_delim(row, expr_open, expr_close, new_keyword);
			if (!declaractions.empty())
				for (auto it = declaractions.begin() + 1; it != declaractions.end(); it++)
				{
					parse_declaration(*it);
				}
		}

		Element parse_header(string_ranges head)
		{
			head = range_trim(head, ' ');
			vector<string_ranges> keywords = split_and_delim(head, expr_open, expr_close, " ");

			Element row;
			row.scope = std::make_shared< VariableRegistry>();

			for (size_t i = 0; i < keywords.size(); i++)
			{
				string current = range_trim(keywords.at(i), ' ').flat();

				std::cout << " param: " << current << std::endl; 

				if (current.empty()) continue; //error GLUU_ERROR_INVALID_ROW_PARAM

				if (widgets.count(current))
				{
					size_t p = widgets.at(current)->fetch_keyword().first;
					vector<string> args;
					for (size_t y = 0; y < p; y++)
					{
						i++;
						if (i >= keywords.size()) {
							assert(false);
							return row;
							}//error GLUU_ERROR_MISSING_ARGS

						string current = range_trim(keywords.at(i), ' ').flat();
						if (current.empty()) {
							y--;  continue;
						}
						args.push_back(current);
					}

					row.widget = widgets.at(current)->make(args, *this);
				}
				else if (keywords_func.count(current))
				{
					auto& p = keywords_func.at(current);
					vector<string> args;
					for (size_t y = 0; y < p.first; y++)
					{
						i++;
						string current = range_trim(keywords.at(i), ' ').flat();
						if (current.empty()) {
							y--;  continue;
						}
						args.push_back(current);
					}
					p.second(*this, row, args);
				}
				else
				{
					//std::cout << "Invalid param: " << current << std::endl; //error
				}
			}

			return row;
		}

		string_ranges extract_header(string_ranges& range)
		{
			string_ranges head = range_until(range, "<");
			range = { head.skip(), prev(range.end()) };
			return head;
		}

		void parse_graphic(string_ranges row, bool is_row)
		{
			string_ranges head = extract_header(row);

			if (row.empty()) return;

			std::cout << "-----------" << head.flat() << std::endl;

			Element row_obj = parse_header(head);
			shared_ptr<VariableRegistry> old_scope = current_scope;
			Element* old_base = graphics->current_row;

			row_obj.is_row = is_row;

			row_obj.scope->name = "Row scope";
			current_scope = row_obj.scope;
			graphics->current_row = &row_obj;
			variable_dictionnary()->enter_scope(current_scope);

			parse_range(row, is_row ? "COL" : "ROW", !is_row);

			variable_dictionnary()->exit_scope();
			graphics->current_row = old_base;
			current_scope = old_scope;

			graphics->current_row->nested.push_back(row_obj);
		}

		void parse_range(string_ranges range, const string& keyword, bool row)
		{
			vector<string_ranges> sub_rows = range_delimiter(range, row_open, row_close);

			for (auto it = sub_rows.begin(); it != sub_rows.end(); it += 2)
			{
				auto in = next(it);
				vector<string_ranges> columns = chain(*it, range_until, keyword);

				if (columns.empty()) return; 
				get_declaractions(columns.front());
				if (columns.size() > 1)
				{
					string_ranges column = { columns.at(1).begin(), in->end() };
					parse_graphic(column, row);
				}
			}
		}

		shared_ptr<Compiled> parse(string& str)
		{
			graphics = make_shared<Compiled>();
			//graphics->widgets = widgets;
			
			graphics->compiled_scope = variable_dictionnary()->make_temporary_scope("Global expression scope");
			current_scope = graphics->compiled_scope;

			variable_dictionnary()->enter_scope(GLUU_scope);
			variable_dictionnary()->enter_scope(current_scope);

			graphics->current_row = &graphics->base_row;
			string row_keyword = "ROW";
			str += '\n';
			remove_all_range(str, "//", "\n", false);
			remove_all_range(str, "/*", "*/", true);
			change_whitespace_to_space(str);

			parse_range(str, row_keyword, true);

			variable_dictionnary()->exit_scope();
			variable_dictionnary()->exit_scope();

			return graphics;
		}

		void render(Rect_d windowSize)
		{
			graphics->base_row.render(windowSize);
		}

		
	};

	typedef Singleton<Parser> Global;
}
