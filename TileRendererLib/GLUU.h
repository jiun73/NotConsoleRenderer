#pragma once

#include "GLUU_import.h"
#include "GLUU_widgets.h"

#define CAT(Arg1, Arg2) CAT_(Arg1, Arg2)
#define CAT_(Arg1, Arg2) Arg1##Arg2 
#define REGISTER_TYPE_N(...) CAT(__COUNTER__,_adder = new FactoryManagerAdder<__VA_ARGS__>(#__VA_ARGS__, true))
#define REGISTER_TYPE(...) CAT(const FactoryManagerAdder<__VA_ARGS__>* _,REGISTER_TYPE_N(__VA_ARGS__))
#define NEW_INSPECTOR_N(b) CAT(__COUNTER__,_inspector)
#define NEW_INSPECTOR_N2(b) CAT(inline GLUU::ImportInspector<b> _,NEW_INSPECTOR_N(b))
#define NEW_INSPECTOR(b) NEW_INSPECTOR_N2(b)
#define INSPECTOR_FIELD(b, n) { #n , [](shared_generic gen) { return make_generic_ref((*((b*)(gen->raw_bytes()))).##n ); }} 
#define INSPECTOR_FIELDC(b, n) { #n , [](shared_generic gen) { return make_generic_container_ref((*((b*)(gen->raw_bytes()))).##n ); }} 
#define INSPECTOR_FIELDS(b, n) { #n , [](shared_generic gen) { return (((b*)(gen->raw_bytes()))->##n##.get_gen() ); }} 

namespace GLUU {
	using std::unique_ptr;

	//inline Import import_std(import_standard);
	//inline ImportWidget<TextWidget> import_text;
	//inline ImportWidget<TextboxWidget> import_textbox;

	/*NEW_INSPECTOR(Element)({
		INSPECTOR_FIELDS(Element, fit),
		INSPECTOR_FIELDS(Element, size),
		INSPECTOR_FIELD(Element, last_dest)
		});*/
}

#define GLUU_IMPORT_MAIN(n) inline ::GLUU::ImportFunction<decltype(n)> gluu_##n##_import("$" + string(#n), n);