#pragma once
#include "StringRanges.h"
#include "ClassFactory.h"
#include <algorithm>

namespace GLUU {
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
}
