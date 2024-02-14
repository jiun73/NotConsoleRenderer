#pragma once
#include "Generics.h"
#include "FunctionGenerics.h"
#include "StringRanges.h"
#include "CommandDictionnary.h"
#include "TileRenderer.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

class CstarParserError {};

inline bool is_string(string_ranges range)
{
	if (range.empty()) return false;
	if (next(range.begin()) == range.end()) return false;

	return (*range.begin() == '"' && *prev(range.end()) == '"');
}

inline bool is_char(string_ranges range)
{
	if (range.empty()) return false;
	if (next(range.begin()) == range.end()) return false;

	return (*range.begin() == '\'' && *prev(range.end()) == '\'');
}

inline bool is_num(string_ranges range)
{
	return std::all_of(range.begin(), range.end(), isdigit);
}

inline bool is_keyword(string_ranges range)
{
	string flat = range.flat();
	if (flat != "new") return true;
	if (ClassFactory::get()->has(flat)) return true;
	return false;
}

inline bool is_var_name(string_ranges range)
{
	if (range.empty()) return false;
	//if (is_keyword(range)) return false;

	return std::all_of(range.begin(), range.end(), [](char c) { return isalnum(c) || c == '_'; }) && !std::all_of(range.begin(), range.end(), isdigit) && !isdigit(*range.begin());
}

inline bool is_func_name(string_ranges range)
{
	if (range.empty()) return false;
	char beg = *range.begin();
	if (!isalnum(beg) && beg != '_') return true;
	return false;
}



struct GLUU
{
	shared_ptr<VariableRegistry> scope;
	vector<GLUU> recursive;

	bool root = true;
	bool func = false;
	bool user_func = false;
	shared_generic constant = nullptr;
	shared_ptr<GenericFunction> function = nullptr;

	vector<string> args_name;
	size_t arg_count = 0;

	void add_arg(const string& str, const string& type)
	{
		scope->add(ClassFactory::get()->make(type), str);
		args_name.push_back(str);
	}

	void set_args(vector<shared_generic>& args)
	{
		size_t i = 0;
		for (auto& name : args_name)
		{
			scope->add(args.at(i), name);
			//*(scope->get(name)) = *args.at(i);
			//scope->get(name).swap(args.at(i));
			scope->get(name)->set(args.at(i));
			i++;
		}
	}

	vector<shared_generic> get_args_from_recursive(bool func_check = false)
	{
		vector<shared_generic> args;
		size_t i = 0;
		for (auto& r : recursive)
		{
			if (func_check && function->args_type(i) == typeid(GLUU&) )
			{
				shared_generic rec = make_shared<GenericRef<GLUU>>(r);
				args.push_back(rec);
			}
			else
			{
				args.push_back(r.evaluate());
			}
			i++;
		}
		return args;
	}

	shared_generic evaluate() 
	{
		if (root)
		{
			vector<shared_generic> ret;

			for (auto& r : recursive)
			{
				shared_generic eval = r.evaluate();
				if(eval != nullptr)
					ret.push_back(eval);
			}

			if (ret.size() == 0) { return nullptr; }
			if (ret.size() == 1) { return ret.at(0); }

			string collect;

			for (auto& r : ret)
			{
				collect += r->stringify();
			}

			shared_generic str_type = make_generic<string>(collect);
			return str_type;
		}
		else
		{
			if (func)
			{
				vector<GenericArgument> args;

				for (auto& arg : get_args_from_recursive(true))
				{
					args.push_back(arg);
				}

				function->args(args);
				return function->call();
			}
			else if (user_func)
			{
				GLUU& star = *(GLUU*)(constant->raw_bytes());

				if (star.args_name.size() > 0)
				{
					vector<shared_generic> args = get_args_from_recursive();
					star.set_args(args);
				}

				return star.evaluate();
			}
			else
			{
				return constant;
			}
		}
	}
};

class GLUUParser;

template<typename T>
class CstarVar
{
private:
	shared_generic generic = nullptr;
	GLUU seq;

	bool is_seq = false;

public:
	CstarVar() {}
	~CstarVar() {}

	void set(const string& str,  GLUUParser& parser);
	shared_generic get_gen() { return generic; }

	T& get() 
	{
		if (generic == nullptr)
		{
			generic = make_generic<T>();
		}
		if (!is_seq)
			return *(T*)generic->raw_bytes();
		else
			return *(T*)seq.evaluate()->raw_bytes();
	}

	operator T()
	{
		return get();
	}

	T& operator->()
	{
		return get();
	}

	T& operator()() 
	{
		return get();
	}
};

struct GLUUGraphics
{
	vector<GLUUGraphics> nested;
	shared_ptr<VariableRegistry> scope;
	CstarVar<size_t> size;
	CstarVar<bool> condition;

	bool is_row = false;

	void render(Rect dest)
	{
		

		if (is_row)
		{
			int pencil = dest.pos.x;
			for (auto& col : nested)
			{
				int sz = 10;
				Rect sub = { {pencil, dest.pos.y},{sz, dest.sz.y} };
				draw_rect(sub);
				col.render(sub);
				pencil += sz;
			}
		}
		else
		{
			int pencil = dest.pos.y;
			for (auto& row : nested)
			{
				Rect sub = { {dest.pos.x, pencil},{dest.sz.x, 10} };
				draw_rect(sub);
				row.render(sub);
				pencil += 10;
			}
		}
	}
};

class GLUUParser 
{
private:
	shared_ptr<VariableRegistry> current_scope;
	GLUUGraphics* current_row = nullptr;
	GLUUGraphics base_row;

public:
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

	void parse_expression(string_ranges str, GLUU& ret_val)
	{
		str = range_trim(str, ' ');
		std::cout << "> " << str.flat() << std::endl;
		vector<string_ranges> ignore = range_delimiter(str, '<', '>');

		vector<GLUU> constants;
		vector<GLUU> functions;

		bool f = true;
		for (auto it = ignore.begin(); it != ignore.end(); it+=2)
		{
			vector<string_ranges> keywords = subchain(chain(*it, range_until, " "), range_until, ",");
			for (auto kw = keywords.begin(); kw != keywords.end(); kw++)
			{
				if (f && kw->flat() == "new")
				{
					get_declaractions(str);	
					return;
				}
				if (f && kw->flat() == "arg")
				{
					if (keywords.size() < 3) return; //error
					string type = next(kw)->flat();
					string name = next(kw, 2)->flat();
					ret_val.add_arg(name, type);
					std::cout << "'" << name  << "' as " << type << std::endl;
					return;
				}
				else if (kw->flat() == "this")
				{
					GLUU constant;
					constant.root = false;
					constant.constant = std::make_shared<GenericRef<GLUU>>(ret_val);
					constants.push_back(constant);
				}
				else if (is_char(*kw))
				{
					std::cout << "\t> ch " << kw->flat() << std::endl;
					kw->begin() += 1;
					kw->end() -= 1;
					GLUU constant;
					constant.root = false;
					constant.constant = make_generic<char>(kw->flat().at(0));
					constants.push_back(constant);
				}
				else if (is_string(*kw))
				{
					std::cout << "\t> str " << kw->flat() << std::endl;
					kw->begin() += 1;
					kw->end() -= 1;
					GLUU constant;
					constant.root = false;
					constant.constant = make_generic<string>(kw->flat());
					constants.push_back(constant);
				}
				else if (is_num(*kw))
				{
					GLUU constant;
					constant.root = false;
					constant.constant = make_generic<int>();
					constant.constant->destringify(kw->flat());
					std::cout << "\t> num " << kw->flat() << " " << constant.constant->stringify() << std::endl;
					
					constants.push_back(constant);
				}
				else if (is_var_name(*kw))
				{
					std::cout << "\t> var " << kw->flat() << std::endl;

					GLUU constant;
					constant.root = false;
					shared_generic var = variable_dictionnary()->get(kw->flat());
					if (var == nullptr) { std::cout << "Invalid var name!" << std::endl;  return; }
					constant.constant = var;
					constants.push_back(constant);
				}
				else if (is_func_name(*kw))
				{
					std::cout << "\t> func " << kw->flat() << std::endl;

					GLUU func;
					func.root = false;
					shared_generic var = variable_dictionnary()->get(kw->flat());
					if (var == nullptr) { std::cout << "Invalid func name!" << std::endl; return; } // error
					if (var->identity() == typeid(GenericFunction))
					{
						func.func = true;
						func.function = std::reinterpret_pointer_cast<GenericFunction>(var);
						if (func.function->arg_count() > 0)
							functions.push_back(func);
						else
							constants.push_back(func);
					}
					else if (var->type() == typeid(GLUU))
					{
						GLUU& star = *(GLUU*)var->raw_bytes();
						func.constant = var;
						func.user_func = true;
						func.arg_count = star.args_name.size();

						if(star.args_name.size() == 0)
							constants.push_back(func);
						else
							functions.push_back(func);
					}
					else
					{
						return; // error
					}
					
				}
				f = false;
			}
			
			if (!next(it)->empty())
			{
				std::cout << "\t> " << next(it)->flat() << std::endl;
				GLUU sub = parse_sequence_next(*next(it));
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

			constants.push_back(*it);

		}

		//for (auto& f : functions)
			//ret_val.recursive.push_back(f);

		for (auto& c : constants)
			ret_val.recursive.push_back(c);
	}

	GLUU parse_sequence(string& str)
	{
		str += '\n';
		remove_all_range(str, "//", "\n", false);
		remove_all_range(str, "/*", "*/", true);
		change_whitespace_to_space(str);
		return parse_sequence_next(str);
	}

	GLUU parse_sequence_next(string_ranges str)
	{
		GLUU ret;
		ret.scope = std::make_shared< VariableRegistry>();
		ret.scope->name = "Expr scope";
		shared_ptr<VariableRegistry> old_scope = current_scope;
		current_scope = ret.scope;
		variable_dictionnary()->enter_scope(current_scope);

		if (str.empty()) return {};

		str = range_trim(str, ' ');

		if (*str.begin() == '<' && *(str.end() - 1) == '>')
		{
			str.begin() += 1;
			str.end() -= 1;
		}

		vector<string_ranges> subseq = range_delimiter(str, '<', '>');
		vector<string_ranges> expressions;
		for (auto it = subseq.begin(); it != subseq.end(); it += 2)
		{
			vector<string_ranges> escape_str = split_escape_delim(*it, '"', '"', ";");
			bool f = true;
			for (auto& e : escape_str)
			{
				if (f)
				{
					if(!expressions.empty())
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

		if (keywords.size() < 2) return; //error

		if (keywords.at(0) == "function" || keywords.at(0) == "init")
		{
			if (keywords.size() < 4) return;

			string name = keywords.at(1);
			size_t eq = dec.flat().find('=');
			string_ranges func = { dec.begin() + eq + 1, dec.end()};
			GLUU expr = parse_sequence_next(func);
			current_scope->add(make_shared<GenericType<GLUU>>(expr), name);

			if (keywords.at(0) == "init")
			{
				expr.evaluate();
			}

			std::cout << "Added new func as '" << name << "' in scope " << current_scope->name << std::endl;
		}
		else
		{	
			if (!current_scope->make(keywords.at(1), keywords.at(0))) { std::cout << "Invalid variable type!" << std::endl; return; } //error	
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
		vector<string_ranges> declaractions = split_escape_delim(row, '<', '>', new_keyword);
		if (!declaractions.empty())
			for (auto it = declaractions.begin() + 1; it != declaractions.end(); it++)
			{
				parse_declaration(*it);
			}
	}

	GLUUGraphics parse_header(string_ranges head)
	{
		vector<string_ranges> keywords = split_escape_delim(head, '<', '>', " ");

		GLUUGraphics row;
		row.scope = std::make_shared< VariableRegistry>();

		for (size_t i = 0; i < keywords.size(); i++)
		{
			string_ranges current = keywords.at(i);

			if (i == keywords.size() - 1) break; //error
			string_ranges next = keywords.at(i + 1);

			range_trim(current, ' ');
			range_trim(next, ' ');

			if (next.flat() == "AS")
			{
				if (i == keywords.size() - 2) break; //error
				string_ranges next_2 = keywords.at(i + 2);
				range_trim(next_2, ' ');
				row.size.get();
				current_scope->add(row.size.get_gen(), next_2.flat());
				i++;
			}
			else
			{
				row.size.set(next.flat(), *this);
				
			}

			i++;
		}

		return row;
	}

	string_ranges extract_header(string_ranges& range)
	{
		string_ranges head = range_until(range, "{");
		range = { head.skip(), prev(range.end()) };
		return head;
	}

	void parse_graphic(string_ranges row, bool is_row)
	{	
		string_ranges head = extract_header(row);

		if (row.empty()) return; //error

		std::cout << "-----------" << head.flat() << std::endl;

		GLUUGraphics row_obj = parse_header(head);
		shared_ptr<VariableRegistry> old_scope = current_scope;
		GLUUGraphics* old_base = current_row;

		row_obj.is_row = is_row;

		row_obj.scope->name = "Row scope";
		current_scope = row_obj.scope;
		current_row = &row_obj;
		variable_dictionnary()->enter_scope(current_scope);

		parse_range(row, is_row ? "COL" : "ROW", !is_row);

		variable_dictionnary()->exit_scope();
		current_row = old_base;
		current_scope = old_scope;

		current_row->nested.push_back(row_obj);
	}

	void parse_range(string_ranges range, const string& keyword, bool row)
	{
		vector<string_ranges> sub_rows = range_delimiter(range, '{', '}');

		for (auto it = sub_rows.begin(); it != sub_rows.end(); it += 2)
		{
			auto in = next(it);
			vector<string_ranges> columns = chain(*it, range_until, keyword);

			if (columns.empty()) return; // error
			get_declaractions(columns.front());
			if (columns.size() > 1)
			{
				string_ranges column = { columns.at(1).begin(), in->end() };
				parse_graphic(column, row);
			}
		}
	}

	CstarParserError parse(string& str)
	{
		current_scope = variable_dictionnary()->global();
		current_row = &base_row;
		string row_keyword = "ROW";
		str += '\n';
		remove_all_range(str, "//", "\n", false);
		remove_all_range(str, "/*", "*/", true);
		change_whitespace_to_space(str);

		parse_range(str, row_keyword, true);

		return {};
	}

	void render(Rect windowSize)
	{
		base_row.render(windowSize);
	}
};

//typedef CstartGlobalParser;

template<typename T>
inline void CstarVar<T>::set(const string& str, GLUUParser& parser)
{
	if (!str.empty() && str.begin() != str.end())
	{
		if (*str.begin() == '<' && *prev(str.end()) == '>')
		{
			string strcop = str;
			seq = parser.parse_sequence(strcop);
			is_seq = true;
			return;
		}
	}

	generic = make_generic<T>();
	generic->destringify(str);
}
