#pragma once
#include <vector>
#include <string>

using std::string;
using std::vector;

inline void remove_ws2(std::string& str)
{
	str.erase(std::remove_if(str.begin(), str.end(), [](int n)
		{
			return (n == '\n') || (n == '\t') || (n == '\v') || (n == '\f') || (n == '\r');
		}), str.end());
}

inline void change_whitespace_to_space(std::string& str)
{
	for (auto& c : str)
	{
		switch (c)
		{
		case '\n': 
		case '\t': 
		case '\v': 
		case '\f': 
		case '\r':
			c = ' ';
			break;
		default:
			break;
		}
	}
}

inline void remove_all_range(string& str, string sdelim, string edelim, bool remove_end)
{
	while (true)
	{
		size_t start = str.find(sdelim);

		if (start == str.npos)
			break;

		size_t end = str.find(edelim, start); //find the first endline after the comment
		str.erase(start, end - start + (remove_end * edelim.size())); //erase it
	}
}

inline vector<string> split(const string& str, const char delim)
{
	if (str.empty()) return {};

	vector<string> buffer;
	size_t last = 0;
	while (true)
	{
		size_t pos = str.find(delim, last);
		if (pos == str.npos)
		{
			buffer.push_back(str.substr(last));
			return buffer;
		}

		buffer.push_back(str.substr(last, pos - last));
		last = pos + 1;
	}
}

/*
* Remove all characters 'c' from the start and end of a string
*/
inline void trim(std::string& str, const char c)
{
	if (str.empty())
		return;

	while (!str.empty() && str.back() == c)
		str.pop_back();

	while (!str.empty() && str.front() == c)
		str.erase(str.begin());
}

#include <regex>

inline bool isNum(const std::string& s) { return std::regex_match(s, std::regex("-?[0-9]+")); }

inline int destringify(const string& str, int& target) 
{
	if (!isNum(str)) return false;
	target = std::stoi(str);
	return true;
}

inline void string_to_upper(string& str) 
{
	for (auto& c : str) c = std::toupper(c);
}

//TODO: fix this shit function
inline void split2(std::vector<std::string>& vec, std::string str, char delim = ' ', char d1 = '"', char d2 = '"') 
{
	std::string buf = str;

	size_t i = buf.find_first_of(delim);
	while (i != buf.npos)
	{
		if (buf.front() == d1)
		{
			size_t ss = buf.find_first_of(d2, 1);
			if (ss != buf.npos)
			{

				std::string ex = buf.substr(1, ss - 1);

				vec.push_back(ex);
				if (!buf.empty())
					buf.erase(0, ss + 1);

				i = buf.find_first_of(delim);

				continue;
			}
		}

		std::string ex = buf.substr(0, i);


		vec.push_back(ex);
		buf.erase(0, i + 1);
		i = buf.find_first_of(delim);
	}

	if (!buf.empty())
		if (buf.front() == d1)
		{
			if (buf.back() == d2)
				vec.push_back(buf.substr(1, buf.length() - 3));
		}
		else
			vec.push_back(buf);
}