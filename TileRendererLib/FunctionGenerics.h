#pragma once
#include "Generics.h"
#include <functional>

using std::function;
using std::vector;
using std::tuple;
using std::conditional_t;
using std::is_same_v;
using std::remove_reference_t;
using std::tuple;

struct GenericFunction : public Generic
{
	virtual shared_generic call() = 0;
	virtual void args(const vector<shared_generic>& values) = 0;
	virtual char* function_bytes() = 0;
};

template<typename T>
class GenericFunctionType;

template<typename R, typename... Args>
class GenericFunctionType<function<R(Args...)>> : public GenericFunction
{
private:
	tuple<typename std::remove_reference<Args>::type*...> tuple = {}; //Arguments in ptr form
	array<shared_generic, sizeof...(Args)> arguments = {}; //Arguments for the function in Typoid form
	function<R(Args...)> _callback_ = nullptr; //Function
	class monostate {};
	conditional_t<is_same_v<R, void> == false, shared_generic, monostate> _return_; //Optionnal return type 
	bool args_was_set = false;

	template<size_t I = 0>
	inline void set_args(const vector<shared_generic>& arg)
	{
		if constexpr (I < sizeof...(Args))
		{
			shared_generic current = arg.at(I);

			if (current == nullptr) //ignore null typoids;
				return set_args<I + 1>(arg);

			if (current->type() != typeid(typename std::tuple_element_t<I, tuple<Args...>>) && typeid(typename std::tuple_element_t<I, tuple<Args...>>) != typeid(shared_generic)) //if arg is shared Typoid, then set it regardless of type inside current
			{
				std::cout << "{arg invalid (should be " << typeid(typename tuple_element_t<I, tuple<Args...>>).name() << ")}";
				return { I,TYPOID_INVALID_INPUT };
			}

			arguments.at(I) = arg.at(I);
			return set_args<I + 1>(arg);
		}
		return { I,TYPOID_SUCCESS };
	}

public:
	GenericFunctionType(function<R(Args...)> function) : _callback_(function) {}
	virtual ~GenericFunctionType() {}

	char* raw_bytes() override {
		return nullptr;//return _return_->raw_bytes(); 
	}
	const type_info& type() override { return typeid(R(Args...)); }
	const type_info& identity() override { return typeid(GenericFunction); }
	size_t size() override { return 0; }

	void set(shared_generic value) override
	{
		if (value->type() != type()) { std::cout << "{set: types do not match}"; return; }
		if (value->identity() != typeid(GenericFunction)) { std::cout << "{set: value is not a function"; return; }

		 shared_ptr<GenericFunction> reinterpret = std::reinterpret_pointer_cast<GenericFunction>(value);

		_callback_ = *(function<R(Args...)>*)(reinterpret->function_bytes());
	}

	string stringify() override { return ""; };
	void destringify(const string& str) override {  }

	shared_generic make() override { return make_shared<GenericFunctionType<function<R(Args...)>>>(*this); }

	shared_generic call() override {}
	void args(const vector<shared_generic>& values) override 
	{

	}
	char* function_bytes() override { return (char*)(&(_callback_)); }
};