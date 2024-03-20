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

	class ExpressionParser;
	class LineParser;

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
		SeqVar<bool> absolute = false;
		SeqVar<Rect> destination = Rect(0,0);
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
		friend ExpressionParser;
		friend LineParser;

	private:
		shared_ptr<VariableRegistry> GLUU_scope;

		shared_ptr<VariableRegistry> current_scope;
		map <string, pair<size_t, function<void(Parser&, Element&, vector<string_ranges>)>>> keywords_func;
		map <string, shared_ptr<Widget>> widgets;

		shared_ptr<Compiled> graphics;

		vector<Errorinfo> errors;
		

		string::iterator source_begin;
		map<size_t, size_t> lines;
		size_t line_cntr = 0;
		size_t seq_level = 0;
		size_t row_level = 0;

		unordered_map<type_index, Inspector> inspectors;

	public:
		const string row_open = "<";
		const string row_close = ">";
		const string expr_open = "{";
		const string expr_close = "}";

		shared_ptr<VariableRegistry> get_scope() { return GLUU_scope; }

		Parser();
		~Parser() {}

		template<typename T>
		void register_inspector(function<shared_generic(shared_generic, const string&)> inspector)
		{
			Inspector inspect;
			inspect.inspect = inspector;
			inspect.type_factory = make_generic<T>();
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

		

		void add_error(Errors code, const string& message, string::iterator it);

		void register_class(shared_ptr <Widget> c);

		void parse_declaration(string_ranges dec);

		shared_generic get_variable_from_scope(string_ranges name)
		{
			shared_generic var = variable_dictionnary()->get(name.flat());
			if (var == nullptr) { add_error(GLUU_ERROR_INVALID_VARIABLE_NAME, "variable '" + name.flat() + "' doesn't exist in this scope", name.begin()); }
			return var;
		}

		Expression parse_expression(string_ranges expression);

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
			vector<string_ranges> ranges = range_delimiter(range, expr_open, expr_close);

			for (auto it = ranges.begin(); it != ranges.end(); it += 2)
			{
				string_ranges sub_head = range_until(*it, "<");

				if (it->end() == sub_head.end())
				{
					continue;
				}

				auto begin = range.begin();

				range = { sub_head.skip(), range.end() };

				return { begin, sub_head.end() };
			}

			auto copy = range;
			range = { range.begin(), range.begin() };
			
			return copy;
		}

		string_ranges extract_tail(string_ranges& range, bool& error)
		{
			vector<string_ranges> ranges = range_delimiter(range, row_open + expr_open, row_close + expr_close);

			for (auto it = ranges.begin(); it != ranges.end(); it += 2)
			{
				string_ranges sub_head = range_until(*it, ">");

				if (it->end() == sub_head.end())
				{
					continue;
				}

				auto end = range.end();

				range = { range.begin(), sub_head.end()};

				return { sub_head.skip(), end };
			}

			return {range.begin(), range.begin()};
		}

		void parse_graphic(string_ranges row, bool is_row)
		{
			bool err = false;
			string_ranges head = extract_header(row, err);
			string_ranges tail = extract_tail(row, err);

			

			if (err)
			{
				return;
			}
			

			for (size_t i = 0; i < row_level; i++)
			{
				std::cout << "\t";
			}
			std::cout << "HEAD " + head.flat() << std::endl;
			std::cout << "BODY " + row.flat() << std::endl;
			std::cout << "TAIL " + tail.flat() << std::endl;


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

			get_declaractions(tail);
		}

		void parse_range(string_ranges range, const string& keyword, bool row)
		{
			range = range_trim(range, ' ');

			/*if (range.empty()) {
				add_error(GLUU_ERROR_INVALID_ROW, "Invalid row", range.begin());
				return;
			}*/



			vector<string_ranges> sub_rows = split_escape_delim(range, expr_open, expr_close, keyword);

			if (sub_rows.empty()) return;

			get_declaractions(sub_rows.front());

			for (auto it = sub_rows.begin() + 1; it != sub_rows.end(); it ++)
			{
				/*vector<string_ranges> sub_sub_rows = split_escape_delim(*it, row_open, row_close, keyword);

				if (sub_sub_rows.empty()) return;

				for (auto it2 = sub_sub_rows.begin() + 1; it2 != sub_sub_rows.end(); it2++)
				{
					parse_graphic(*it2, row);
				}*/

				//std::cout << "SPLIT" << it->flat() << std::endl;

				vector<string_ranges> sub_sub_rows = range_delimiter(*it, row_open, row_close);
				bool even = true;
				for (auto it2 = sub_sub_rows.begin(); it2 != sub_sub_rows.end(); it2 += 2)
				{
					auto in = next(it2);

					string_ranges column = { it2->begin(), in->end() };
					column = range_trim(column, ' ');

					if (even)
					{
						

						if (column.empty()) continue;

						std::cout << ":: '" << column.flat() << "' " << std::endl;
						parse_graphic(column, row);
						even = false;
					}
					else
					{
						std::cout << "?? '" << column.flat() << "' " << std::endl;

						get_declaractions(column);

						even = true;
					}
				}
				
			}

			//vector<string_ranges> sub_rows = range_delimiter(range, row_open, row_close);

			//for (auto it = sub_rows.begin(); it != sub_rows.end(); it += 2)
			//{
			//	auto in = next(it);

			//	vector<string_ranges> columns = chain(*it, range_until, keyword);
 
			//	
			//	if (columns.size() > 1)
			//	{
			//		string_ranges column = { columns.at(1).begin(), in->end() }; 
			//		column = range_trim(column, ' ');
			//		//column.end() -= 1;
			//		parse_graphic(column, row);
			//	}
			//}
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

			output_errors();

			return graphics;
		}

		bool output_errors() 
		{
			if (!errors.empty())
			{
				std::cout << "Errors during compilation: " << std::endl;
				for (auto& e : errors)
				{
					std::cout << "At line " << e.line << " character " << e.ch << std::endl;
					std::cout << e.message << std::endl;
				}
				errors.clear();
				return true;
			}
			return false;
		}

		void render(Rect_d windowSize)
		{
			graphics->base_row.render(windowSize);
		}

		
	};

	typedef Singleton<Parser> Global;
}
