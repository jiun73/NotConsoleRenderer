#pragma once
#include "CommandDictionnary.h"

namespace GLUU {
	//using std::queue;

	class ExpressionParser;
	class LineParser;

	enum Errors
	{
		GLUU_ERROR_INVALID_EXPRESSION_KEYWORD,
		GLUU_ERROR_INVALID_ARG_FORMAT,
		GLUU_ERROR_INVALID_VARIABLE_IDENTITY,
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
		GLUU_ERROR_INVALID_MEMBER_EXPRESSION,

		GLUU_ERROR_RUNTIME_MISSING_RETURN_FLAG,
		GLUU_ERROR_RUNTIME_INVALID_MEMBER,
		GLUU_ERROR_RUNTIME_NULL_RETURN
	};

	struct DebugInfo
	{
		size_t line;
		size_t ch;
		size_t tot_ch;
		size_t file = 0;

		shared_ptr<VariableRegistry> scope = nullptr;

		void print() 
		{
			std::cout << "At line " << line << " character " << ch << std::endl;
		}
	};

	struct CompileErrorInfo
	{
		DebugInfo debug;
		string message;
		Errors code;

		void print()
		{
			std::cout << "Compile error!" << std::endl;
			debug.print();
			std::cout << message << std::endl;
		}
	};

	struct RuntimeErrorInfo
	{
		DebugInfo debug;
		string message;
		Errors code;

		void print() 
		{
			std::cout << "Runtime error!" << std::endl;
			debug.print();
			std::cout << message << std::endl;
		}
	};

	struct Debugger
	{
		vector<CompileErrorInfo> compiler_errors;
		vector<DebugInfo> runtime_info;

		string::iterator source_begin;
		map<size_t, size_t> lines;
		size_t line_cntr = 0;
		size_t seq_level = 0;
		size_t row_level = 0;
		bool verbose_compiler = false;

		void enter_runtime(const DebugInfo& info) { runtime_info.push_back(info); }
		void exit_runtime() { runtime_info.pop_back(); }

		void throw_error(Errors code, const string& message)
		{
			RuntimeErrorInfo info;
			info.code = code;
			info.message = message;

			if (!runtime_info.empty())
			{
				info.debug = runtime_info.back();
				info.print();
			}

			Commands::open();
			CommandSpace space("GLUU");

			space.temp.push_back({ "break", [&info](__COMMAND_ARGS__)
				{
					info.print();
				} });

			space.temp.push_back({ "error", [&info](__COMMAND_ARGS__)
				{
					info.print();
				} });

			space.temp.push_back({ "scope", [&info](__COMMAND_ARGS__)
				{
					if (info.debug.scope == nullptr)
					{
						std::cout << "no scope available" << std::endl;
						return;
					}

					std::cout << info.debug.scope->name << std::endl;

					for (auto v : info.debug.scope->all())
					{
						std::cout << v.first << " = " << v.second->stringify() << std::endl;
					}
				} });


			auto* rtinf = &runtime_info;
			bool stop = true;

			space.temp.push_back({ "callstack", [rtinf](__COMMAND_ARGS__)
				{
					for (auto& m : *rtinf)
					{
						m.print();
						std::cout << "/" << std::endl;
					}
				} });

			space.temp.push_back({ "scopes", [rtinf](__COMMAND_ARGS__)
				{
					for (auto& info : *rtinf)
					{
						info.print();
						std::cout << "/" << std::endl;

						if (info.scope == nullptr)
						{
							std::cout << "no scope available" << std::endl;
							continue;
						}

						std::cout << info.scope->name << std::endl;

						for (auto v : info.scope->all())
						{
							std::cout << v.first << " = " << v.second->stringify() << std::endl;
						}
					}
				} });

			space.temp.push_back({ "resume", [&stop](__COMMAND_ARGS__)
				{
					stop = false;
				} });

			Commands::get()->enter_space(space);
			while (stop)
			{
				Commands::get()->update();
			}
			Commands::get()->exit_space(space);
		}

		DebugInfo make_info(string::iterator it)
		{
			DebugInfo debug;
			debug.tot_ch = std::distance(source_begin, it);
			auto line_it = lines.lower_bound(debug.tot_ch);
			auto prev_line_it = prev(line_it);

			if (line_it == lines.end())
			{
				debug.line = prev(line_it)->second;
			}
			else
			{
				debug.line = line_it->second;
			}

			if (prev_line_it == lines.end())
			{
				debug.ch = debug.tot_ch;
			}
			else
			{
				debug.ch = debug.tot_ch - prev_line_it->first;
			}
			return debug;
		}

		void static_error(Errors code, const string& message, string::iterator it);
		bool output_errors();
		void fetch_lines(const string& str);
		void print_row(const string& msg = "");
		void next_row() { row_level++; }
		void prev_row() { row_level--; }
		void next_level();
		void prev_level() { seq_level--; }
		void output_seq(const string& str, bool start = false);
	};
}