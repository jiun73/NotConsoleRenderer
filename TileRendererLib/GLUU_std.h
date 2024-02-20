#pragma once

#include "GLUU_parser.h"

template<typename T>
inline bool GLUU_is_id(shared_generic gen)
{
	return gen->identity() == typeid(T);
}

template<typename T>
inline bool GLUU_is_type(shared_generic gen)
{
	return gen->type() == typeid(T);
}

template<typename T>
inline T& GLUU_conv_type(shared_generic gen)
{
	return *(T*)(gen->raw_bytes());
}

template<typename T>
inline shared_ptr<T> GLUU_rein(shared_generic gen)
{
	return std::reinterpret_pointer_cast<T>(gen);
}

template<typename T>
inline void GLUU_import_function(const string& s, function<T> f)
{
	shared_generic func = std::make_shared<GenericFunctionType<function<T>>>(f);
	variable_dictionnary()->global()->add(func, s);
}

using _sgen_ = shared_generic;

inline void GLUU_import_standard()
{
	GLUU_import_function<void(_sgen_, _sgen_)>("=", [](_sgen_ b, _sgen_ a) {a->set(b); });

	GLUU_import_function<bool(_sgen_, _sgen_)>("==", [](_sgen_ b, _sgen_ a)
		{
			if (!GLUU_is_id<GenericObject>(a)) return false;
			if (!GLUU_is_id<GenericObject>(b)) return false;
			return GLUU_rein<GenericObject>(a)->equals(b);
		});

	GLUU_import_function<void(int&)>("++", [](int& a) {a++; });
	GLUU_import_function<void(int&)>("--", [](int& a) {a--; });
	GLUU_import_function<int(int, int)>("/", [](int b, int a) {return a / b; });
	GLUU_import_function<int(int, int)>("*", [](int b, int a) {return a * b; });
	GLUU_import_function<int(int, int)>("+", [](int b, int a) {return a + b; });
	GLUU_import_function<int(int, int)>("-", [](int b, int a) {return a - b; });
	GLUU_import_function<bool(int, int)>("/-", [](int b, int a) {return a < b; });
	GLUU_import_function<bool(int, int)>("/+", [](int b, int a) {return a > b; });
	GLUU_import_function<bool()>(":true", []() {return true; });
	GLUU_import_function<bool()>(":false", []() {return false; });
	GLUU_import_function<bool(bool, bool)>(":or", [](bool b, bool a) {return a || b; });
	GLUU_import_function<bool(bool, bool)>(":and", [](bool b, bool a) {return a && b; });

	GLUU_import_function<void(GLUU&, GLUU&)>("$while", [](GLUU& expr, GLUU& condition)
		{
			while (true)
			{
				shared_generic gen = condition.evaluate();
				
				if (!GLUU_is_type<bool>(gen)) { std::cout << "wrong type for while" << std::endl;  return; }
				if (!GLUU_conv_type<bool>(gen)) return;

				expr.evaluate();
			}
		});

	GLUU_import_function<void(GLUU&, bool)>("$if", [](GLUU& expr, bool b) {if (b) expr.evaluate(); });

	GLUU_import_function<void(GLUU&, shared_generic)>("$foreach", [](GLUU& expr, shared_generic a)
		{
			if (!GLUU_is_id<GenericContainer>(a)) { std::cout << "Cannot foreach a non-container" << std::endl; ; return; }
			for (size_t i = 0; i < GLUU_rein<GenericContainer>(a)->size(); i++) expr.evaluate();
		});

	GLUU_import_function<string(_sgen_)>(":type", [](_sgen_ a) {return a->type().name(); });
	GLUU_import_function<void(_sgen_)>(":cout", [](_sgen_ a) {std::cout << a->stringify() << std::endl; });
	GLUU_import_function<int()>(":ticks", []() { return (int)SDL_GetTicks(); });

	GLUU_import_function<void(string, _sgen_)>("#=", [](string b, shared_generic a) { a->destringify(b); });
	GLUU_import_function<void(_sgen_)>("#X", [](_sgen_) {});
	GLUU_import_function<string()>("##", []() { return "\n"; });
	GLUU_import_function<string(_sgen_)>("#", [](_sgen_ a) { return a->stringify(); });

	GLUU_import_function<void(_sgen_, _sgen_)>("-pushb", [](_sgen_ b, _sgen_ a) 
		{ 
			if (!GLUU_is_id<GenericContainer>(a)) { std::cout << "Cannot push a non-container" << std::endl;; return; }

			auto container = GLUU_rein<GenericContainer>(a);
			container->insert(b, container->size());
		});

	GLUU_import_function<void(_sgen_, _sgen_)>("-pushf", [](_sgen_ b, _sgen_ a)
		{
			if (!GLUU_is_id<GenericContainer>(a)) { std::cout << "Cannot push a non-container" << std::endl;; return; }

			auto container = GLUU_rein<GenericContainer>(a);
			container->insert(b, 0);
		});

	GLUU_import_function<void(int, _sgen_, _sgen_)>("-ins", [](size_t i, _sgen_ b, _sgen_ a)
		{
			if (!GLUU_is_id<GenericContainer>(a)) { std::cout << "Cannot push a non-container" << std::endl;; return; }

			auto container = GLUU_rein<GenericContainer>(a);
			container->insert(b, i);
		});

	GLUU_import_function<_sgen_(int, _sgen_)>("-at", [](size_t i, _sgen_ a)-> _sgen_
		{
			if (!GLUU_is_id<GenericContainer>(a)) { std::cout << "Cannot at a non-container" << std::endl; return nullptr; }

			auto container = GLUU_rein<GenericContainer>(a);
			return container->at(i);
		});

	GLUU_import_function<_sgen_( _sgen_)>("@", [](_sgen_ a)-> _sgen_
		{
			if (!GLUU_is_id<GenericObject>(a)) { std::cout << "Cannot dereference a non-object!" << std::endl; return nullptr; }

			shared_ptr<GenericObject> obj = std::reinterpret_pointer_cast<GenericObject>(a);
			return obj->dereference();
		});
}
