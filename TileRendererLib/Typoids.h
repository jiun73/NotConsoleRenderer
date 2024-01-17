#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#include <iostream>
#include <array>
#include <map>
#include <tuple>
#include <variant>
#include <cassert>
#include <typeindex>
//#include "Functions.h"
#include "Stringify.h"

/*
* Typoids :
* Type as in 'Typing' and 'Semi-defined Type'.
* Yes, this is confusing, but what they essentially do is add the ability to remove strong types
* we can then edit the values of the classes using only strings
* using AP22::stringify and AP22::destringify (you can then add template specializations to these to make your own translations)
* It can store values, and pointers (ObjectTypoid)
* Or even functions (FunctionTypoids)
*
*/

using std::string;
using std::vector;
using std::array;
using std::tuple;
using std::cout;
using std::endl;
using std::type_index;
using std::function;
using std::remove_reference;
using std::conditional_t;
using std::shared_ptr;

template<typename T>
struct has_const_iterator
{
private:
	typedef char                      yes;
	typedef struct { char array[2]; } no;

	template<typename C> static yes test(typename C::const_iterator*);
	template<typename C> static no  test(...);
public:
	static const bool value = sizeof(test<T>(0)) == sizeof(yes);
	typedef T type;
};

template <typename T>
struct has_begin_end
{
	template<typename C> static char(&f(typename std::enable_if<
		std::is_same<decltype(static_cast<typename C::const_iterator(C::*)() const>(&C::begin)),
		typename C::const_iterator(C::*)() const>::value, void>::type*))[1];

	template<typename C> static char(&f(...))[2];

	template<typename C> static char(&g(typename std::enable_if<
		std::is_same<decltype(static_cast<typename C::const_iterator(C::*)() const>(&C::end)),
		typename C::const_iterator(C::*)() const>::value, void>::type*))[1];

	template<typename C> static char(&g(...))[2];

	static bool const beg_value = sizeof(f<T>(0)) == 1;
	static bool const end_value = sizeof(g<T>(0)) == 1;
};

template<typename T>
struct is_container : std::integral_constant<bool, has_const_iterator<T>::value&& has_begin_end<T>::beg_value&& has_begin_end<T>::end_value>
{ };

template<typename T, typename D = void>
struct is_map : std::false_type {};

template<typename T1, typename T2>
struct is_map<std::map<T1, T2>> : std::true_type {};

template<typename T, typename D = void>
struct is_pair : std::false_type {};

template<typename T1, typename T2>
struct is_pair<std::pair<T1, T2>> : std::true_type {};

template<typename T>
struct is_string : std::false_type {};

template<>
struct is_string<std::string> : std::true_type {};

template<typename T1, typename T2 = void>
struct pair_remove_const { typedef T1 type; };

template<typename T1, typename T2>
struct pair_remove_const<std::pair<T1, T2>> { typedef std::pair<std::remove_const_t<T1>, std::remove_const_t<T2>> type; };

template<int ...> struct seq {};

template<int N, int ...S> struct gens : gens<N - 1, N - 1, S...> { };

template<int ...S> struct gens<0, S...> { typedef seq<S...> type; };

template <typename R, typename ...Args>
struct save_it_for_later
{
	std::tuple<typename std::remove_reference<Args>::type*...> params;
	std::function<R(Args...)> func;

	R delayed_dispatch()
	{
		return callFunc(typename gens<sizeof...(Args)>::type()); // Item #1
	}

	template<int ...S>
	R callFunc(seq<S...>)
	{
		return func((*std::get<S>(params)) ...);
	}
};

enum TypoidErrors
{
	TYPOID_IS_NULL = -2,
	TYPOID_NO_TRANSLATION,
	TYPOID_INVALID_INPUT,
	TYPOID_SUCCESS
};

class Typoid
{
private:

public:
	bool temp = false;

	Typoid() {}
	virtual ~Typoid() {}

	//checks
	virtual bool null() { return true; }
	virtual bool isFunc() { return false; }
	virtual bool isObj() { return false; }
	virtual bool isPtr() { return false; }
	virtual bool isContainer() { return false; }
	virtual bool isNum() { return false; }

	//raw
	virtual char* raw() { return nullptr; }
	virtual char* obj() { return nullptr; }
	virtual Typoid* safe() { return nullptr; }

	//manip
	virtual string get() { return "{empty}"; }
	virtual int edit(const string& s) { return TYPOID_IS_NULL; }
	virtual shared_ptr<Typoid> dereference() { return nullptr; }
	virtual void set(shared_ptr<Typoid> value) {}
	virtual void set_strict(shared_ptr<Typoid> value) {}

	//type
	virtual string type_name() { return "{error type}"; }
	virtual type_index type() { return typeid(void); }

	//function typoids
	virtual string call() { return ""; }
	virtual shared_ptr<Typoid> get_return() { return nullptr; }
	virtual int edit_args(const vector<string>& args) { return TYPOID_IS_NULL; }
	virtual int args(vector<shared_ptr<Typoid>> args) { return TYPOID_IS_NULL; }
	virtual int args_count() { return 0; }

	//container typoids
	virtual string get_at(size_t id) { return "{not a container}"; }
	virtual shared_ptr<Typoid> ptr_at(size_t id) { return nullptr; }
	virtual shared_ptr<Typoid> map_at(shared_ptr<Typoid> id) { return nullptr; }
	virtual int edit_at(const string& s, size_t id) { return TYPOID_IS_NULL; }
	virtual shared_ptr<Typoid> pair_first() { return nullptr; }
	virtual shared_ptr<Typoid> pair_second() { return nullptr; }
	virtual size_t size() { return 0; }
	virtual void insert(shared_ptr<Typoid> val, size_t index) { }
	virtual void erase(size_t index) { }

	//factory
	virtual shared_ptr<Typoid> make() { return std::make_shared<Typoid>(); }
	virtual shared_ptr<Typoid> make_ptr() { return std::make_shared<Typoid>(); }

	//operators
	virtual shared_ptr<Typoid> operator_plus(shared_ptr<Typoid> other) { return nullptr; }
};

template <size_t I = 0, typename... Ts>typename std::enable_if<I == sizeof...(Ts), void>::type
getClassNames(vector<string>& list)
{
	return;
}
template <size_t I = 0, typename... Ts> typename std::enable_if<(I < sizeof...(Ts)), void>::type
	getClassNames(vector<string>& list)
{
	//typeid(std::tuple_element_t<I, std::tuple<Ts...>>).name()
	// TODO: fix?
	//list.push_back(AP22::type_name_abbreviated<std::tuple_element_t<I, std::tuple<Ts...>>>());
	getClassNames<I + 1, Ts...>(list);
}

//#include "Operators.h"

template<typename T>
class ObjectTypoid : public Typoid
{
private:
	std::remove_const_t<T> object = T(); //The object with const removed

public:
	ObjectTypoid();
	ObjectTypoid(T obj) : object(obj) {}
	virtual ~ObjectTypoid() {}

	T& get_strict() { return object; }

	void set_strict(shared_ptr<Typoid> value)
	{
		if (value->type() != type())
		{
			std::cout << "{types do not match}";
			std::cout << "{" << value->type_name() << " should be " << type_name() << "}";
			return;
		}

		if constexpr (std::is_nothrow_copy_assignable<T>::value)
		{

			object = *(T*)(value->raw());
		}
		else
		{
			std::cout << "{could not assign}";
		}

	}

	void set(shared_ptr<Typoid> value)
	{
		using no_ptr = std::remove_const_t<std::remove_pointer_t<T>>;

		if (value->type() != type())
		{
			std::cout << "{types do not match}";
			std::cout << "{" << value->type_name() << " should be " << type_name() << "}";
			return;
		}

		if constexpr (std::is_nothrow_copy_assignable<no_ptr>::value)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				*object = *(T)(value->raw());
			}
			else
			{
				object = *(T*)(value->raw());
			}
		}
		else
		{
			std::cout << "{could not assign}";
		}

	}

	string get_at(size_t id) override
	{
		return "{not a container}";
	}

	shared_ptr<Typoid> ptr_at(size_t id) override
	{
		using no_ptr = std::remove_const_t<std::remove_pointer_t<T>>;
		if constexpr (is_container<no_ptr>::value)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				if constexpr (std::is_same_v<string, no_ptr>) //I've tried everything, and I can't get insert to work with string, so fuck this
				{
					return nullptr;
				}
				else
				{
					size_t i = 0;
					for (auto it = object->begin(); it != object->end(); it++)
					{
						if (i == id)
						{
							return std::make_shared<ObjectTypoid<typename no_ptr::value_type*>>(&(*it));
						}
						i++;
					}
					cout << "{out of range! size: " << i << "}";
					return nullptr;
				}
			}
			else
			{
				size_t i = 0;
				for (auto it = object.begin(); it != object.end(); it++)
				{
					if (i == id)
					{
						return std::make_shared<ObjectTypoid<typename T::value_type*>>(&(*it));
					}
					i++;
				}
				cout << "{out of range! size: " << i << "}";
				return nullptr;
			}
		}
		else
		{
			cout << "{not a container}";
			return nullptr;
		}
	}

	int edit_at(const string& s, size_t id) override
	{
		return TYPOID_IS_NULL;
	}

	shared_ptr<Typoid> pair_first() override
	{
		if constexpr (is_pair<T>::value)
		{
			return std::make_shared<ObjectTypoid<typename T::first_type*>>(&(object.first));;
		}
		else
		{
			cout << "{not a pair}";
			return nullptr;
		}
	}
	shared_ptr<Typoid> pair_second() override
	{
		if constexpr (is_pair<T>::value)
		{
			return std::make_shared<ObjectTypoid<typename T::second_type*>>(&(object.second));
		}
		else
		{
			cout << "{not a pair}";
			return nullptr;
		}
	}

	shared_ptr<Typoid> map_at(shared_ptr<Typoid> id) override
	{
		if constexpr (is_container<T>::value)
		{
			if constexpr (is_map<T>::value)
			{
				if (id->type() == type_index(typeid(typename T::key_type)))
				{
					typename T::key_type* key = (typename T::key_type*)(id->raw());

					if (object.count(*key))
						return std::make_shared<ObjectTypoid<typename T::mapped_type*>>(&object.at(*key));

					cout << "{invalid key!}";
					return nullptr;
				}
				cout << "{wrong key type}";
				return nullptr;
			}
		}
		cout << "{not a map " << type_name() << "}";
		return nullptr;
	}
	size_t size() override
	{
		using no_ptr = std::remove_const_t<std::remove_pointer_t<T>>;
		if constexpr (is_container<no_ptr>::value)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				return object->size();
			}
			else
			{
				return object.size();
			}
		}
		return 0;
	}

	void insert(shared_ptr<Typoid> val, size_t index) override
	{
		using no_ptr = std::remove_const_t<std::remove_pointer_t<T>>;

		if constexpr (std::is_same_v<string, no_ptr>) //I've tried everything, and I can't get insert to work with string, so fuck this
		{
			return;
		}
		else
		{

			if constexpr (is_container<no_ptr>::value)
			{
				if (val->type() != type_index(typeid(pair_remove_const<typename no_ptr::value_type>::type)))
				{
					cout << "{wrong value type " << val->type_name() << " expected " << typeid(typename no_ptr::value_type).name() << "}";
					return;
				}


				typename no_ptr::value_type* value = (typename no_ptr::value_type*)val->raw();
				if constexpr (std::is_pointer_v<T>)
				{
					auto it = object->begin();

					for (size_t i = 0; i < index; i++)
						it++;
					object->insert(it, *value);
				}
				else
				{
					auto it = object.begin();
					for (size_t i = 0; i < index; i++)
						it++;
					object.insert(it, *value);
				}
				return;


			}
		}
		cout << "{not a container}";
		return;
	}

	void erase(size_t index) override
	{
		using no_ptr = std::remove_const_t<std::remove_pointer_t<T>>;
		if constexpr (std::is_same_v<string, no_ptr>) //I've tried everything, and I can't get insert to work with string, so fuck this
		{
			return;
		}
		else
			if constexpr (is_container<no_ptr>::value)
			{
				if constexpr (std::is_pointer_v<T>)
				{
					auto it = object->begin();
					for (size_t i = 0; i < index; i++)
						it++;
					object->erase(it);
					return;
				}
				else
				{
					auto it = object.begin();
					for (size_t i = 0; i < index; i++)
						it++;
					object.erase(it);
					return;
				}
			}
		cout << "{not a container}";
		return;
	}

	bool isObj() override { return true; }
	bool isPtr()	override { return std::is_pointer<T>::value; }
	bool isNum()	override { return std::is_arithmetic<T>::value; }
	bool isContainer() override { return is_container<T>::value; }
	bool null()	override;
	char* raw()		override;
	char* obj()		override;
	string		type_name()	override { return typeid(T).name(); }
	type_index	type()		override { return typeid(T); }

	string		get()					override;
	int			edit(const string& s)	override;

	shared_ptr<Typoid> dereference()
	{
		if constexpr (std::is_pointer_v<T>)
			return std::make_shared < ObjectTypoid<std::remove_pointer_t<T>>>(*object);
		else
			return std::make_shared < ObjectTypoid<T>>(object);;
	}

	Typoid* safe() override { return this; }
	shared_ptr<Typoid> make() override { return std::make_shared<ObjectTypoid<T>>(); }
	shared_ptr<Typoid> make_ptr() override
	{
		if constexpr (std::is_pointer_v<T>)
		{
			return std::make_shared<ObjectTypoid<T>>();
		}
		else
			return std::make_shared<ObjectTypoid<T*>>();
	}// gets the compiler stuck in an infinite loop?

};

template<typename T>
class FactoryTypoid : public Typoid
{
public:
	shared_ptr<Typoid> make() override { return std::make_shared<ObjectTypoid<T>>(); }
};

template<typename T>
class FunctionTypoid;

template<typename R, typename... Args>
class FunctionTypoid<std::function<R(Args...)>> : public Typoid
{
private:
	tuple<typename remove_reference<Args>::type*...> tuple = {}; //Arguments in ptr form
	array<shared_ptr<Typoid>, sizeof...(Args)> arguments = {}; //Arguments for the function in Typoid form
	function<R(Args...)> function = nullptr; //Function
	class monostate {};
	conditional_t<std::is_same_v<R, void> == false, shared_ptr<Typoid>, monostate> ret; //Optionnal return type 
	bool args_was_set = false;

	void cleanArgs();

public:
	FunctionTypoid(std::function<R(Args...)> function) : function(function) {}
	~FunctionTypoid() {}

	bool		null()			override { return (function == nullptr); }
	bool		isFunc()		override { return true; }
	char* raw()			override { return (char*)(&function); }
	char* obj()			override { return nullptr; }
	string		type_name()		override;
	type_index	type()			override { return typeid(R); }
	string		get()			override { return "{function}"; }
	string		call()			override;

	shared_ptr<Typoid> get_return() override 
	{
		if constexpr (std::is_same_v<void, R>)
		{
			return nullptr;
		}
		else
		{
			return ret;
		}
	}

	template<size_t I, typename T>
	T* getTuple()
	{
		return (T*)(arguments.at(I)->raw());
	} //Returns the the Ith element of a tuple and casts it to T

	template <size_t I = 0> //Set ptr forms to point to the objects in Typoids of arguments
	inline void setTuple()
	{
		if constexpr (I < sizeof...(Args))
		{
			using tuple_type = std::tuple<typename std::remove_reference<Args>::type...>;

			if constexpr (std::is_same_v<std::tuple_element_t<I, tuple_type>, shared_ptr<Typoid>>)//if arg is shared ptr, return the ptr 
			{
				std::get<I>(tuple) = &arguments.at(I);
			}
			else //otherwise return the value inside the ptr
			{
				std::get<I>(tuple) = getTuple<I, std::tuple_element_t<I, tuple_type>>();
			}
			setTuple<I + 1>();
		}
	}

	template<size_t I = 0>
	inline std::pair<size_t, int> setArgs(const std::vector<shared_ptr<Typoid>>& arg)
	{
		if constexpr (I < sizeof...(Args))
		{
			shared_ptr<Typoid> current = arg.at(I);

			if (current == nullptr) //ignore null typoids;
				return setArgs<I + 1>(arg);

			if (current->type() != type_index(typeid(typename std::tuple_element_t<I, std::tuple<Args...>>)) && typeid(typename std::tuple_element_t<I, std::tuple<Args...>>) != typeid(shared_ptr<Typoid>)) //if arg is shared Typoid, then set it regardless of type inside current
			{
				std::cout << "{arg invalid (should be " << typeid(typename std::tuple_element_t<I, std::tuple<Args...>>).name() << ")}";
				return { I,TYPOID_INVALID_INPUT };
			}

			arguments.at(I) = arg.at(I);
			return setArgs<I + 1>(arg);
		}
		return { I,TYPOID_SUCCESS };
	}

	template<size_t I = 0>
	inline std::pair<size_t, int> makeAt(const std::vector<std::string>& args)
	{
		if constexpr (I < sizeof...(Args))
		{
			using tuple_type = std::tuple<typename std::remove_reference<Args>::type...>;
			arguments.at(I) = std::make_shared<ObjectTypoid<std::tuple_element_t<I, tuple_type>>>();
			arguments.at(I)->temp = true;
			int log = arguments.at(I)->edit(args.at(I));
			if (log == TYPOID_SUCCESS)
				return makeAt<I + 1>(args);
			else
				return { I,log };
		}
		return { I,TYPOID_SUCCESS };
	}

	int edit_args(const std::vector<std::string>& args) override
	{
		if (null())
			return TYPOID_IS_NULL;

		if (args.size() != arguments.size())
		{
			std::cout << "{wrong arg number}";
			args_was_set = false;
			return TYPOID_INVALID_INPUT;
		}

		std::pair<size_t, int> log = makeAt(args);
		if (log.second != TYPOID_SUCCESS)
		{
			std::cout << "{error found at I = " << log.first << "}";
			args_was_set = false;
			return log.second;
		}

		args_was_set = true;
		return TYPOID_SUCCESS;
	}

	int args_count() override { return sizeof...(Args); }

	int args(std::vector<shared_ptr<Typoid>> args) override
	{
		if (null())
			return TYPOID_IS_NULL;

		if (args.size() != arguments.size())
		{
			std::cout << "{wrong arg number (was " << args.size() << ")}";
			args_was_set = false;
			return TYPOID_INVALID_INPUT;
		}

		std::pair<size_t, int> log = setArgs(args);

		if (log.second != TYPOID_SUCCESS)
		{
			args_was_set = false;
			std::cout << "{error found at I = " << log.first << "}";
			return log.second;
		}
		else
		{
			args_was_set = true;
			return log.second;
		}
	}

	Typoid* safe() override {
		if constexpr (std::is_same_v<R, void> == false)
		{
			return ret.get();
		}
		return nullptr;
	}
};

template<typename T>
class StrictTypoid
{
private:
	std::shared_ptr<Typoid> ptr = nullptr;

	void clean()
	{
		ptr.reset();
	}

public:
	StrictTypoid() {}
	StrictTypoid(const T& obj) { get() = obj; }
	StrictTypoid(const StrictTypoid<T>& obj) { ptr = obj.ptr; }
	~StrictTypoid() { clean(); }

	string stringify()
	{
		if (ptr == nullptr)
			return "";

		return ptr->get();
	}

	int try_edit(const string& str)
	{
		get(); //sets up if non-existant
		return ptr->edit(str);
	}

	//return TRUE if succesfull, otherwise FALSE
	bool set(std::shared_ptr<Typoid> t)
	{
		if (t->type() != type_index(typeid(T)))
			return false;

		if (t->isFunc())
			return false;

		if (t->null())
			return false;

		clean();

		ptr = t;

		return true;
	}

	T& get()
	{
		if (ptr == nullptr)
		{
			ptr = std::make_shared< ObjectTypoid<T>>();
		}
		return *(T*)ptr->obj();
	}

	string str()
	{
		if (ptr == nullptr)
			return "";
		return ptr->get();
	}

	T& operator() ()
	{
		return get();
	}

	StrictTypoid<T>& operator=(const T& o)
	{
		get() = o;
		return *this;
	}
	StrictTypoid<T>& operator=(const StrictTypoid<T>& o)
	{
		ptr = o.ptr;
		return *this;
	}
};

#include "ClassFactory.h"

template<typename T>
inline ObjectTypoid<T>::ObjectTypoid()
{
	//object = T();
	//ClassFactory::get()->add<T>(type_name());
}

template<typename T>
inline bool ObjectTypoid<T>::null()
{
	if constexpr (std::is_pointer<T>::value)
		return (object == nullptr);
	else
		return false;
}

template<typename T>
inline char* ObjectTypoid<T>::raw()
{
	return (char*)(&object);
}

template<typename T>
inline char* ObjectTypoid<T>::obj()
{
	if constexpr (std::is_pointer<T>::value)
	{
		return (char*)(object);
	}
	else
		return (char*)(&object);
}

template<typename T>
inline string ObjectTypoid<T>::get()
{
	if constexpr (std::is_pointer<T>::value)
	{
		return strings::stringify(*object);
	}
	else
		return strings::stringify(object);
}

template<typename T>
inline int ObjectTypoid<T>::edit(const string& s)
{
	int log;
	if constexpr (std::is_pointer<T>::value)
	{
		typedef std::remove_pointer<T>::type B;
		if (null())
			object = new B();

		log = strings::destringify(*object, s);
	}
	else
		log = strings::destringify(object, s);


	if (log == TYPOID_INVALID_INPUT)
		std::cout << "{invalid string}";
	else if (log == TYPOID_NO_TRANSLATION)
		std::cout << "{no translation}";

	return log;
}

template<typename R, typename ...Args>
inline void FunctionTypoid<std::function<R(Args...)>>::cleanArgs()
{
	//for (auto& args : arguments) if (args->temp) delete args;
}

template<typename R, typename ...Args>
inline string FunctionTypoid<std::function<R(Args...)>>::type_name()
{
	std::stringstream output;
	vector<string> list;
	output << typeid(R).name() << "(";
	getClassNames<0, Args...>(list);
	size_t i = 0;
	for (auto& name : list)
	{
		output << name << ((i == list.size() - 1) ? "" : ", ");
		i++;
	}
	output << ")";
	return output.str();
}

template<typename R, typename ...Args>
inline string FunctionTypoid<std::function<R(Args...)>>::call()
{
	if (null())
		return "{function null}";

	if (!args_was_set)
		return "{arguments not set}";

	setTuple();

	save_it_for_later<R, Args...> saved = { tuple, function };
	//assert(0);
	if constexpr (std::is_same_v<R, void> == true)
	{
		saved.delayed_dispatch();
		return "{void}";
	}
	else
	{
		if constexpr (std::is_same_v<shared_ptr<Typoid>, R>)
		{
			ret = saved.delayed_dispatch();
			if (ret != nullptr)
				return ret->get();
		}
		else
		{
			ret = std::make_shared<ObjectTypoid<R>>(saved.delayed_dispatch());
			if (ret != nullptr)
				return ret->get();
		}
	}

	cleanArgs();
	return "";
}
