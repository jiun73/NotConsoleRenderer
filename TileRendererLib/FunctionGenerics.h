#pragma once
#include "Generics.h"
#include "ObjectGenerics.h"
#include <functional>

using std::function;
using std::vector;
using std::tuple;
using std::conditional_t;
using std::is_same_v;
using std::remove_reference_t;
using std::tuple;

template<int ...> struct seq {};

template<int N, int ...S> struct gens : gens<N - 1, N - 1, S...> { };

template<int ...S> struct gens<0, S...> { typedef seq<S...> type; };

template <typename R, typename ...Args>
struct save_it_for_later
{
	tuple<typename std::remove_reference<Args>::type*...> params;
	function<R(Args...)> func;

	R delayed_dispatch() { return call_func(typename gens<sizeof...(Args)>::type()); }

	template<int ...S>
	R call_func(seq<S...>) { return func((*std::get<S>(params)) ...); }
};

struct GenericArgument 
{
	bool is_string = false;
	shared_generic value = nullptr;
	string str;

	GenericArgument(shared_generic value) : value(value), is_string(false) {}
	template<typename T>
	GenericArgument(shared_ptr<GenericType<T>> value) : value(value), is_string(false) {}
	GenericArgument(string string) : str(string), is_string(true) {}
	~GenericArgument() {}
};

struct GenericFunction : public Generic
{
	virtual shared_generic call() = 0;
	virtual void args(const vector<GenericArgument>& values) = 0;
	virtual const type_info& args_type(size_t i) = 0;
	virtual size_t arg_count() = 0;
	virtual char* function_bytes() = 0;
	virtual const type_info& return_type() = 0;
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

	template<size_t I, typename T>
	T* get_tuple() //Returns the the Ith element of a tuple and casts it to T
	{
		return (T*)(arguments.at(I)->raw_bytes());
	} 

	template <size_t I = 0> //Set ptr forms to point to the objects in Typoids of arguments
	inline void set_tuple()
	{
		if constexpr (I < sizeof...(Args))
		{
			using tuple_type = std::tuple<typename std::remove_reference<Args>::type...>;

			if constexpr (std::is_same_v<std::tuple_element_t<I, tuple_type>, shared_generic>)//if arg is shared ptr, return the ptr 
			{
				std::get<I>(tuple) = &arguments.at(I);
			}
			else //otherwise return the value inside the ptr
			{
				std::get<I>(tuple) = get_tuple<I, std::tuple_element_t<I, tuple_type>>();
			}
			set_tuple<I + 1>();
		}
	}

	template<size_t I = 0>
	inline void set_args(const vector<GenericArgument>& arg)
	{
		if constexpr (I < sizeof...(Args))
		{
			shared_generic current;
			if (arg.at(I).is_string)
				current = make_generic_from_string<std::tuple_element_t<I, std::tuple<std::remove_reference<Args>::type...>>>(arg.at(I).str);
			else
				current = arg.at(I).value;

			if (current == nullptr) //ignore null typoids;
			{
				set_args<I + 1>(arg);
				return;
			}

			const type_info& info = typeid(typename std::tuple_element_t<I, std::tuple<Args...>>);

			if (current->type() != info && info != typeid(shared_generic)) //if arg is shared Typoid, then set it regardless of type inside current
			{
				std::cout << "{arg invalid (should be " << info.name() << ")}";
				//return { I,TYPOID_INVALID_INPUT };
			}

			arguments.at(I) = current;
			set_args<I + 1>(arg);
		}
		//return { I,TYPOID_SUCCESS };
	}

public:
	GenericFunctionType(function<R(Args...)> function) : _callback_(function) {}
	virtual ~GenericFunctionType() {}

	char* raw_bytes() override {
		return nullptr;//return _return_->raw_bytes(); 
	}
	const type_info& type() override { return typeid(R(Args...)); }
	const type_info& return_type() override { return typeid(R); }
	const type_info& identity() override { return typeid(GenericFunction); }
	size_t size() override { return 0; }

	template <size_t I = 0>
	const type_info& unfold_args_type(size_t i)
	{
		if constexpr (I < sizeof...(Args))
		{
			if (I == i)
			{
				return typeid(typename std::tuple_element_t<I, std::tuple<Args...>>);
			}
			else
				return unfold_args_type<I + 1>(i);
		}

		return typeid(void);
	}

	const type_info& args_type(size_t i) override
	{
		return unfold_args_type(i);
	}

	void set(shared_generic value) override
	{
		if (value->type() != type()) { std::cout << "{set: types do not match}"; return; }
		if (value->identity() != typeid(GenericFunction)) { std::cout << "{set: value is not a function"; return; }

		 shared_ptr<GenericFunction> reinterpret = std::reinterpret_pointer_cast<GenericFunction>(value);

		_callback_ = *(function<R(Args...)>*)(reinterpret->function_bytes());
	}

	string stringify() override { return ""; };
	int destringify(const string& str) override { return -2; }

	shared_generic make() override { return make_shared<GenericFunctionType<function<R(Args...)>>>(*this); }

	shared_generic call() override 
	{
		set_tuple<0>();

		save_it_for_later<R, Args...> saved = { tuple, _callback_ };

		if constexpr (is_same_v<R, void> == true)
		{
			saved.delayed_dispatch();
			return nullptr;
		}
		else
		{
			if constexpr (is_same_v<shared_generic, R>)
				_return_ = saved.delayed_dispatch();
			else
				_return_ = std::make_shared<GenericType<R>>(saved.delayed_dispatch());
			return _return_;
		}

		return nullptr;
	}

	void args(const vector<GenericArgument>& values) override
	{
		set_args(values);
	}

	size_t arg_count()
	{
		return sizeof...(Args);
	}

	char* function_bytes() override { return (char*)(&(_callback_)); }
};