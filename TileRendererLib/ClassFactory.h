#pragma once
#include <map>
#include <string>
#include "Singleton.h"
#include "Generics.h"
#include "Stringify.h"

using std::map;
using std::string;

/*
* Class factory, c'est une facon de creer d'autres objects d'un type qu'on a enregistrer
* sans que le type doit être definit statiquement (et donc qui ne peut pas changer apres la compilation)
* donc, si on a le nom du type, on peut creer autant d'objects qu'on veut
*/

class FactoryManager
{
private:
	map<string, shared_generic> factories;

public:
	FactoryManager() {}
	~FactoryManager() {}

	template<typename T>
	void add(const string& name)
	{
		if (!has(name))
		{
			factories.emplace(name, new GenericType<T>());
		}
	}
	shared_generic factory(const string& name) { return factories.at(name); }
	shared_generic	make(const string& name) { if (has(name)) return factory(name)->make(); return nullptr; }
	bool has(const string& name) { return factories.count(name); }
};

typedef Singleton<FactoryManager> ClassFactory;

template<typename T>
struct FactoryManagerAdder
{
	FactoryManagerAdder(const string& name) { ClassFactory::get()->add<T>(name); }
	~FactoryManagerAdder() {}
};

#define __REGISTER_CLASS__(type) const FactoryManagerAdder<type>* type##_adder = new FactoryManagerAdder<type>(#type);
#define __REGISTER_CLASS_NAME__(type,name) const FactoryManagerAdder<type>* name##_adder = new FactoryManagerAdder<type>(#type);