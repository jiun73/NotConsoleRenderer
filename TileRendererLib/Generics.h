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
	virtual const type_info& identity() = 0;
	virtual void set(shared_generic value) = 0;
	virtual size_t size() = 0;

	virtual string stringify() = 0;
	virtual void destringify(const string& str) = 0;

	virtual shared_generic make() = 0;
};