#pragma once
#include <string>
#include <vector>
#include <memory>

namespace GLUU {
	using std::string;
	using std::pair;
	using std::vector;
	using std::shared_ptr;

	struct Element;
	class Parser;

	struct Widget
	{
		virtual pair<size_t, string> fetch_keyword() = 0;
		virtual void update(Element& graphic) = 0;
		virtual shared_ptr<Widget> make(vector<string_ranges>& args, Parser& parser) = 0;
	};
}
