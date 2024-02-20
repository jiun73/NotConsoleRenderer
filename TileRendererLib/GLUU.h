#pragma once
#include "GLUU_types.h"
#include "CommandDictionnary.h"

struct GLUU
{
	shared_ptr<VariableRegistry> scope;
	vector<GLUU> recursive;

	bool root = true;
	bool func = false;
	bool user_func = false;
	shared_generic constant = nullptr;
	shared_ptr<GenericFunction> function = nullptr;

	vector<string> args_name;
	string func_name;
	size_t arg_count = 0;

	void add_arg(const string& str, const string& type)
	{
		scope->add(ClassFactory::get()->make(type), str);
		args_name.push_back(str);
	}

	void set_args(vector<shared_generic>& args)
	{
		size_t i = 0;
		for (auto& name : args_name)
		{
			scope->add(args.at(i), name);
			//*(scope->get(name)) = *args.at(i);
			//scope->get(name).swap(args.at(i));
			scope->get(name)->set(args.at(i));
			i++;
		}
	}

	vector<shared_generic> get_args_from_recursive(bool func_check = false)
	{
		vector<shared_generic> args;
		size_t i = 0;
		for (auto& r : recursive)
		{
			if (func_check && function->args_type(i) == typeid(GLUU&))
			{
				shared_generic rec = make_shared<GenericRef<GLUU>>(r);
				args.push_back(rec);
			}
			else
			{
				args.push_back(r.evaluate());
			}
			i++;
		}
		return args;
	}

	vector<GenericArgument> generic_to_args(const vector<shared_generic>& gen)
	{
		vector<GenericArgument> args;

		for (auto& arg : get_args_from_recursive(true))
		{
			args.push_back(arg);
		}

		return args;
	}

	shared_generic evaluate()
	{
		if (root)
		{
			vector<shared_generic> ret;

			for (auto& r : recursive)
			{
				shared_generic eval = r.evaluate();
				if (eval != nullptr)
					ret.push_back(eval);
			}

			if (ret.size() == 0) { return nullptr; }
			if (ret.size() == 1) { return ret.at(0); }

			string collect;

			for (auto& r : ret)
			{
				collect += r->stringify();
			}

			if (constant == nullptr)
			{
				constant = make_generic<string>();
			}

			shared_generic str_type = make_generic<string>(collect);

			constant->set(str_type);

			
			return constant;
		}
		else
		{
			if (func)
			{
				if (function->args(generic_to_args(get_args_from_recursive(true))))
					return function->call();
				else
					assert(false); //error
				return nullptr;
			}
			else if (user_func)
			{
				GLUU& star = *(GLUU*)(constant->raw_bytes());

				if (star.args_name.size() > 0)
				{
					vector<shared_generic> args = get_args_from_recursive();
					star.set_args(args);
				}

				return star.evaluate();
			}
			else
			{
				return constant;
			}
		}
	}
};