#include "pch.h"
#include "GLUU_exprParser.h"
#include "GLUU_lineParser.h"

namespace GLUU {
	Expression ExpressionParser::parse_sequence_next(string_ranges str)
	{
		Expression ret(return_flags.back());
		ret.root = true;
		ret.scope = std::make_shared< VariableRegistry>();
		ret.scope->name = "Expr scope";
		shared_ptr<VariableRegistry> old_scope = parser->current_scope;
		parser->current_scope = ret.scope;
		variable_dictionnary()->enter_scope(parser->current_scope);

		if (str.empty())
		{
			parser->add_error(GLUU_ERROR_EMPTY_SEQUENCE, "Trying to parse an empty sequence (why?)", str.begin());
			return Expression(return_flags.back());
		}

		str = range_trim(str, ' ');

		if (*str.begin() == parser->expr_open.at(0) && *(str.end() - 1) == parser->expr_close.at(0))
		{
			str.begin() += 1;
			str.end() -= 1;
		}

		vector<string_ranges> subseq = range_delimiter(str, parser->expr_open, parser->expr_close);
		vector<string_ranges> expressions;
		for (auto it = subseq.begin(); it != subseq.end(); it += 2)
		{
			vector<string_ranges> escape_str = split_escape_delim(*it, "\"", "\"", ";");
			bool f = true;
			for (auto& e : escape_str)
			{
				if (f)
				{
					if (!expressions.empty())
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
			LineParser line_parser(parser, this, &return_flags);
			line_parser.parse(e, ret);
		}

		variable_dictionnary()->exit_scope();
		parser->current_scope = old_scope;

		return ret;
	}
}