#pragma once
#include "ObjectGenerics.h"
#include "FunctionGenerics.h"
#include "ContainerGenerics.h"
#include "Singleton.h"
#include "ClassFactory.h"
#include <map>
#include <set>

using std::map;
using std::multimap;
using std::set;

class VariableDictionnary;

/*
* keeps track of all variables for a given scope
*/
class VariableRegistry
{
private:
	map<string, shared_generic> entries;
	multimap<string, shared_generic> overloads;

public:
	string name;
	void add(shared_generic ptr, const string& name) 
	{
		if (ptr->identity() == typeid(GenericFunction) && entries.count(name))
		{
			if (get(name)->identity() == typeid(GenericFunction))
			{
				auto fn = get(name);
				shared_ptr<GenericFunction> fn_type = std::reinterpret_pointer_cast<GenericFunction>(fn);
				shared_ptr<GenericFunction> overload_type = std::reinterpret_pointer_cast<GenericFunction>(ptr);

				if(fn_type->arg_count() == overload_type->arg_count())
					overloads.emplace(name, ptr);
				else
				{
					std::cout << "GLUU only supports overloads with the same arg count" << std::endl;
					return;
				}
			}

			std::cout << "Cannot add an overload to a non-function" << std::endl;
		}
		else
			entries.emplace(name, ptr);
	}
	shared_generic get(const string& name) { return entries.at(name); }
	bool match_fn(const string& name, const vector<GenericArgument>& args, shared_generic& ret) 
	{
		if (has_fn(name))
		{
			auto fn = get(name);
			shared_ptr<GenericFunction> fn_type = std::reinterpret_pointer_cast<GenericFunction>(fn);
			if (fn_type->args(args))
			{
				ret = fn;
				return true;
			}
		}

		if (overloads.count(name))
		{
			auto range = overloads.equal_range(name);

			for (auto it = range.first; it != range.second; it++)
			{
				auto fn = it->second;
				shared_ptr<GenericFunction> fn_type = std::reinterpret_pointer_cast<GenericFunction>(fn);
				if (fn_type->args(args))
				{
					ret = fn;
					return true;
				}
			}
		}

		return false;
	}

	bool has_fn(const string& name) { return entries.count(name) && entries.at(name)->identity() == typeid(GenericFunction); }
	bool has(const string& name) { return entries.count(name); }

	bool make(const string& str, const string& type)
	{
		if (!ClassFactory::get()->has(type)) return false;
		add(ClassFactory::get()->make(type), str);
		return true;
	}

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
		std::cout << "new scope " << name << std::endl;
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

	shared_generic get_fn(const string& name, const vector<GenericArgument>& args)
	{
		shared_generic ret;
		for (auto it = scopes.rbegin(); it != scopes.rend(); it++)
		{
			if ((*it)->match_fn(name, args, ret))
				return ret;
		}
		return nullptr;
	}

	vector<shared_ptr<VariableRegistry>>& all() { return scopes; }
	vector<shared_ptr<VariableRegistry>> all_saved() 
	{ 
		vector<shared_ptr<VariableRegistry>> ret;
		for (auto& s : saved_scopes)
			ret.push_back(s);
		return ret; 
	}
};

inline VariableDictionnary* variable_dictionnary() { return Singleton< VariableDictionnary>::get(); }
template<typename T> inline void track_variable(T& var, const string& name) { variable_dictionnary()->global()->add(std::make_shared <GenericRef<T>>(var), name); }