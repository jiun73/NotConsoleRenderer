#pragma once
#include "ObjectGenerics.h"
#include "ContainerGenerics.h"

/*
* Behold, a generic generic
* 
* it just calls the functions inside of the pointer  it holds
* the only difference is that the set function does not change the value, but the pointer
* so you can change which object it acts on
* 
*/
class MetaGeneric : public GenericContainer
{
	shared_generic obj = nullptr;

public:

	char* raw_bytes() override { return obj->raw_bytes(); }

	const type_info& type() override { return obj->type(); }
	const type_info& identity() override { return obj->identity(); }
	bool set(shared_generic value) override { obj = value; return true; };
	size_t size() override { return obj->size(); };

	string stringify() override { return obj->stringify(); };
	int destringify(const string& str) override { return obj->destringify(str); };

	shared_generic make() override { return obj->make(); };

	shared_ptr<GenericContainer> to_container() 
	{
		if (identity() == typeid(GenericContainer))
		{
			return std::reinterpret_pointer_cast<GenericContainer>(obj);
		}
		return nullptr;
	}

	shared_ptr<GenericObject> to_object() 
	{
		if (identity() == typeid(GenericObject))
		{
			return std::reinterpret_pointer_cast<GenericObject>(obj);
		}
		return nullptr;
	}

	 shared_generic at(size_t i) override { return to_container()->at(i);};
	 void insert(shared_generic value, size_t i) override { return to_container()->insert(value, i); };
	 void erase(size_t i) override { return to_container()->erase(i); };
	 size_t container_size() override { return to_container()->container_size(); };
	 shared_generic make_value() override { return to_container()->make_value(); };

	 shared_generic dereference() override { return to_object()->dereference(); }
	 shared_generic reference() override { return to_object()->reference(); };
	 shared_generic first() override { return to_object()->first(); }
	 shared_generic second() override { return to_object()->second(); }
	 bool equals(shared_generic a) override { return to_object()->equals(a); };
};

class MetaGenericFactory : public MetaGeneric
{
public:
	shared_generic make() override 
	{ 
		auto meta = std::make_shared<MetaGeneric>();
		meta->set(std::make_shared<NullGeneric>());
		return meta;
	};
};