#pragma once

#include "GLUU_parser.h"
#include "GLUU_std.h"

#include "Singleton.h"
#include "File.h"

namespace GLUU {
	using std::unique_ptr;

	using Compiled_ptr = shared_ptr<Compiled>;

	inline Parser* parser() { return Global::get(); }

	inline Compiled_ptr parse(string& str) { return parser()->parse(str); }
	inline Compiled_ptr parse_copy(string str) { return parser()->parse(str); }

	inline Compiled_ptr parse_file(const string& str)
	{
		File file(str, FILE_READING_STRING);
		return parse_copy(file.getString());
	}
}