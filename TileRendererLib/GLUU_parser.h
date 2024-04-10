#pragma once
#include "Generics.h"
#include "FunctionGenerics.h"
#include "StringRanges.h"
#include "CommandDictionnary.h"
#include "NotConsoleRenderer.h"

#include "GLUU_expr.h"
#include "GLUU_styler.h"
#include "GLUU_elem.h"

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
		GLUU_ERROR_INVALID_STYLER,
		GLUU_ERROR_WIDGET_MISSING_ARGS,
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

#include <map>
	using ::std::map;
	using ::std::make_pair;
	using ::std::type_index;

	struct Compiled
	{
		shared_ptr<VariableRegistry> compiled_scope;
		vector<Expression> callbacks;
		vector<Element*> popups;
		Element* last_focused = nullptr;
		Element* current_row = nullptr;
		Element base_row;

		void init() 
		{
			base_row.get_popups(popups);
		}

		void render(Rect_d window) 
		{ 
			for (auto& c : callbacks)
			{
				c.evaluate();
			}

			bool move = false;
			size_t move_index = 0;

			V2d_i mouse_pos = mouse_position();
			bool click = mouse_left_pressed() || mouse_right_pressed();

			for (auto& e : popups)
			{
				if (e->set_popup(window, mouse_pos, click))
				{
					if (last_focused != nullptr)
					{
						last_focused->focus = false;
					}

					last_focused = e;
					move = true;
				}//popup are rendered after everything, and in a certain order

				if (!move) move_index++;
			}

			if (move)
			{
				popups.erase(popups.begin() + move_index);
				popups.insert(popups.begin(), last_focused);
			}

			base_row.set(window); 

			MouseInfo mouse;
			mouse.pos = mouse_pos;
			mouse.click = click;

			for (auto& e : popups)
			{
				e->update(mouse, true);
				e->update_l2(true);
			}

			base_row.update(mouse);
			base_row.update_l2();

			

			base_row.render();

			for (auto& e : popups)
				e->render(true);
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
		map <string, map<string, shared_ptr<StylerInterface>>> stylers;

		shared_ptr<Compiled> graphics;

		vector<Errorinfo> errors;
		

		string::iterator source_begin;
		map<size_t, size_t> lines;
		size_t line_cntr = 0;
		size_t seq_level = 0;
		size_t row_level = 0;

		unordered_map<type_index, Inspector> inspectors;
		string current_style;

	public:
		const string default_style_name = "default";
		const string row_keyword = "row";
		const string col_keyword = "col";

		const string row_open = "<";
		const string row_close = ">";
		const string expr_open = "{";
		const string expr_close = "}";

		shared_ptr<VariableRegistry> get_scope() { return GLUU_scope; }

		Parser();
		~Parser() {}

		void register_styler(shared_ptr < StylerInterface> styler, const string& widget_for, const string& pack_name)
		{
			if (!stylers.count(pack_name))
				stylers.emplace(pack_name, map<string, shared_ptr<StylerInterface>>());
			stylers[pack_name].emplace(widget_for, styler);
		}

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

			current_style = default_style_name;

			Element row;
			row.scope = std::make_shared< VariableRegistry>();

			for (size_t i = 0; i < keywords.size(); i++)
			{
				string current = range_trim(keywords.at(i), ' ').flat();

				if (current.empty()) {continue; } //error GLUU_ERROR_INVALID_ROW_PARAM

				if (widgets.count(current))
				{
					size_t p = widgets.at(current)->fetch_keyword().first;
					vector<string_ranges> args = get_keyword_args(keywords, p, i);
					if (args.size() != p)
					{
						add_error(GLUU_ERROR_WIDGET_MISSING_ARGS, "Not enough arguments fo widget '" + current + "'", keywords.at(i).begin());
						return row;
					}
					row.widget = widgets.at(current)->make(args, *this);

					if (!stylers.count(current_style))
					{
						add_error(GLUU_ERROR_INVALID_STYLER, "No Stylers in pack '" + current_style + "'", keywords.at(i).begin());
					}

					else if (!stylers.at(current_style).count(current))
					{
						add_error(GLUU_ERROR_INVALID_STYLER, "No Styler for '" + current+ "' in pack '" + current_style + "'", keywords.at(i).begin());
					}
					else
					{
						row.widget->styler = stylers.at(current_style).at(current);
					}
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

			Element row_obj = parse_header(head);
			shared_ptr<VariableRegistry> old_scope = current_scope;
			Element* old_base = graphics->current_row;

			row_obj.is_row = is_row;

			row_obj.scope->name = "Row scope";
			current_scope = row_obj.scope;
			graphics->current_row = &row_obj;
			variable_dictionnary()->enter_scope(current_scope);
			row_level++;

			parse_range(row, is_row ? col_keyword : row_keyword, !is_row);

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



			vector<string_ranges> sub_rows = split_escape_delim(range, expr_open + row_open, expr_close + row_close, keyword);

			if (sub_rows.empty()) return;

			get_declaractions(sub_rows.front());

			for (auto it = sub_rows.begin() + 1; it != sub_rows.end(); it++)
			{
				parse_graphic(*it, row);
			}

				/*vector<string_ranges> sub_sub_rows = split_escape_delim(*it, row_open, row_close, keyword);

				if (sub_sub_rows.empty()) return;

				for (auto it2 = sub_sub_rows.begin() + 1; it2 != sub_sub_rows.end(); it2++)
				{
					
				}*/

			//	//std::cout << "SPLIT" << it->flat() << std::endl;

			//	vector<string_ranges> sub_sub_rows = range_delimiter(*it, row_open, row_close);
			//	bool even = true;
			//	for (auto it2 = sub_sub_rows.begin(); it2 != sub_sub_rows.end(); it2 += 2)
			//	{
			//		auto in = next(it2);

			//		string_ranges column = { it2->begin(), in->end() };
			//		column = range_trim(column, ' ');

			//		if (even)
			//		{
			//			

			//			if (column.empty()) continue;

			//			std::cout << ":: '" << column.flat() << "' " << std::endl;
			//			parse_graphic(column, row);
			//			even = false;
			//		}
			//		else
			//		{
			//			std::cout << "?? '" << column.flat() << "' " << std::endl;

			//			get_declaractions(column);

			//			even = true;
			//		}
			//	}
			//	
			//}

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

			graphics->init();

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
			graphics->base_row.set(windowSize);
		}

		
	};

	typedef Singleton<Parser> Global;
}
