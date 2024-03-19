#pragma once
#include "GLUU_exprParser.h"

namespace GLUU {
	class LineParser
	{
		Parser* parser = nullptr;
		ExpressionParser* expression_parser = nullptr;

		queue<shared_ptr<bool>>* return_flags = nullptr;

		//vector<string_ranges>::iterator& current_

	public:
		LineParser(Parser* parser, ExpressionParser* expression_parser, queue<shared_ptr<bool>>* return_flags) : parser(parser), return_flags(return_flags), expression_parser(expression_parser) {}
		~LineParser() {};

		void parse_function_keyword(string_ranges kw, vector<Expression>& constants, vector<Expression>& functions, bool rev = false);

		bool parse_keyword(vector<string_ranges>::iterator& kw, vector<Expression>& constants, vector<Expression>& functions, bool& f, string_ranges full, size_t size, Expression& ret_val, vector<string_ranges>::iterator end, bool& make_return);

		void add_constant(vector<Expression>& constants, const Expression& constant);

		void parse_expression(string_ranges str, Expression& ret_val);

		template<typename T>
		Expression make_const(const T& obj)
		{
			Expression constant(return_flags->back());
			constant.root = false;
			constant.constant = make_generic<T>(obj);
			return constant;
		}

		void parse(string_ranges str, Expression& ret_val)
		{
			parse_expression(str, ret_val);
		}
	};
}