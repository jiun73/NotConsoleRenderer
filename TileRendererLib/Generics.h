#pragma once
#include "Stringify.h"
#include <type_traits>
#include <memory>
#include <string>

using std::string;
using std::shared_ptr;
using std::make_shared;
using std::is_pointer_v;
using std::remove_pointer_t;

struct Generic;

typedef shared_ptr<Generic> shared_generic;

struct Generic
{
	Generic() {}
	virtual ~Generic() {};

	virtual char* raw_bytes() = 0;
	virtual const type_info& type() = 0;
	virtual void set(shared_generic value) = 0;
	virtual size_t size() = 0;

	virtual string stringify() = 0;
	virtual void destringify(const string& str) = 0;

	virtual shared_generic dereference() = 0;
	virtual shared_generic make() = 0;

	virtual bool is_function() = 0;
	virtual char* function_bytes() = 0;
};

template<typename T>
class GenericType;

template<typename T>
class GenericRef : public Generic
{
private:
	T* _object_;

public:
	GenericRef() {}
	GenericRef(T& copy) : _object_(&copy) {}
	~GenericRef() {}

	char* raw_bytes() override { return (char*)(&(*_object_)); }
	bool is_function() override { return true; }
	char* function_bytes() override { return nullptr; }
	const type_info& type() override { return typeid(T); }
	size_t size() override { return sizeof(T); }

	void set(shared_generic value)
	{
		if (value->type() != type()) { std::cout << "{set: types do not match}"; return; }
		*_object_ = *(T*)(value->raw_bytes());
	}

	string stringify() override { return strings::stringify(*_object_); };
	void destringify(const string& str) override { strings::destringify(*_object_, str); }

	shared_generic dereference() override;
	shared_generic make() override;
};

template<typename T>
class GenericType : public Generic
{
private:
	T _object_;

public:
	GenericType() {}
	GenericType(const T& copy) : _object_(copy) {}
	~GenericType() {}

	char* raw_bytes() override { return (char*)(&_object_); }
	bool is_function() override { return true; }
	char* function_bytes() override { return nullptr; }
	const type_info& type() override { return typeid(T); }
	size_t size() override { return sizeof(T); }

	void set(shared_generic value) 
	{
		if (value->type() != type()) return;
		_object_ = *(T*)(value->raw_bytes());
	}

	string stringify() override { return strings::stringify(_object_); };
	void destringify(const string& str) override { strings::destringify(_object_, str); }

	shared_generic dereference() override 
	{
		if constexpr (is_pointer_v<T>) return make_shared < GenericRef<remove_pointer_t<T>>>(*_object_);
		else return make_shared<GenericType<T>>(*this);
	}

	shared_generic make() override { return make_shared<GenericType<T>>(); }
};

template<typename T>
inline std::shared_ptr<GenericType<T>> make_generic(const T& t) { return make_shared< GenericType<T>>(t); }

template<typename T>
inline shared_generic GenericRef<T>::dereference()
{
	if constexpr (is_pointer_v<T>) return make_shared < GenericRef<remove_pointer_t<T>>>(*this);
	else return make_shared<GenericType<T>>(*_object_);
}

template<typename T>
inline shared_generic GenericRef<T>::make() { return make_shared<GenericType<T>>(); }
