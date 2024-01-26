#pragma once
#include "Generics.h"
#include <functional>

using std::function;
using std::tuple;
using std::conditional_t;
using std::is_same_v;
using std::remove_reference_t;

template<typename T>
class FunctionGeneric;

template<typename R, typename... Args>
class FunctionGeneric<function<R(Args...)>> : public Generic
{
private:
	tuple<typename remove_reference_t<Args>*...> tuple = {}; //Arguments in ptr form
	array<shared_generic, sizeof...(Args)> arguments = {}; //Arguments for the function in Typoid form
	function<R(Args...)> _callback_ = nullptr; //Function
	class monostate {};
	conditional_t<is_same_v<R, void> == false, shared_generic, monostate> _return_; //Optionnal return type 
	bool args_was_set = false;

public:
	FunctionGeneric(function<R(Args...)> function) : _callback_(function) {}
	virtual ~FunctionGeneric() {}

	char* raw_bytes() override { return _return_->raw_bytes(); }
	const type_info& type() override { return typeid(R(Args...)); }
	size_t size() override { return 0; }

	void set(shared_generic value)
	{
		if (value->type() != type()) { std::cout << "{set: types do not match}"; return; }
		*_callback_ = *(function<R(Args...)>*)(value->function_bytes());
	}

	string stringify() override { return strings::stringify(*_object_); };
	void destringify(const string& str) override { strings::destringify(*_object_, str); }

	shared_generic dereference() override {}
	shared_generic make() override { return make_shared<FunctionGeneric<function<R(Args...)>>>(*this); }

	bool is_function() override { return true; }
	char* function_bytes() override { return (char*)(&(*_callback_)); }
};