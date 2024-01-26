#pragma once
#include "Stringify.h"
#include <type_traits>
#include <memory>
#include <string>

using std::string;
using std::shared_ptr;

struct Generic;

typedef shared_ptr<Generic> shared_generic;

struct Generic
{
	Generic() {}
	virtual ~Generic() {};

	virtual char* raw_bytes() = 0;
	virtual const type_info& type() = 0;
	virtual void set(shared_generic value) = 0;

	virtual string stringify() = 0;
	virtual void destringify(const string& str) = 0;

	virtual shared_generic dereference() = 0;
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
	const type_info& type() override { return typeid(T); }

	void set(shared_generic value)
	{
		if (value->type() != type()) { std::cout << "{set: types do not match}"; return; }

		*_object_ = *(T*)(value->raw_bytes());
	}

	string stringify() override { return strings::stringify(*_object_); };
	void destringify(const string& str) override { strings::destringify(*_object_, str); }

	shared_generic dereference() override;
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
	const type_info& type() override { return typeid(T); }

	void set(shared_generic value) 
	{
		if (value->type() != type()) return;

		_object_ = *(T*)(value->raw_bytes());
	}

	string stringify() override { return strings::stringify(_object_); };
	void destringify(const string& str) override { strings::destringify(_object_, str); }

	shared_generic dereference() override 
	{
		if constexpr (std::is_pointer_v<T>) return std::make_shared < GenericRef<std::remove_pointer_t<T>>>(*_object_);
		else return std::make_shared<GenericType<T>>(*this);
	}
};

template<typename T>
inline std::shared_ptr<GenericType<T>> make_generic(const T& t) { return std::make_shared< GenericType<T>>(t); }

template<typename T>
inline shared_generic GenericRef<T>::dereference()
{
	if constexpr (std::is_pointer_v<T>) return std::make_shared < GenericRef<std::remove_pointer_t<T>>>(*this);
	else return std::make_shared<GenericType<T>>(*_object_);
}
