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

class Cstar
{
	string head;
	vector<Cstar> recursive;

	GenericFunction* function = nullptr;
	shared_generic* constant = nullptr;

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
	CstarRows* current_row;
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

	Cstar parse_expression(string_ranges str)
	{
		
	}

	Cstar parse_sequence(string_ranges str)
	{
		Cstar ret;

		vector<string_ranges> sequence = range_delimiter(str, '<', '>');
		if (sequence.size() != 2) return; //error
		vector<string_ranges> strings = range_delimiter(str, '"', '"');
		vector<string_ranges> expressions;

		bool add = false;

		for (auto it = strings.begin(); it != strings.end(); it += 2)
		{

			vector<string_ranges> split = chain(*it, range_until, ";");
			for (auto& s : split)
			{
				if (add)
				{
					expressions.back() = {expressions.back().begin(), s.end()};
					add = false;
				}
				else
				{
					expressions.push_back(s);
				}
			}
			if (!next(it)->empty())
			{
				expressions.back() = { expressions.back().begin(), next(it)->end() };
				add = true;
			}
		}

		for (auto& e : expressions)
		{
			parse_expression(e);
		}
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
			std::cout << "made new variable " << keywords.at(0) << " " << keywords.at(1) << " in scope " << current_scope->name << std::endl;
			if (!current_scope->make(keywords.at(1), keywords.at(0))) return; //error		
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

		current_scope = row_obj.scope;
		current_row = &row_obj;

		parse_range(row, "COL", false);

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
