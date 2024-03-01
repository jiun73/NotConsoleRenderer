#pragma once

#include "GLUU_parser.h"

#include "Color.h"

namespace GLUU {


	template<typename T>
	inline bool is_iden(shared_generic gen)
	{
		return gen->identity() == typeid(T);
	}

	template<typename T>
	inline bool is_type(shared_generic gen)
	{
		return gen->type() == typeid(T);
	}

	template<typename T>
	inline T& conv_type(shared_generic gen)
	{
		return *(T*)(gen->raw_bytes());
	}

	template<typename T>
	inline shared_ptr<T> rein(shared_generic gen)
	{
		return std::reinterpret_pointer_cast<T>(gen);
	}

	template<typename T>
	inline void import_function(const string& s, function<T> f)
	{
		shared_generic func = std::make_shared<GenericFunctionType<function<T>>>(f);
		Global::get()->get_scope()->add(func, s);
	}

	using _sgen_ = shared_generic;

	inline void import_standard()
	{
		__REGISTER_CLASS__(int);
		__REGISTER_CLASS__(double);
		__REGISTER_CLASS__(bool);
		__REGISTER_CLASS__(char);
		__REGISTER_CLASS__(size_t);
		__REGISTER_CLASS__(string);
		__REGISTER_CLASS__(Color);
		const FactoryManagerAdder<vector<string>>* vector_string__adder = new FactoryManagerAdder<vector<string>>("vector<string>", true);

		import_function<void(_sgen_, _sgen_)>("=", [](_sgen_ b, _sgen_ a)
			{
				a->set(b);
			});
		import_function<void()>("!!!", []()
			{
				std::cout << "brpt" << std::endl;
			});

		import_function<bool(_sgen_, _sgen_)>("==", [](_sgen_ b, _sgen_ a)
			{
				if (!is_iden<GenericObject>(a)) return false;
				if (!is_iden<GenericObject>(b)) return false;
				return rein<GenericObject>(a)->equals(b);
			});

		import_function<void(int&)>("++", [](int& a) {a++; });
		import_function<void(int&)>("--", [](int& a) {a--; });
		import_function<int(int, int)>("/", [](int b, int a) {return a / b; });
		import_function<int(int, int)>("*", [](int b, int a) {return a * b; });
		import_function<int(int, int)>("+", [](int b, int a) {return a + b; });
		import_function<int(int, int)>("-", [](int b, int a) {return a - b; });
		import_function<bool(int, int)>("/-", [](int b, int a) {return a < b; });
		import_function<bool(int, int)>("/+", [](int b, int a) {return a > b; });
		import_function<bool()>(":true", []() {return true; });
		import_function<bool()>(":false", []() {return false; });
		import_function<bool(bool, bool)>(":or", [](bool b, bool a) {return a || b; });
		import_function<bool(bool, bool)>(":and", [](bool b, bool a) {return a && b; });
		import_function<bool(bool)>(":not", [](bool a) {return !a; });

		import_function<void(Expression&, Expression&)>("$while", [](Expression& expr, Expression& condition)
			{
				while (true)
				{
					shared_generic gen = condition.evaluate();

					if (!is_type<bool>(gen)) { std::cout << "wrong type for while" << std::endl;  return; }
					if (!conv_type<bool>(gen)) return;

					expr.evaluate();
				}
			});

		import_function<void(Expression&, bool)>("$if", [](Expression& expr, bool b) {if (b) expr.evaluate(); });

		import_function<void(Expression&, shared_generic)>("$foreach", [](Expression& expr, shared_generic a)
			{
				if (!is_iden<GenericContainer>(a)) { std::cout << "Cannot foreach a non-container" << std::endl; ; return; }
				for (size_t i = 0; i < rein<GenericContainer>(a)->size(); i++) expr.evaluate();
			});



		import_function<string(_sgen_)>(":type", [](_sgen_ a) {return a->type().name(); });
		import_function<int(bool)>("(bool-int)", [](bool a) {return a; });
		import_function<uint8_t(int)>("(u8)", [](int a) {return a; });
		import_function<uint16_t(int)>("(u16)", [](int a) {return a; });
		import_function<uint32_t(int)>("(u32)", [](int a) {return a; });
		import_function<uint64_t(int)>("(u64)", [](int a) {return a; });
		import_function<size_t(int)>("(size_t)", [](int a) {return a; });
		import_function<void(_sgen_)>(":cout", [](_sgen_ a) {std::cout << a->stringify() << std::endl; });
		import_function<int()>(":ticks", []() { return (int)SDL_GetTicks(); });

		import_function<void(string, _sgen_)>("#=", [](string b, shared_generic a) { a->destringify(b); });
		import_function<void(_sgen_)>("#X", [](_sgen_) {});
		import_function<string()>("##", []() { return "\n"; });
		import_function<string(_sgen_)>("#", [](_sgen_ a) { return a->stringify(); });

		import_function<void(_sgen_, _sgen_)>("-pushb", [](_sgen_ b, _sgen_ a)
			{
				if (!is_iden<GenericContainer>(a)) { std::cout << "Cannot push a non-container" << std::endl;; return; }

				auto container = rein<GenericContainer>(a);
				container->insert(b, container->size());
			});

		import_function<void(_sgen_, _sgen_)>("-pushf", [](_sgen_ b, _sgen_ a)
			{
				if (!is_iden<GenericContainer>(a)) { std::cout << "Cannot push a non-container" << std::endl;; return; }

				auto container = rein<GenericContainer>(a);
				container->insert(b, 0);
			});

		import_function<void(int, _sgen_, _sgen_)>("-ins", [](size_t i, _sgen_ b, _sgen_ a)
			{
				if (!is_iden<GenericContainer>(a)) { std::cout << "Cannot push a non-container" << std::endl;; return; }

				auto container = rein<GenericContainer>(a);
				container->insert(b, i);
			});

		import_function<_sgen_(int, _sgen_)>("-at", [](size_t i, _sgen_ a)-> _sgen_
			{
				if (!is_iden<GenericContainer>(a)) { std::cout << "Cannot at a non-container" << std::endl; return nullptr; }

				auto container = rein<GenericContainer>(a);
				return container->at(i);
			});

		import_function<_sgen_(_sgen_)>("@", [](_sgen_ a)-> _sgen_
			{
				if (!is_iden<GenericObject>(a)) { std::cout << "Cannot dereference a non-object!" << std::endl; return nullptr; }

				shared_ptr<GenericObject> obj = std::reinterpret_pointer_cast<GenericObject>(a);
				return obj->dereference();
			});
	}
}
