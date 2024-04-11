#pragma once

#include "GLUU_import.h"
#include "GLUU_widgets.h"

namespace GLUU {
	using std::unique_ptr;

	inline Import import_std(import_standard);
	inline ImportWidget<TextWidget> import_text;
	inline ImportWidget<TextboxWidget> import_textbox;

	inline ImportInspector<Element> elem_inspector([](shared_generic gen, const string& str) -> shared_generic
		{
			Element& obj = *(Element*)(gen->raw_bytes());
			if (str == "fit") return obj.fit.get_gen();
			if (str == "size") return obj.size.get_gen();
			if (str == "dest") return make_generic_ref(obj.last_dest);
			return nullptr;
		});
}

#define GLUU_IMPORT_MAIN(n) inline ::GLUU::ImportFunction<decltype(n)> gluu_##n##_import("$" + string(#n), n);