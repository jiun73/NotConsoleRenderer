#pragma once
#include "ObjectGenerics.h"
#include "FunctionGenerics.h"
#include "ContainerGenerics.h"
#include "Singleton.h"
#include <map>
#include <set>

using std::map;
using std::set;

class VariableDictionnary;

/*
* keeps track of all variables for a given scope
*/
class VariableRegistry
{
private:
	map<string, shared_generic> entries;

public:
	string name;
	void add(shared_generic ptr, const string& name) { entries.emplace(name, ptr); }
	shared_generic get(const string& name) { return entries.at(name); }
	bool has(const string& name) { return entries.count(name); }

	map<string, shared_generic>& all() { return entries; }

	friend VariableDictionnary;
};

/*
* keeps track of all varaibles regardless of scope
*/
class VariableDictionnary 
{
private:
	//List of 'saved' scopes. They are used to make non-active scopes readable
	set<shared_ptr<VariableRegistry>> saved_scopes;
	vector<shared_ptr<VariableRegistry>> scopes;

public:
	VariableDictionnary() 
	{
		//this is the global registry, always at index = 0
		enter_scope(make_new_scope("Global"));
	}
	~VariableDictionnary() {}

	shared_ptr<VariableRegistry> global() { return scopes.at(0); }

	shared_ptr<VariableRegistry> make_temporary_scope(const string& name = "")
	{
		shared_ptr<VariableRegistry> pointer = std::make_shared< VariableRegistry>();
		pointer->name = name;
		return pointer;
	}

	shared_ptr<VariableRegistry> make_new_scope(const string& name = "")
	{
		shared_ptr<VariableRegistry> pointer = make_temporary_scope(name);
		saved_scopes.emplace(pointer);
		return pointer;
	}

	void forget_scope(shared_ptr<VariableRegistry> registry) { saved_scopes.erase(registry); }
	void enter_scope(shared_ptr<VariableRegistry> registry){scopes.push_back({ registry });}
	void exit_scope() { if (scopes.size() > 1)scopes.pop_back(); }
	void exit_all() { scopes.resize(1); }

	shared_generic get(const string& name)
	{
		for (auto it = scopes.rbegin(); it != scopes.rend(); it++)
		{
			if ((*it)->has(name))
				return (*it)->get(name);
		}
		return nullptr;
	}

	vector<shared_ptr<VariableRegistry>>& all() { return scopes; }
};

inline VariableDictionnary* variable_dictionnary() { return Singleton< VariableDictionnary>::get(); }
template<typename T> inline void track_variable(T& var, const string& name) { variable_dictionnary()->global()->add(std::make_shared <GenericRef<T>>(var), name); }