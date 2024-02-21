#pragma once

#include "GLUU_parser.h"
#include "GLUU_std.h"
#include "GLUU_widgets.h"

#include "Singleton.h"

namespace GLUU {
	using std::unique_ptr;

	using Compiled_ptr = shared_ptr<Compiled>;

	typedef Singleton<Parser> Global;

	inline Parser* parser() { return Global::get(); }

	inline Compiled_ptr parse(string& str) { return parser()->parse(str); }

	struct Import
	{
		Import(function<void()> callback) { callback(); }
		~Import() { }
	};

	template<typename T>
	struct ImportWidget 
	{
		static_assert(std::is_base_of_v<Widget, T>);

		ImportWidget() { parser()->register_class(make_shared<T>()); }
		~ImportWidget() {}
	};

	inline Import import_std(import_standard);
	inline ImportWidget<TextWidget> import_text;
	inline ImportWidget<TextboxWidget> import_textbox;
	inline ImportWidget<ButtonWidget> import_button;
}