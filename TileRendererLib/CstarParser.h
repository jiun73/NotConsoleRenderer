#pragma once
#include "Generics.h"
#include "FunctionGenerics.h"
#include "StringRanges.h"
#include "CommandDictionnary.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

class CstarParserError {};

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



struct Cstar
{
	shared_ptr<VariableRegistry> scope;
	vector<Cstar> recursive;

	bool root = true;
	bool func = false;
	shared_generic constant = nullptr;

	shared_generic evaluate() 
	{

	}
};

struct CstarRows 
{
	vector<CstarRows> nested_rows;
	shared_ptr<VariableRegistry> scope;
	size_t size = 1;
	bool condition = 1;
};

class CstarParser 
{
private:
	shared_ptr<VariableRegistry> current_scope;
	CstarRows* current_row = nullptr;
	CstarRows base_row;

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

	void parse_expression(string_ranges str, Cstar& ret)
	{
		str = range_trim(str, ' ');
		std::cout << "> " << str.flat() << std::endl;
		vector<string_ranges> ignore = range_delimiter(str, '<', '>');

		vector<Cstar> constants;
		vector<Cstar> functions;
		vector<Cstar> subs;

		bool f = true;
		for (auto it = ignore.begin(); it != ignore.end(); it+=2)
		{
			vector<string_ranges> keywords = subchain(chain(*it, range_until, " "), range_until, ",");
			for (auto& kw : keywords)
			{
				if (f && kw.flat() == "new")
				{
					get_declaractions(str);
					return;
				}

				if (is_var_name(kw))
				{
					std::cout << "\t> var " << kw.flat() << std::endl;

					Cstar constant;
					constant.root = false;
					shared_generic var = variable_dictionnary()->get(kw.flat());
					if (var == nullptr) { std::cout << "Invalid var name!" << std::endl;  return; }
					constant.constant = var;
					constants.push_back(constant);
				}
				else if (is_func_name(kw))
				{
					std::cout << "\t> func " << kw.flat() << std::endl;

					Cstar func;
					func.root = false;
					shared_generic var = variable_dictionnary()->get(kw.flat());	
					if (var == nullptr) { std::cout << "Invalid func name!" << std::endl; return; } // error
					if (var->identity() != typeid(GenericFunction)) return; //error
					func.func = true;
					func.constant = var;
					functions.push_back(func);
				}
				f = false;
			}
			
			if (!next(it)->empty())
			{
				std::cout << "\t> " << next(it)->flat() << std::endl;
				Cstar sub = parse_sequence_next(*next(it));
				subs.push_back(sub);
			}
		}


	}

	Cstar parse_sequence(string& str)
	{
		str += '\n';
		remove_all_range(str, "//", "\n", false);
		remove_all_range(str, "/*", "*/", true);
		change_whitespace_to_space(str);
		return parse_sequence_next(str);
	}

	Cstar parse_sequence_next(string_ranges str)
	{
		Cstar ret;
		ret.scope = std::make_shared< VariableRegistry>();
		ret.scope->name = "Expr scope";
		current_scope = ret.scope;
		variable_dictionnary()->enter_scope(current_scope);

		if (str.empty()) return {};

		str = range_trim(str, ' ');

		if (*str.begin() == '<' && *(str.end() - 1) == '>')
		{
			str.begin() += 1;
			str.end() -= 1;
		}

		std::cout << "full: " << str.flat() << std::endl;

		vector<string_ranges> subseq = range_delimiter(str, '<', '>');
		vector<string_ranges> expressions;
		for (auto it = subseq.begin(); it != subseq.end(); it += 2)
		{
			vector<string_ranges> escape_str = split_escape_delim(*it, '"', '"', ';');
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

		return ret;
	}

	void parse_declaration(string_ranges dec)
	{
		vector<string> keywords = split_and_trim(dec);

		if (keywords.size() < 2) return; //error

		if (keywords.at(0) == "function")
		{

		}
		else
		{	
			std::cout << "making... variable '" << keywords.at(0) << "' " << keywords.at(1) << " in scope " << current_scope->name << std::endl;
			if (!current_scope->make(keywords.at(1), keywords.at(0))) return; //error	
			std::cout << "made new variable " << keywords.at(0) << " " << keywords.at(1) << " in scope " << current_scope->name << std::endl;
		}	
	}

	void get_declaractions(string_ranges row)
	{
		string new_keyword = "new ";

		vector<string_ranges> declaractions = chain(row, range_until, new_keyword);
		if (!declaractions.empty())
			for (auto it = declaractions.begin() + 1; it != declaractions.end(); it++)
				parse_declaration(*it);
	}

	CstarRows parse_header(string_ranges head)
	{
		vector<string> keywords = split_and_trim(head);

		CstarRows row;
		row.scope = std::make_shared< VariableRegistry>();

		for (size_t i = 0; i < keywords.size(); i++)
		{
			const string& current = keywords.at(i);

			if (current == "SIZE")
			{
				if (i == keywords.size() - 1) return CstarRows(); //error
				const string& next = keywords.at(i + 1);

				if (isNum(next)) //TODO: change this
				{
					int i;
					strings::destringify(i, next);
					row.size = i;
				}

				i++;
			}
		}

		return row;
	}

	string_ranges extract_header(string_ranges& range)
	{
		string_ranges head = range_until(range, "{");
		range = { head.skip(), prev(range.end()) };
		return head;
	}

	void parse_column(string_ranges column)
	{
		string_ranges head = extract_header(column);

		std::cout << "-------------" << head.flat() << std::endl;

		parse_range(column, "ROW", true);
	}

	void parse_row(string_ranges row)
	{	
		string_ranges head = extract_header(row);

		if (row.empty()) return; //error

		std::cout << "-----------" << head.flat() << std::endl;
		//std::cout << "-------------" << row.flat() << std::endl;

		CstarRows row_obj = parse_header(head);
		shared_ptr<VariableRegistry> old_scope = current_scope;
		CstarRows* old_base = current_row;

		row_obj.scope->name = "Row scope";
		current_scope = row_obj.scope;
		current_row = &row_obj;
		variable_dictionnary()->enter_scope(current_scope);

		parse_range(row, "COL", false);

		variable_dictionnary()->exit_scope();
		current_row = old_base;
		current_scope = old_scope;

		current_row->nested_rows.push_back(row_obj);
	}

	void parse_range(string_ranges range, const string& keyword, bool row )
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
				if (row)
					parse_row(column);
				else
					parse_column(column);
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

		//vector<string_ranges> rows = chain(str, range_until, row_keyword);
		//if (rows.empty()) return {}; //error

		//get_declaractions(rows.at(0));

		//for (auto it = rows.begin() + 1; it != rows.end(); it++)
		//{
		//	vector<string_ranges> row_in = chain(*it, range_inside, '{', '}');
		//	std::cout << row_in.size() << std::endl;

		//	if (row_in.size() < 1) return {};//error
		//	parse_row(row_in.at(0));
		//	if (row_in.size() == 2)
		//		get_declaractions(row_in.at(1));
		//}

		return {};
	}
};

//typedef CstartGlobalParser;
