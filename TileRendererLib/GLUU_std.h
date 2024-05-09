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
				if (!a->set(b))
				{
					Global::get()->debugger.throw_error(GLUU_ERROR_NOT_ENOUGH_ARGS, "Error when setting arg");
				};
			});
		import_function<void()>("!!!", []()
			{
				Global::get()->debugger.throw_error(GLUU_ERROR_NOT_ENOUGH_ARGS, "Hit a breakpoint");
			});

		import_function<void(_sgen_)>("=!", [](_sgen_ a)
			{
				Global::get()->get_scope()->add(a, "debugger_value");
				Global::get()->debugger.throw_error(GLUU_ERROR_NOT_ENOUGH_ARGS, "Hit a breakpoint with object");
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
		import_function<bool(int, int)>("<", [](int b, int a) {return a < b; });
		import_function<bool(int, int)>(">", [](int b, int a) {return a > b; });
		import_function<bool()>(":true", []() {return true; });
		import_function<bool()>(":false", []() {return false; });
		import_function<bool(bool, bool)>(":or", [](bool b, bool a) {return a || b; });
		import_function<bool(bool, bool)>(":and", [](bool b, bool a) {return a && b; });
		import_function<bool(bool)>(":not", [](bool a) {return !a; });

		import_function<void(double&)>("!++", [](double& a) {a++; });
		import_function<void(double&)>("!--", [](double& a) {a--; });
		import_function<double(double, double)>("!/", [](double b, double a) {return a / b; });
		import_function<double(double, double)>("!*", [](double b, double a) {return a * b; });
		import_function<double(double, double)>("!+", [](double b, double a) {return a + b; });
		import_function<double(double, double)>("!-", [](double b, double a) {return a - b; });
		import_function<bool(double, double)>("!/-", [](double b, double a) {return a < b; });
		import_function<bool(double, double)>("!/+", [](double b, double a) {return a > b; });

		import_function<void(Expression&, Expression&)>("$while", [](Expression& expr, Expression& condition)
			{
				while (true)
				{
					shared_generic gen = condition.evaluate_next();

					if (!is_type<bool>(gen)) { std::cout << "wrong type for while" << std::endl;  return; }
					if (!conv_type<bool>(gen)) return;

					expr.func_base->constant = expr.evaluate_next();

					if (expr.has_returned()) break;
				}
			});

		import_function<void(Expression&, bool)>("$if", [](Expression& expr, bool b) 
			{
				if (b) 
				{
					expr.func_base->constant = expr.evaluate_next();
				};
			});

		/*import_function<void(Expression&, shared_generic)>("$foreach", [](Expression& expr, shared_generic a)
			{
				if (!is_iden<GenericContainer>(a)) { std::cout << "Cannot foreach a non-container" << std::endl; ; return; }
				for (size_t i = 0; i < rein<GenericContainer>(a)->container_size(); i++)
				{
					expr.func_base->constant = expr.evaluate_next();
					if (expr.has_returned()) break;
				}
			});*/

		import_function<void(Expression&, shared_generic)>("$foreach", [](Expression& expr, shared_generic a)
			{
				if (!is_iden<GenericContainer>(a)) { std::cout << "Cannot foreach a non-container" << std::endl; ; return; }
				if(expr.args_name.size() != 1) { std::cout << "foreach requires at least one arg of the value-type in the expression " << std::endl; ; return; }
				auto cont = rein<GenericContainer>(a);
				for (size_t i = 0; i < cont->container_size(); i++)
				{
					vector<shared_generic> args = { rein<GenericObject>(cont->at(i))->reference() };
					expr.set_args(args);
					expr.func_base->constant = expr.evaluate_next();
					if (expr.has_returned()) break;
				}
			});


		import_function<string(_sgen_)>(":type", [](_sgen_ a) {return a->type().name(); });
		import_function<int(bool)>("(bool-int)", [](bool a) {return a; });
		import_function<int(size_t)>("(size_t-int)", [](size_t a) {return (int)a; });
		import_function<double(int)>("(.)", [](int a) {return (double)a; });
		import_function<int(double)>("(~)", [](double a) {return (int)a; });
		import_function<uint8_t(int)>("(u8)", [](int a) {return (uint8_t)a; });
		import_function<uint16_t(int)>("(u16)", [](int a) {return (uint16_t)a; });
		import_function<uint32_t(int)>("(u32)", [](int a) {return (uint32_t)a; });
		import_function<uint64_t(int)>("(u64)", [](int a) {return (uint64_t)a; });
		import_function<size_t(int)>("(size_t)", [](int a) {return (size_t)a; });
		import_function<void(_sgen_)>(":cout", [](_sgen_ a) {std::cout << a->stringify() << std::endl; });
		import_function<int()>(":ticks", []() { return (int)SDL_GetTicks(); });

		import_function<Rect(int, int, int, int)>(":rect", [](int h, int w, int y, int x) 
			{
				return Rect(x,y,h,w); 
			}
		);

		import_function<V2d_i(int, int)>(":vec", [](int y, int x)
			{
				return V2d_i(x, y);
			}
		);

		import_function<void(string, _sgen_)>("#=", [](string b, shared_generic a) { a->destringify(b); });
		import_function<void(_sgen_)>("#X", [](_sgen_) {});
		import_function<string()>("##", []() { return "\n"; });
		import_function<string(_sgen_)>("#", [](_sgen_ a) { return a->stringify(); });

		import_function<void(_sgen_, _sgen_)>("-push", [](_sgen_ b, _sgen_ a)
			{
				if (!is_iden<GenericContainer>(a)) { std::cout << "Cannot push a non-container" << std::endl;; return; }

				auto container = rein<GenericContainer>(a);
				container->insert(b, container->container_size());
			});

		import_function<void(_sgen_, _sgen_)>("-append", [](_sgen_ b, _sgen_ a)
			{
				if (!is_iden<GenericContainer>(a)) { std::cout << "Cannot push a non-container" << std::endl;; return; }

				auto container = rein<GenericContainer>(a);
				container->insert(b, 0);
			});

		import_function<void(int, _sgen_, _sgen_)>("-insert", [](size_t i, _sgen_ b, _sgen_ a)
			{
				if (!is_iden<GenericContainer>(a)) { std::cout << "Cannot push a non-container" << std::endl;; return; }

				auto container = rein<GenericContainer>(a);
				container->insert(b, i);
			});

		import_function<_sgen_(int, _sgen_)>("-at", [](size_t i, _sgen_ a)-> _sgen_
			{
				if (!is_iden<GenericContainer>(a)) 
				{ 
					std::cout << "Cannot at a non-container" << std::endl; 
					return nullptr; 
				}

				auto container = rein<GenericContainer>(a);
				return container->at(i);
			});

		import_function<_sgen_(_sgen_)>("-back", [](_sgen_ a)-> _sgen_
			{
				if (!is_iden<GenericContainer>(a))
				{
					std::cout << "Cannot at a non-container" << std::endl;
					return nullptr;
				}

				auto container = rein<GenericContainer>(a);
				return container->at(container->container_size() - 1);
			});

		import_function<size_t(_sgen_)>("-longsize", [](_sgen_ a)-> size_t
			{
				if (!is_iden<GenericContainer>(a)) { std::cout << "Cannot longsize a non-container" << std::endl; return 0; }

				auto container = rein<GenericContainer>(a);
				return container->container_size();
			});

		import_function<int(_sgen_)>("-size", [](_sgen_ a) -> int
			{
				if (!is_iden<GenericContainer>(a)) 
				{
					std::cout << "Cannot size a non-container" << std::endl; 
					return 0; 
				}

				auto container = rein<GenericContainer>(a);
				return (int)container->container_size();
			});

		import_function<_sgen_(_sgen_)>("@", [](_sgen_ a)-> _sgen_
			{
				if (!is_iden<GenericObject>(a)) { std::cout << "Cannot dereference a non-object!" << std::endl; return nullptr; }

				shared_ptr<GenericObject> obj = std::reinterpret_pointer_cast<GenericObject>(a);
				return obj->dereference();
			});

		import_function<_sgen_(_sgen_)>("&", [](_sgen_ a)-> _sgen_
			{
				if (!is_iden<GenericObject>(a)) { std::cout << "Cannot reference a non-object!" << std::endl; return nullptr; }

				shared_ptr<GenericObject> obj = std::reinterpret_pointer_cast<GenericObject>(a);
				return obj->reference();
			});

		import_function<int(_sgen_, _sgen_)>("-count", [](_sgen_ b, _sgen_ a) -> int
			{
				if (!is_iden<GenericContainer>(a))
				{
					std::cout << "Cannot size a non-container" << std::endl;
					return 0;
				}

				auto container = rein<GenericContainer>(a);
				int count = 0;

				for (size_t i = 0; i < container->container_size(); i++)
				{
					_sgen_ at = container->at(i);
					if (!is_iden<GenericObject>(at)) { return 0;  }

					auto obj_at = rein<GenericObject>(at);
					if(obj_at->equals(b)) count++;
				}

				return count;
			});

		import_function<bool(_sgen_, _sgen_)>("-has", [](_sgen_ b, _sgen_ a)
			{
				if (!is_iden<GenericContainer>(a))
				{
					std::cout << "Cannot size a non-container" << std::endl;
					return false;
				}

				auto container = rein<GenericContainer>(a);
				int count = 0;

				for (size_t i = 0; i < container->container_size(); i++)
				{
					_sgen_ at = container->at(i);
					if (!is_iden<GenericObject>(at)) { return false; }

					auto obj_at = rein<GenericObject>(at);
					if (obj_at->equals(b)) count++;
				}

				return (count > 0);
			});

		import_function<int(_sgen_, _sgen_)>("-find", [](_sgen_ b, _sgen_ a) -> int
			{
				if (!is_iden<GenericContainer>(a))
				{
					std::cout << "Cannot size a non-container" << std::endl;
					return -1;
				}

				auto container = rein<GenericContainer>(a);

				for (size_t i = 0; i < container->container_size(); i++)
				{
					_sgen_ at = container->at(i);
					if (!is_iden<GenericObject>(at)) { return -1; }

					auto obj_at = rein<GenericObject>(at);
					if (obj_at->equals(b)) return i;
				}

				return -1;
			});

		import_function<bool(_sgen_, _sgen_)>("-has_key", [](_sgen_ b, _sgen_ a) -> int
			{
				if (!is_iden<GenericContainer>(a))
				{
					std::cout << "Cannot has_key a non-container" << std::endl;
					return false;
				}

				auto container = rein<GenericContainer>(a);

				for (size_t i = 0; i < container->container_size(); i++)
				{
					_sgen_ at = container->at(i);
					//if (!is_iden<GenericObject>(at)) { return false; }

					if (!Global::get()->inspectors.count(at->type())) { return false; }

					_sgen_ key = Global::get()->inspectors.at(at->type()).inspect(at, "first");

					if (key == nullptr) { return false; }

					if (!is_iden<GenericObject>(key)) { return false; }

					auto key_obj = rein<GenericObject>(key);
					if (key_obj->equals(b)) return true;
				}

				return false;
			});

		import_function<int(_sgen_, _sgen_)>("-key_i", [](_sgen_ b, _sgen_ a) -> int
			{
				if (!is_iden<GenericContainer>(a))
				{
					std::cout << "Cannot key_i a non-container" << std::endl;
					return -1;
				}

				auto container = rein<GenericContainer>(a);

				for (size_t i = 0; i < container->container_size(); i++)
				{
					_sgen_ at = container->at(i);
					//if (!is_iden<GenericObject>(at)) { return false; }

					if (!Global::get()->inspectors.count(at->type())) { return false; }

					_sgen_ key = Global::get()->inspectors.at(at->type()).inspect(at, "first");

					if (key == nullptr) { return false; }

					if (!is_iden<GenericObject>(key)) { return false; }

					auto key_obj = rein<GenericObject>(key);
					if (key_obj->equals(b)) return i;
				}

				return -1;
			});

		import_function<_sgen_(_sgen_)>("-new_obj", [](_sgen_ a) -> _sgen_
			{
				if (!is_iden<GenericContainer>(a))
				{
					std::cout << "Cannot new_obj a non-container" << std::endl;
					return nullptr;
				}

				auto container = rein<GenericContainer>(a);

				return container->make_value();
			});

		import_function<void(_sgen_, _sgen_)>("~", [](_sgen_ b, _sgen_ a)
			{
				if (a->metaidentity() != typeid(MetaGeneric))
				{
					std::cout << "Cannot new_obj a non-container" << std::endl;
					return;
				}

				auto meta = rein<MetaGeneric>(a);

				meta->set_ptr(b);
			});
	}
}
