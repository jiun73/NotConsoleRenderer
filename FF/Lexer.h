#pragma once
#include <map>

namespace RAS
{
	using std::map;

	class Lexer
	{
	private:
		map<size_t, map<size_t, size_t>> aliases;

	public:

		void add_lexer(size_t key)
		{
			aliases.emplace(key, map<size_t, size_t>());
		}

		void add_alias(size_t lexer, size_t key, size_t value)
		{
			aliases.at(lexer).emplace(key, value);
		}

		size_t get_alias(size_t lexer, size_t key)
		{
			return aliases.at(lexer).at(key);
		}
	};
}