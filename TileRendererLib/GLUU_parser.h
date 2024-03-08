#pragma once
#include "Generics.h"
#include "FunctionGenerics.h"
#include "StringRanges.h"
#include "CommandDictionnary.h"
#include "NotConsoleRenderer.h"

#include "GLUU_expr.h"
#include "GLUU_seqvar.h"
#include "GLUU_wint.h"

#include <typeindex>
#include <queue>

namespace GLUU {
	using std::queue;

	enum Errors 
	{
		GLUU_ERROR_INVALID_EXPRESSION_KEYWORD,
		GLUU_ERROR_INVALID_ARG_FORMAT,
		GLUU_ERROR_INVALID_FUNCTION_NAME,
		GLUU_ERROR_INVALID_VARIABLE_NAME,
		GLUU_ERROR_NOT_ENOUGH_ARGS,
		GLUU_ERROR_EMPTY_SEQUENCE,
		GLUU_ERROR_INVALID_DECLARATION,
		GLUU_ERROR_INVALID_FUNCTION_DECLARATION,
		GLUU_ERROR_KEYWORD_MISSING_ARGS,
		GLUU_ERROR_INVALID_KEYWORD,
		GLUU_ERROR_INVALID_TYPE,
		GLUU_ERROR_INVALID_ROW,
		GLUU_ERROR_INVALID_STRING_TRANSLATION,
		GLUU_ERROR_INVALID_RETURN,
		GLUU_ERROR_INVALID_MEMBER_EXPRESSION
	};

	struct Errorinfo 
	{
		size_t line;
		size_t ch;
		size_t tot_ch;
		string message;
		Errors code;
	};

	struct Element
	{
		vector<Element> nested;
		shared_ptr<VariableRegistry> scope;
		SeqVar<size_t> size = 10;
		SeqVar<bool> condition = true;
		SeqVar<bool> fit = false;
		Rect_d last_dest;

		bool is_row = false;

		int get_size_max()
		{
			int i = 0;
			for (auto& n : nested)
			{
				if (n.condition())
					i += (int)n.size();
			}

			return i;
		}

		double map_size(double sz, double max)
		{
			return sz * (size() / max);
		}

		double get_size(const Rect_d& dest, Element& other, bool vert)
		{
			if (!other.condition()) return 0;

			if (fit)
			{
				if (!vert)
					return other.map_size(dest.sz.x, get_size_max());
				else
					return other.map_size(dest.sz.y, get_size_max());
			}
			else
			{
				return (double)other.size();
			}

		}

		void render(Rect_d dest)
		{
			if (!condition()) return;

			if (is_row)
			{
				double pencil = dest.pos.x;
				for (auto& col : nested)
				{
					double sz = get_size(dest, col, false);

					Rect_d sub = { {pencil, dest.pos.y},{sz, dest.sz.y} };
					draw_rect(sub);
					col.render(sub);
					pencil += sz;
				}
			}
			else
			{
				double pencil = dest.pos.y;
				for (auto& row : nested)
				{
					double sz = get_size(dest, row, true);;

					Rect_d sub = { {dest.pos.x, pencil},{dest.sz.x, sz} };
					draw_rect(sub);
					row.render(sub);
					pencil += sz;
				}
			}
			last_dest = dest;
		}

		shared_ptr < Widget> widget = nullptr;

		void update()
		{
			if (!condition()) return;

			if (widget != nullptr)
				widget->update(*this);

			for (auto& n : nested)
			{
				n.update();
			}
		}

		void update_l2() 
		{
			if (!condition()) return;

			if (widget != nullptr)
				widget->update_l2(*this);

			for (auto& n : nested)
			{
				n.update_l2();
			}
		}
	};

#include <map>
	using ::std::map;
	using ::std::make_pair;
	using ::std::type_index;

	struct Compiled
	{
		shared_ptr<VariableRegistry> compiled_scope;
		vector<Expression> callbacks;
		Element* current_row = nullptr;
		Element base_row;

		void render(Rect_d window) 
		{ 
			for (auto& c : callbacks)
			{
				c.evaluate();
			}
			base_row.render(window); 
			base_row.update();
			base_row.update_l2();
		}
		void update()
		{
			//base_row.update();
		}
	};

	class Parser
	{
	private:
		shared_ptr<VariableRegistry> GLUU_scope;

		shared_ptr<VariableRegistry> current_scope;
		map <string, pair<size_t, function<void(Parser&, Element&, vector<string_ranges>)>>> keywords_func;
		map <string, shared_ptr<Widget>> widgets;

		shared_ptr<Compiled> graphics;

		vector<Errorinfo> errors;
		queue<shared_ptr<bool>> return_flags;		

		string::iterator source_begin;
		map<size_t, size_t> lines;
		size_t line_cntr = 0;
		size_t seq_level = 0;
		size_t row_level = 0;

		unordered_map<type_index, Inspector> inspectors;

	public:
		const char row_open = '<';
		const char row_close = '>';
		const char expr_open = '{';
		const char expr_close = '}';

		shared_ptr<VariableRegistry> get_scope() { return GLUU_scope; }

		Parser();
		~Parser() {}

		template<typename T>
		void register_inspector(function<shared_generic(shared_generic, const string&)> inspector)
		{
			Inspector inspect;
			inspect.inspect = inspector;
			inspectors.emplace(typeid(T), inspect);
		}

		void next_level()
		{
			if (seq_level != 0)
			{
				for (size_t i = 0; i < row_level + 1; i++)
				{
					std::cout << "\t";
				}

				for (size_t i = 0; i < seq_level - 1; i++)
				{
					//(char)(179) <<
					std::cout << " ";
				}
				std::cout << (char)(192) << (char)(191) << std::endl;
			}
			seq_level++;
		}

		void prev_level()
		{
			seq_level--;
		}

		void output_seq(const string& str, bool start = false)
		{
			if (seq_level == 0) 
			{
				std::cout << std::endl;
				for (size_t i = 0; i < row_level + 1; i++)
				{
					std::cout << "\t";
				}
				std::cout<< str << std::endl;
			}
			else
			{

				for (size_t i = 0; i < row_level + 1; i++)
				{
					std::cout << "\t";
				}

				for (size_t i = 0; i < seq_level - 1; i++)
				{
					//(char)(179) <<
					std::cout << " ";
				}
				if (start)
					std::cout << (char)(192);
				else
					std::cout << (char)(179);
				//std::cout << (char)(192);

				
				std::cout << "  " << str << std::endl;
			}
		}

		template<typename T>
		Expression make_const(const T& obj)
		{
			Expression constant(return_flags.back());
			constant.root = false;
			constant.constant = make_generic<T>(obj);
			return constant;
		}

		void add_error(Errors code, const string& message, string::iterator it);

		void register_class(shared_ptr <Widget> c);

		void parse_function_keyword(string_ranges kw, vector<Expression>& constants, vector<Expression>& functions, bool rev);

		bool parse_keyword(vector<string_ranges>::iterator& kw, vector<Expression>& constants, vector<Expression>& functions, bool& f, string_ranges full, size_t size, Expression& ret_val, vector<string_ranges>::iterator end, bool& make_return);

		void parse_expression(string_ranges str, Expression& ret_val);

		Expression parse_new_sequence(string& str)
		{
			source_begin = str.begin();
			variable_dictionnary()->enter_scope(GLUU_scope);
			Expression e = parse_sequence(str);
			variable_dictionnary()->exit_scope();
			return e;
		}

		Expression parse_sequence(string& str)
		{
			str += '\n';
			remove_all_range(str, "//", "\n", false);
			remove_all_range(str, "/*", "*/", true);
			change_whitespace_to_space(str);
			return parse_sequence_next(str);
		}

		Expression parse_sequence_base(string_ranges str)
		{
			return_flags.push(make_shared<bool>());
			auto expr = parse_sequence_next(str);
			return_flags.pop();
			return expr;
		}

		Expression parse_sequence_next(string_ranges str)
		{
			Expression ret(return_flags.back());
			ret.scope = std::make_shared< VariableRegistry>();
			ret.scope->name = "Expr scope";
			shared_ptr<VariableRegistry> old_scope = current_scope;
			current_scope = ret.scope;
			variable_dictionnary()->enter_scope(current_scope);

			if (str.empty())
			{
				add_error(GLUU_ERROR_EMPTY_SEQUENCE, "Trying to parse an empty sequence (why?)", str.begin());
				return Expression(return_flags.back());
			}

			str = range_trim(str, ' ');

			if (*str.begin() == expr_open && *(str.end() - 1) == expr_close)
			{
				str.begin() += 1;
				str.end() -= 1;
			}

			vector<string_ranges> subseq = range_delimiter(str, expr_open, expr_close);
			vector<string_ranges> expressions;
			for (auto it = subseq.begin(); it != subseq.end(); it += 2)
			{
				vector<string_ranges> escape_str = split_escape_delim(*it, '"', '"', ";");
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
				parse_expression(e, ret);
			}

			variable_dictionnary()->exit_scope();
			current_scope = old_scope;

			
			return ret;
		}

		void parse_declaration(string_ranges dec)
		{
			vector<string_ranges> keywords = split_and_trim(dec);

			if (keywords.size() < 1) { add_error(GLUU_ERROR_INVALID_DECLARATION, "'new' found with no declaration after", dec.begin()); return; } //error GLUU_ERROR_INVALID_DECLARATION
			if (keywords.size() < 1) { add_error(GLUU_ERROR_INVALID_DECLARATION, "'new' with no name for declaration", dec.begin()); return; } //error GLUU_ERROR_INVALID_DECLARATION

			if (keywords.at(0).flat() == "function" || keywords.at(0).flat() == "init" || keywords.at(0).flat() == "callback")
			{
				if (keywords.size() < 4) { add_error(GLUU_ERROR_INVALID_FUNCTION_DECLARATION, "Invalid function declaraction (new function = [expr])", dec.begin()); return; } //error GLUU_ERROR_INVALID_FUNCTION_DECLARATION

				string name = keywords.at(1).flat();
				size_t eq = dec.flat().find('=');
				string_ranges func = { dec.begin() + eq + 1, dec.end() };
				Expression expr = parse_sequence_base(func);
				current_scope->add(make_shared<GenericType<Expression>>(expr), name);

				if (keywords.at(0).flat() == "init")
				{
					expr.evaluate();
				}

				if (keywords.at(0).flat() == "callback")
				{
					graphics->callbacks.push_back(expr);
				}

				std::cout << "Added new func as '" << name << "' in scope " << current_scope->name << std::endl;
			}
			else
			{
				if (!current_scope->make(keywords.at(1).flat(), keywords.at(0).flat())) { add_error(GLUU_ERROR_INVALID_TYPE, "type '" + keywords.at(0).flat() + "' doesn't exist or is not registered", dec.begin()); return; } //error GLUU_ERROR_INVALID_TYPE
				std::cout << "made new variable " << keywords.at(0).flat() << " " << keywords.at(1).flat() << " in scope " << current_scope->name << std::endl;
				if (keywords.size() > 3 && keywords.at(2).flat() == "=")
				{
					string type = keywords.at(0).flat();
					string name = keywords.at(1).flat();
					string str = keywords.at(3).flat();
					int i = current_scope->get(name)->destringify(str);
					if (i == -1)
					{
						add_error(GLUU_ERROR_INVALID_STRING_TRANSLATION, "could not convert '" + str + "' to type '" + type + "' (no translation)", dec.begin()); return;
					}
					else if (i == 0)
					{
						add_error(GLUU_ERROR_INVALID_STRING_TRANSLATION, "could not convert '" + str + "' to type '" + type + "' (invalid string)", dec.begin()); return;
					}
				}
			}
		}

		void get_declaractions(string_ranges row)
		{
			string new_keyword = "new ";
			vector<string_ranges> declaractions = split_escape_delim(row, expr_open, expr_close, new_keyword);
			if (!declaractions.empty())
				for (auto it = declaractions.begin() + 1; it != declaractions.end(); it++)
				{
					parse_declaration(*it);
				}
		}

		vector<string_ranges> get_keyword_args(vector<string_ranges>& keywords, size_t cnt, size_t& i)
		{
			vector<string_ranges> args;
			for (size_t y = 0; y < cnt; y++)
			{
				i++;
				if (i >= keywords.size()) {
					add_error(GLUU_ERROR_KEYWORD_MISSING_ARGS, "Not enough params for keyword '" + keywords.at(i - 1).flat() + "'", keywords.at(i - 1).begin());
					return {};
				}//error GLUU_ERROR_MISSING_ARGS

				string_ranges current = range_trim(keywords.at(i), ' ');
				if (current.empty()) {
					y--;  continue;
				}
				args.push_back(current);
			}
			return args;
		}

		Element parse_header(string_ranges head)
		{
			head = range_trim(head, ' ');
			vector<string_ranges> keywords = split_and_delim(head, expr_open, expr_close, " ");

			Element row;
			row.scope = std::make_shared< VariableRegistry>();

			for (size_t i = 0; i < keywords.size(); i++)
			{
				string current = range_trim(keywords.at(i), ' ').flat();

				if (current.empty()) { continue; } //error GLUU_ERROR_INVALID_ROW_PARAM

				if (widgets.count(current))
				{
					size_t p = widgets.at(current)->fetch_keyword().first;
					vector<string_ranges> args = get_keyword_args(keywords, p, i);
					if (args.size() != p) return row;
					row.widget = widgets.at(current)->make(args, *this);
				}
				else if (keywords_func.count(current))
				{
					auto& p = keywords_func.at(current);
					vector<string_ranges> args = get_keyword_args(keywords, p.first, i);
					if (args.size() != p.first) return row;
					p.second(*this, row, args);
				}
				else
				{
					add_error(GLUU_ERROR_INVALID_KEYWORD, "Unrecognised keyword '" + current + "'", keywords.at(i).begin());
					//std::cout << "Invalid param: " << current << std::endl; //error
				}
			}

			return row;
		}

		string_ranges extract_header(string_ranges& range, bool& error)
		{
			string_ranges head = range_until(range, "<");
			range = { head.skip(), range.end() };

			/*if (head.end() == range.end())
			{
				add_error(GLUU_ERROR_INVALID_ROW, "Could not get a body for this row (could not find end)", range.begin());
				error = true;
				range = { range.begin(), range.begin() };
			}*/

			
			return head;
		}

		void parse_graphic(string_ranges row, bool is_row)
		{
			bool err = false;
			string_ranges head = extract_header(row, err);

			if (err)
			{
				return;
			}
			

			for (size_t i = 0; i < row_level; i++)
			{
				std::cout << "\t";
			}
			std::cout << head.flat() << std::endl;

			Element row_obj = parse_header(head);
			shared_ptr<VariableRegistry> old_scope = current_scope;
			Element* old_base = graphics->current_row;

			row_obj.is_row = is_row;

			row_obj.scope->name = "Row scope";
			current_scope = row_obj.scope;
			graphics->current_row = &row_obj;
			variable_dictionnary()->enter_scope(current_scope);
			row_level++;

			parse_range(row, is_row ? "COL" : "ROW", !is_row);

			row_level--;
			variable_dictionnary()->exit_scope();
			graphics->current_row = old_base;
			current_scope = old_scope;

			graphics->current_row->nested.push_back(row_obj);
		}

		void parse_range(string_ranges range, const string& keyword, bool row)
		{
			range = range_trim(range, ' ');

			/*if (range.empty()) {
				add_error(GLUU_ERROR_INVALID_ROW, "Invalid row", range.begin());
				return;
			}*/

			vector<string_ranges> sub_rows = range_delimiter(range, row_open, row_close);

			for (auto it = sub_rows.begin(); it != sub_rows.end(); it += 2)
			{
				auto in = next(it);

				vector<string_ranges> columns = chain(*it, range_until, keyword);
 
				get_declaractions(columns.front());
				if (columns.size() > 1)
				{
					string_ranges column = { columns.at(1).begin(), in->end() }; 
					column = range_trim(column, ' ');
					//column.end() -= 1;
					parse_graphic(column, row);
				}
			}
		}

		shared_ptr<Compiled> parse(string& str)
		{
			graphics = make_shared<Compiled>();
			//graphics->widgets = widgets;
			
			graphics->compiled_scope = variable_dictionnary()->make_temporary_scope("Global expression scope");
			current_scope = graphics->compiled_scope;

			current_scope->add(make_generic_ref(graphics->base_row), "BASE");

			variable_dictionnary()->enter_scope(GLUU_scope);
			variable_dictionnary()->enter_scope(current_scope);

			graphics->current_row = &graphics->base_row;
			string row_keyword = "ROW";
			str += '\n';
			remove_all_range(str, "//", "\n", false);
			remove_all_range(str, "/*", "*/", true);

			lines.clear();
			line_cntr = 1;
			size_t coffset = 0;
			while (true)
			{
				size_t f = str.find('\n', coffset);

				if (f == str.npos) break;

				lines.emplace(f, line_cntr);
				line_cntr++;
				coffset = f + 1;
			}

			change_whitespace_to_space(str);

			source_begin = str.begin();
			
			parse_range(str, row_keyword, true);

			variable_dictionnary()->exit_scope();
			variable_dictionnary()->exit_scope();

			if (!errors.empty())
			{
				std::cout << "Errors during compilation: " << std::endl;
				for (auto& e : errors)
				{
					std::cout << "At line " << e.line << " character " << e.ch << std::endl;
					std::cout << e.message << std::endl;
				}
			}

			return graphics;
		}

		void render(Rect_d windowSize)
		{
			graphics->base_row.render(windowSize);
		}

		
	};

	typedef Singleton<Parser> Global;
}
