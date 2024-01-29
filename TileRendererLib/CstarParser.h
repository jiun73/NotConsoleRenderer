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

class CstarRows 
{
	vector<CstarRows> nested_rows;
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

	void parse_declaration(string_ranges dec)
	{
		vector<string> keywords = split_and_trim(dec);

		if (keywords.size() < 2) return; //error

		if (keywords.at(0) == "function")
		{

		}
		else
		{		
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

		for (size_t i = 0; i < keywords.size(); i++)
		{
			const string& current = keywords.at(i);

			if (current == "SIZE")
			{
				if (i == keywords.size() - 1) return CstarRows(); //error
				const string& next = keywords.at(i + 1);

				i++;
			}
		}
	}

	void parse_row(string_ranges row)
	{	
		string_ranges head = range_until(row, "{");
		row = { head.skip(), prev(row.end()) };

		if (row.empty()) return; //error

		std::cout << "-----------" << head.flat() << std::endl;
		std::cout << "-------------" << row.flat() << std::endl;
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

		vector<string_ranges> rows = chain(str, range_until, row_keyword);
		if (rows.empty()) return {}; //error

		get_declaractions(rows.at(0));

		for (auto it = rows.begin() + 1; it != rows.end(); it++)
		{
			vector<string_ranges> row_in = chain(*it, range_first_level, '{', '}');
			std::cout << row_in.size() << std::endl;

			if (row_in.size() < 1) return {};//error
			parse_row(row_in.at(0));
			if (row_in.size() == 2)
				get_declaractions(row_in.at(1));
		}
	}
};

//typedef CstartGlobalParser;
