#pragma once
#include "ObjectGenerics.h"

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
struct is_container : std::integral_constant<bool, has_const_iterator<T>::value&& has_begin_end<T>::beg_value&& has_begin_end<T>::end_value>{ };

template<typename T>
struct remove_param_const { typedef T type; };

template<template<typename...> typename T, typename... I>
struct remove_param_const<T<I...>> { typedef T<std::remove_const_t<I>...> type; };

struct GenericContainer : public GenericObject
{
	virtual shared_generic at(size_t i) = 0;
	virtual void insert(shared_generic value, size_t i) = 0;
	virtual void erase(size_t i) = 0;
	virtual size_t container_size() = 0;
};

template<typename T>
class GenericContainerType;

template<typename T>
class GenericContainerRef : public GenericContainer
{
	//static_assert(is_container<T>::value);

	typename T::iterator iter_at(size_t i)
	{
		size_t iter = 0;
		for (auto it = container->begin(); it != container->end(); it++)
		{
			if (iter == i) return it;
			iter++;
		}
		return container->end();
	}

protected:
	T* container;

public:
	using value_type = typename remove_param_const<typename T::value_type>::type;

	GenericContainerRef() {}
	GenericContainerRef(T& copy) : container(&copy) {}
	~GenericContainerRef() {}

	char* raw_bytes() override { return (char*)(&(*container)); }
	const type_info& type() override
	{
		if constexpr (!is_type_complete_v<T>)
		{
			std::cout << "{type not complete}"; return typeid(void);
		}
		else return typeid(T);
	}
	const type_info& identity() override { return typeid(GenericContainer); }
	size_t size() override
	{

		if constexpr (!is_type_complete_v<T>)
		{
			std::cout << "{type not complete}"; return 0;
		}
		else return sizeof(T);
	}

	void set(shared_generic value)
	{
		if (value->type() != type()) { std::cout << "{set: types do not match}"; return; }
		if constexpr (!is_type_complete_v<T>) { std::cout << "{type not complete}"; return; }
		else if constexpr (std::is_abstract_v<T>) { std::cout << "{type is abract}"; return; }
		else if constexpr (std::is_copy_assignable_v<T>) *container = *(T*)(value->raw_bytes());
		else
		{
			std::cout << "not nothrow!!!" << std::endl;
		}
	}

	string stringify() override { return strings::stringify(*container); };
	int destringify(const string& str) override { return strings::destringify(*container, str); }

	bool equals(shared_generic gen) override
	{
		if (gen->type() != type()) return false;
		if constexpr (!operators::has_operator_equals<T, bool(T)>::value && !std::is_arithmetic_v<T>) { return false; }
		else return (*(T*)gen->raw_bytes() == *container);
	}

	shared_generic dereference() override;
	shared_generic reference() override { return std::make_shared<GenericContainerRef<T>>(*container); }

	shared_generic make() override;

	shared_generic at(size_t i) override
	{
		typename T::iterator it = iter_at(i);
		if (it != container->end()) return std::make_shared<GenericRef<typename T::value_type>>(&(*it));
		return nullptr;
	}

	size_t container_size() override
	{
		size_t iter = 0;
		for (auto it = container->begin(); it != container->end(); it++) iter++;
		return iter;
	}

	void insert(shared_generic value, size_t i) override
	{
		if (value->type() != typeid(value_type))
		{
			std::cout << "{ insert types do not match '" << value->type().name() << "' with container type '" << typeid(value_type).name() << "' }" << std::endl;
			return;
		}

		const value_type& cast = *(value_type*)value->raw_bytes();

		container->insert(iter_at(i), cast);
	};

	void erase(size_t i) override
	{
		container->erase(iter_at(i));
	};
};

template<typename T>
inline shared_generic GenericContainerRef<T>::dereference()
{
	if constexpr (!is_type_complete_v<T>) { return make_shared < GenericContainerRef<T>>(*this); }
	else if constexpr (is_pointer_v<T>) return make_shared < GenericRef<remove_pointer_t<T>>>(*this);
	else
	{
		return make_shared<GenericContainerType<T>>(*container);
	}
}

template<typename T>
inline shared_generic GenericContainerRef<T>::make() { return make_shared<GenericContainerRef<T>>(); }

template<typename T>
inline shared_ptr<GenericContainerRef<T>> make_generic_container_ref(T& ref)
{
	shared_ptr<GenericContainerRef<T>> ptr = std::make_shared<GenericContainerRef<T>>(ref);
	return ptr;
}

template<typename T>
class GenericContainerType : public GenericContainer
{
private:
	static_assert(is_container<T>::value);
	T container;

	typename T::iterator iter_at(size_t i)
	{
		size_t iter = 0;
		for (auto it = container.begin(); it != container.end(); it++)
		{
			if (iter == i) return it;
			iter++;
		}
		return container.end();
	}

public:
	using value_type = typename remove_param_const<typename T::value_type>::type;

	GenericContainerType() {}
	GenericContainerType(const T& container) : container(container) {}
	~GenericContainerType() {}

	char* raw_bytes() override { return (char*)(&container); }
	const type_info& type() override { return typeid(T); }
	const type_info& identity() override { return typeid(GenericContainer); }
	size_t size() override { return sizeof(T); }

	void set(shared_generic value)
	{
		if (value->type() != type()) return;
		container = *(T*)(value->raw_bytes());
	}

	string stringify() override { return strings::stringify(container); };
	int destringify(const string& str) override { return strings::destringify(container, str); }

	shared_generic make() override { return make_shared<GenericContainerType<T>>(); }

	shared_generic at(size_t i) override 
	{
		typename T::iterator it = iter_at(i);
		if (it != container.end()) return std::make_shared<GenericRef<typename T::value_type>>(&(*it));
		return nullptr;
	}

	size_t container_size() override 
	{ 
		size_t iter = 0;
		for (auto it = container.begin(); it != container.end(); it++) iter++;
		return iter;
	}

	void insert(shared_generic value, size_t i) override
	{
		if (value->type() != typeid(value_type)) return;

		const value_type& cast = *(value_type*)value->raw_bytes();

		container.insert(iter_at(i), cast);
	};

	shared_generic dereference() override
	{
		if constexpr (is_pointer_v<T>) return make_shared < GenericContainerRef<remove_pointer_t<T>>>(*container);
		else return make_shared<GenericContainerType<T>>(*this);
	}

	shared_generic reference() override
	{
		if constexpr (is_pointer_v<T>) return make_shared < GenericContainerType<T>>(container);
		else return make_shared<GenericType<T*>>(&container);
	}

	bool equals(shared_generic gen) override
	{
		if (gen->type() != type()) return false;
		if constexpr (!operators::has_operator_equals<T, bool(T)>::value && !std::is_arithmetic_v<T>) { return false; }
		else return (*(T*)gen->raw_bytes() == *container);
	}

	void erase(size_t i) override 
	{
		container.erase(iter_at(i));
	};
};