#pragma once
#include <string>
#include <vector>
#include <functional>

using std::string;
using std::vector;
using std::pair;
using std::function;

using std::prev;
using std::next;

typedef string::iterator string_iter;

typedef pair<string_iter, string_iter> string_pair;

class string_ranges
{
private:
	string_pair pair;
	string_iter skip_to;

public:
	string_ranges(string& str) : pair(str.begin(), str.end()), skip_to(str.end()) {}
	string_ranges(const string_iter& begin, const string_iter& end) : pair(begin, end), skip_to(end) {}
	string_ranges(const string_iter& begin, const string_iter& end, const string_iter& skip) : pair(begin, end), skip_to(skip) {}
	~string_ranges() {}

	string_iter& begin() { return pair.first; }
	string_iter& end() { return pair.second; }
	string_iter& skip() { return skip_to; }

	string flat() 
	{
		string ret(begin(), end());
		return ret;
	}

	bool empty() 
	{
		return pair.first == pair.second;
	}
};

inline string_ranges range_trim(string_ranges range, char c)
{
	string_iter iter_front = range.begin();
	while (iter_front != range.end() && *iter_front == c) iter_front++;

	string_iter iter_back = range.end();
	while (iter_back != iter_front && *prev(iter_back) == c) iter_back--;

	return { iter_front, iter_back  };
}

inline string_ranges range_until(string_ranges range, const string& str)
{
	if (str.empty()) return range;

	size_t str_i = 0;
	for (auto it = range.begin(); it != range.end(); it++)
	{
		if (*it == str.at(str_i))
		{
			str_i++;

			if (str_i == str.size())
				return { range.begin(), prev(it,str_i - 1), next(it) };
		}
		else
			str_i = 0;
	}

	return range;
}

inline string_ranges range_outside(string_ranges range, char open, char end)
{
	bool out = true;
	int level = 0;
	string_iter iter_open;
	for (auto it = range.begin(); it != range.end(); it++)
	{
		if (*it == end) level--;

		if (*it == end && !out && level <= 0) return { range.begin(), iter_open, next(it) };

		

		if (*it == open && out && level <= 0)
		{
			iter_open = it;
			out = false;
		}

		
		
		if (*it == open) level++;
		
		
	}

	if (out) return range;
	else return { range.begin(), iter_open, range.end() };
}

inline string_ranges range_inside(string_ranges range, char open, char end)
{
	int level = 0;
	bool in = false;
	for (auto it = range.begin(); it != range.end(); it++)
	{
		if (*it == end && in)
		{
			level--;
		}

		if (*it == end && in)
		{
			if (level == 0)
				return { range.begin(), it + 1 };

			continue;
		}

		if (*it == open)
		{
			level++; in = true; continue;
		}

		

		
	}

	return range;
}

inline vector<string_ranges> range_delimiter(string_ranges range, char open, char end)
{
	vector<string_ranges> ret;
	string_iter iter = range.begin();
	while (iter != range.end())
	{
		string_ranges out = range_outside({iter, range.end()}, open, end);
		string_ranges in = range_inside({ out.end(), range.end() }, open, end);
		ret.push_back(out);
		ret.push_back(in);
		iter = in.end();
	}
	return ret;
}



template<typename... Ts>
inline vector<string_ranges> chain(string_ranges range, string_ranges(range_func)(string_ranges, Ts...), std::remove_reference_t<Ts>... extras)
{
	vector<string_ranges> ret;

	auto current = range.begin();
	while (current != range.end())
	{
		auto sub_range = range_func({ current, range.end()}, extras...);

		ret.push_back(sub_range);

		current = sub_range.skip();
	}

	return ret;
}

template<typename... Ts>
inline vector<string_ranges> subchain(const vector<string_ranges>& ranges, string_ranges(range_func)(string_ranges, Ts...), std::remove_reference_t<Ts>... extras)
{
	vector<string_ranges> ret;
	for (auto& r : ranges)
	{
		vector<string_ranges> sub = chain(r, range_func, extras...);
		for (auto& s : sub)
			ret.push_back(s);
	}
	return ret;
}

inline vector<string_ranges> split_escape_delim(string_ranges range, char open, char end, char split)
{
	vector<string_ranges> strings = range_delimiter(range, open, end);
	vector<string_ranges> expressions;

	bool add = false;

	for (auto it = strings.begin(); it != strings.end(); it += 2)
	{

		vector<string_ranges> split = chain(*it, range_until, ";");
		for (auto& s : split)
		{
			if (add)
			{
				expressions.back() = { expressions.back().begin(), s.end() };
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

	return expressions;
}

//template<typename T>
//struct chain {};

//template<typename R, typename... Ts>
//struct chain <function<R(Ts...)>>
//{
//	static vector<string_range> get(function<R(Ts...)> range_func, string_range range, Ts... pass)
//	{ 
//		vector<string_range> ret;
//
//		auto current = range.begin();
//		while (current != range.end())
//		{
//			auto sub_range = range_func({ current, range.second }, pass...);
//
//			ret.push_back(sub_range);
//
//			current = sub_range.skip();
//		}
//
//		return ret;
//	}
//};