#pragma once
#include <vector>
#include <string>

using std::string;
using std::vector;

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