#pragma once

#include "GLUU_global.h"

namespace GLUU {
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

	template<typename T>
	struct ImportStyler
	{
		static_assert(std::is_base_of_v<StylerInterface, T>);

		ImportStyler(const string& pack_name)
		{
			shared_ptr<StylerInterface> ptr = std::make_shared<T>();
			parser()->register_styler(ptr, ptr->widget_name(), pack_name);
		}
		~ImportStyler() {}
	};
}