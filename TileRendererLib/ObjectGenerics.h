#pragma once
#include "Generics.h"
#include "Operators.h"

template<typename, typename = void>
constexpr bool is_type_complete_v = false;

template<typename T>
constexpr bool is_type_complete_v
<T, std::void_t<decltype(sizeof(T))>> = true;

struct GenericObject : public Generic
{
	virtual shared_generic dereference() = 0;
	virtual shared_generic reference() = 0;
	virtual bool equals(shared_generic) = 0;
};

template<typename T>
class GenericType;

template<typename T>
class GenericRef : public GenericObject
{
private:
	T* _object_;

public:
	GenericRef() {}
	GenericRef(T& copy) : _object_(&copy) {}
	~GenericRef() {}

	char* raw_bytes() override { return (char*)(&(*_object_)); }
	const type_info& type() override 
	{ 
		if constexpr (!is_type_complete_v<T>)
		{
			std::cout << "{type not complete}"; return typeid(void);
		}
		else return typeid(T);
	}
	const type_info& identity() override { return typeid(GenericObject); }
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
		else if constexpr (std::is_copy_assignable_v<T>) *_object_ = *(T*)(value->raw_bytes());
		else
		{
			std::cout << "not nothrow!!!" << std::endl;
		}
	}

	string stringify() override { return strings::stringify(*_object_); };
	int destringify(const string& str) override { return strings::destringify(*_object_, str); }

	bool equals(shared_generic gen) override
	{
		if (gen->type() != type()) return false;
		if constexpr (!operators::has_operator_equals<T, bool(T)>::value && !std::is_arithmetic_v<T>) { return false; }
		else return (*(T*)gen->raw_bytes() == *_object_);
	}

	shared_generic dereference() override;
	shared_generic reference() override { return std::make_shared<GenericRef<T>>(*_object_); }

	shared_generic make() override;
};

template<typename T>
class GenericType : public GenericObject
{
private:
	T _object_;

public:
	GenericType() {}
	GenericType(const T& copy) : _object_(copy) {}
	~GenericType() {}

	char* raw_bytes() override { return (char*)(&_object_); }
	const type_info& type() override { return typeid(T); }
	const type_info& identity() override { return typeid(GenericObject); }
	size_t size() override { return sizeof(T); }

	void set(shared_generic value)
	{
		if (value->type() != type()) 
		{ 
			std::cout << "{set: types do not match }" << value->type().name() << " " << type().name() << std::endl; 
			return; 
		}
		if constexpr (!is_type_complete_v<T>) { std::cout << "{type not complete}"; return; }
		else if constexpr (std::is_abstract_v<T>) { std::cout << "{type is abract}"; return; }
		else if constexpr (std::is_copy_assignable_v<T>) _object_ = *(T*)(value->raw_bytes());
		else
		{
			std::cout << "not nothrow!" << std::endl;
		}
	}

	string stringify() override { return strings::stringify(_object_); };
	int destringify(const string& str) override { return strings::destringify(_object_, str); }

	shared_generic dereference() override
	{
		if constexpr (is_pointer_v<T>) return make_shared < GenericRef<remove_pointer_t<T>>>(*_object_);
		else return make_shared<GenericType<T>>(*this);
	}

	shared_generic reference() override
	{
		if constexpr (is_pointer_v<T>) return make_shared < GenericType<T>>(_object_);
		else return make_shared<GenericType<T*>>(&_object_);
	}

	bool equals(shared_generic gen) override
	{
		if (gen->type() != type()) return false;
		if constexpr (!operators::has_operator_equals<bool, T, T>::value && !std::is_arithmetic_v<T>) return false;
		else return (*(T*)gen->raw_bytes() == _object_);
	}

	shared_generic make() override { return make_shared<GenericType<T>>(); }
};

class NullGeneric : public Generic
{
	 char* raw_bytes() override { return nullptr; };
	 const type_info& type() override { return typeid(void); };
	 const type_info& identity() override { return typeid(NullGeneric); };
	 void set(shared_generic value) override { };
	 size_t size() override { return 0; };

	 string stringify() override { return "null"; }
	 int destringify(const string& str) override { return -2; }

	 shared_generic make() override { return make_shared<NullGeneric>(); }
};

template<typename T>
inline shared_ptr<GenericType<T>> make_generic() { return make_shared< GenericType<T>>(); }

template<typename T>
inline shared_ptr<GenericType<T>> make_generic(const T& t) { return make_shared< GenericType<T>>(t); }

template<typename T>
inline shared_ptr<GenericType<T>> make_generic_from_string(const string& t) 
{ 
	shared_ptr<GenericType<T>> ptr = make_generic<T>();
	ptr->destringify(t);
	return ptr; 
}

template<typename T>
inline shared_ptr<GenericRef<T>> make_generic_ref(T& ref)
{
	shared_ptr<GenericRef<T>> ptr = std::make_shared<GenericRef<T>>(ref);
	return ptr;
}

template<typename T>
inline shared_generic GenericRef<T>::dereference()
{
	if constexpr (!is_type_complete_v<T>) { return make_shared < GenericRef<T>>(*this); }
	else if constexpr (is_pointer_v<T>) return make_shared < GenericRef<remove_pointer_t<T>>>(*this);
	else 
	{
		 return make_shared<GenericType<T>>(*_object_);
	}
}

template<typename T>
inline shared_generic GenericRef<T>::make() { return make_shared<GenericRef<T>>(); }
