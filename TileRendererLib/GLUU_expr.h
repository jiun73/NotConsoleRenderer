#pragma once
#include "GLUU_types.h"
#include "CommandDictionnary.h"

#define GLUU_Make(_args, _name) pair<size_t, string> fetch_keyword() override {return { _args, _name };} shared_ptr<Widget> make(vector<string_ranges>& args, Parser& parser) override

#include <typeindex>

namespace GLUU {


	using std::type_index;

	struct Inspector
	{
		function<shared_generic(shared_generic, const string&)> inspect;
	};

	struct Expression
	{
		shared_ptr<VariableRegistry> scope;
		vector<Expression> recursive;
		shared_generic constant = nullptr;
		bool root = true;
		
		//Return handling
		bool return_call = false;
		shared_ptr<bool> ret_flag = nullptr;
		
		//Functions
		shared_ptr<GenericFunction> function = nullptr;
		Expression* func_base = nullptr;
		vector<string> args_name;
		string func_name;
		size_t arg_count = 0;
		bool func = false;
		bool user_func = false;

		//Inspection
		bool is_inspector = false;
		Inspector* inspector;

		Expression() { ret_flag = make_shared<bool>(); }
		Expression(shared_ptr<bool> return_flag) : ret_flag(return_flag) {}
		~Expression() {}

		type_index get_type()
		{
			if (root)
			{
				vector<type_index> types;
				for (auto r : recursive)
				{
					type_index type = r.get_type();

					if (type == typeid(void))
						continue;
					else
						types.push_back(type);
				}
				if (types.size() == 0) return typeid(void);
				if (types.size() == 1) return types.at(0);
				if (types.size() > 1) return typeid(string);
			}
			else if (func)
			{
				return function->return_type();
			}
			else if (user_func)
			{
				Expression& star = *(Expression*)(constant->raw_bytes());
				return star.get_type();
			}
			else return constant->type();
		}

		bool has_returned()
		{
			assert(ret_flag != nullptr); //Fatal error! There's a mistake in the parsing
			return *ret_flag;
		}

		void add_arg(const string& str, const string& type)
		{
			scope->add(ClassFactory::get()->make(type), str);

			args_name.push_back(str);
		}

		void add_arg_ref(const string& str, const string& type)
		{
			shared_ptr<GenericObject> ptr = std::reinterpret_pointer_cast<GenericObject>(ClassFactory::get()->make(type));
			scope->add(ptr->reference(), str);
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
				if (func_check && function->args_type(i) == typeid(Expression&))
				{
					auto rec = make_shared<GenericRef<Expression>>(&r);
					(*(Expression*)rec->raw_bytes()).func_base = this;
					args.push_back(rec);
				}
				else
				{
					auto eval = r.evaluate_next();
					args.push_back(eval);

					if (has_returned())
					{
						return { eval };
					}
				}
				i++;
			}
			return args;
		}

		vector<GenericArgument> generic_to_args(const vector<shared_generic>& gen)
		{
			vector<GenericArgument> args;

			for (auto& arg : gen)
			{
				args.push_back(arg);
			}

			return args;
		}

		shared_generic evaluate() 
		{
			*ret_flag = false;
			return evaluate_next();
		}

		shared_generic evaluate_next()
		{
			if (return_call)
			{
				if (recursive.size() > 0) {
					auto eval = recursive.at(0).evaluate_next();
					*ret_flag = true;
					return eval;;
				}
				else
				{
					*ret_flag = true;
					return nullptr;
				}
			}

			if (root)
			{
				vector<shared_generic> ret;

				for (auto& r : recursive)
				{
					shared_generic eval = r.evaluate_next();

					if (has_returned())
					{
						return eval;
					}

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
					auto args = get_args_from_recursive(true);

					if (has_returned())
					{
						return args.at(0);
					}

					if (function->args(generic_to_args(args)))
					{
						auto ret = function->call();

						if (has_returned())
						{
							return constant;
						}

						return ret;
					}
					else
						assert(false); //error GLUU_ERROR_COULD_NOT_SET_ARGS
					return nullptr;
				}
				else if (user_func)
				{
					Expression& star = *(Expression*)(constant->raw_bytes());

					if (star.args_name.size() > 0)
					{
						vector<shared_generic> args = get_args_from_recursive();

						if (has_returned())
						{
							return args.at(0);
						}

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
}
