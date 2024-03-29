#pragma once
#include <map>
#include <string>
#include "Singleton.h"
#include "Generics.h"
#include "ContainerGenerics.h"
#include "Stringify.h"

using std::map;
using std::string;

/*
* Class factory, c'est une facon de creer d'autres objects d'un type qu'on a enregistrer
* sans que le type doit �tre definit statiquement (et donc qui ne peut pas changer apres la compilation)
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
			std::cout << "Added type " << name << " to factory" << std::endl;
			if constexpr (is_container<T>::value)
			{
				std::cout << " .... as container" << std::endl;
				factories.emplace(name, new GenericContainerType<T>());
			}
			else
			{
				
				factories.emplace(name, new GenericType<T>());
			}
		}
	}
	shared_generic factory(const string& name) { return factories.at(name); }
	shared_generic	make(const string& name) { if (has(name)) return factory(name)->make(); return nullptr; }
	bool has(const string& name) { return factories.count(name); }
};

typedef Singleton<FactoryManager> ClassFactory;

#include "StringRanges.h"

template<typename T>
struct FactoryManagerAdder
{
	FactoryManagerAdder(const string& name, bool replace_char = false) 
	{
		if (replace_char)
		{
			string copy = name;
			copy = replace(replace(copy, '<', '('), '>', ')').flat();
			ClassFactory::get()->add<T>(copy);
		}
		else
		{
			ClassFactory::get()->add<T>(name);
		}
	}
	~FactoryManagerAdder() {}
};

#define __REGISTER_CLASS__(type) const FactoryManagerAdder<type>* type##_adder = new FactoryManagerAdder<type>(#type, true);
#define __REGISTER_CLASS_NAME__(type,name) const FactoryManagerAdder<type>* name##_adder = new FactoryManagerAdder<type>(#type, true);