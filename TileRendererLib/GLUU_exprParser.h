#pragma once
#include "GLUU_parser.h"

namespace GLUU {
	class LineParser;

	class ExpressionParser
	{
		friend LineParser;

		Parser* parser = nullptr;

		queue<shared_ptr<bool>> return_flags;

		Expression parse_sequence_next(string_ranges str);

	public:
		ExpressionParser(Parser* parser) : parser(parser) {}
		~ExpressionParser() {}

		Expression parse_new_sequence(string& str)
		{
			variable_dictionnary()->enter_scope(parser->GLUU_scope);
			str += '\n';
			remove_all_range(str, "//", "\n", false);
			remove_all_range(str, "/*", "*/", true);
			change_whitespace_to_space(str);
			Expression e = parse_sequence_next(str);
			variable_dictionnary()->exit_scope();
			return e;
		}

		Expression parse(string_ranges str)
		{
			return_flags.push(make_shared<bool>());
			auto expr = parse_sequence_next(str);
			return_flags.pop();

			parser->output_errors();
			
			return expr;
		}
	};
}