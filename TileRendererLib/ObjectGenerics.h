#pragma once
#include "Generics.h"

class GenericObject : public Generic
{
	virtual shared_generic dereference() = 0;
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
	const type_info& type() override { return typeid(T); }
	const type_info& identity() override { return typeid(GenericObject); }
	size_t size() override { return sizeof(T); }

	void set(shared_generic value)
	{
		if (value->type() != type()) { std::cout << "{set: types do not match}"; return; }
		if constexpr (std::is_nothrow_copy_assignable_v<T>) *_object_ = *(T*)(value->raw_bytes());
	}

	string stringify() override { return strings::stringify(*_object_); };
	int destringify(const string& str) override { return strings::destringify(*_object_, str); }

	shared_generic dereference() override;
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
		if (value->type() != type()) { std::cout << "Types do not match! " << std::endl;  return; }
		if constexpr (std::is_copy_assignable_v<T>) 
		{ 
			_object_ = *(T*)(value->raw_bytes());
		}
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

	shared_generic make() override { return make_shared<GenericType<T>>(); }
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
inline shared_generic GenericRef<T>::dereference()
{
	if constexpr (is_pointer_v<T>) return make_shared < GenericRef<remove_pointer_t<T>>>(*this);
	else return make_shared<GenericType<T>>(*_object_);
}

template<typename T>
inline shared_generic GenericRef<T>::make() { return make_shared<GenericType<T>>(); }
