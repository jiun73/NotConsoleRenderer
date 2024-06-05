#include "pch.h"

void GLUU::Debugger::static_error(Errors code, const string& message, string::iterator it)
{
	CompileErrorInfo info;
	info.debug = make_info(it);
	info.message = message;
	info.code = code;
	compiler_errors.push_back(info);
}

bool GLUU::Debugger::output_errors()
{
	if (!compiler_errors.empty())
	{
		std::cout << "Errors during compilation: " << std::endl;
		for (auto& e : compiler_errors)
		{
			std::cout << "At line " << e.debug.line << " character " << e.debug.ch << std::endl;
			std::cout << e.message << std::endl;
		}
		compiler_errors.clear();
		return true;
	}
	return false;
}

void GLUU::Debugger::fetch_lines(const string& str)
{
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
}

void GLUU::Debugger::print_row(const string& msg)
{
	if (output_compile_tree)
	{
		for (size_t i = 0; i < row_level; i++)
		{
			std::cout << "\t";
		}

		for (size_t i = 0; i < row_level; i++)
		{
			std::cout << "\t";
		}
		std::cout << "HEAD " + msg << std::endl;
	}
}

void GLUU::Debugger::next_level()
{
	if (output_compile_tree)
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
}

void GLUU::Debugger::output_seq(const string& str, bool start)
{
	if (output_compile_tree)
	{
		if (seq_level == 0)
		{
			std::cout << std::endl;
			for (size_t i = 0; i < row_level + 1; i++)
			{
				std::cout << "\t";
			}
			std::cout << str << std::endl;
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
}
