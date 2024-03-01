#pragma once

#include "GLUU_parser.h"
#include "GLUU_std.h"
#include "GLUU_widgets.h"

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

	template <typename T>
	struct ImportFunction;

	template <typename R, typename... Ts>
	struct ImportFunction <R(Ts...)>
	{
		ImportFunction(const string& name, function<R(Ts...)> func) 
		{
			import_function<R(Ts...)>(name, func);
		}
		~ImportFunction() {}
	};

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

	template<typename T>
	struct ImportInspector 
	{
		ImportInspector(function<shared_generic(shared_generic, const string&)> func)
		{
			parser()->register_inspector<T>(func);
		}
		~ImportInspector() {}
	};

	inline Import import_std(import_standard);
	inline ImportWidget<TextWidget> import_text;
	inline ImportWidget<TextboxWidget> import_textbox;
	inline ImportWidget<ButtonWidget> import_button;

	inline ImportInspector<Element> elem_inspector([](shared_generic gen, const string& str) -> shared_generic
		{
			Element& obj = *(Element*)(gen->raw_bytes());
			if (str == "fit") return obj.fit.get_gen();
			if (str == "size") return obj.size.get_gen();
			return nullptr;
		});
}

#define GLUU_IMPORT_MAIN(n) inline GLUU::ImportFunction<decltype(n)> gluu_##n##_import("$" + string(#n), n);