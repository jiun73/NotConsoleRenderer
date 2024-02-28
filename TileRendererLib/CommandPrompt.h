#pragma once
//#include "Functions.h"
#include "ThreadPool.h"
#include <vector>
#include <list>
#include <map>
#include <string>
#include <functional>
#include "Stringify.h"

//#include "Typoids.h"

using std::vector;
using std::string;

/*
* This is a class handy for storing values and functions for debugging purposes,
* It also allows adding and calling commands from the command prompt, also for debugging purposes
*/

class CommandArguments //class for getting arguments in a command lambda
{
private:
	vector<string> args;
	size_t non_var_size = 0;

public:
	CommandArguments(const vector<string>& _args, const size_t& non_var) : args(_args), non_var_size(non_var) {}
	~CommandArguments() {}

	/*
	* Get variadic argument pack as a vector
	* (when all arguments are of the same type, you can specify it, otherwise use std::string)
	*/
	template<typename T>	vector<T>		var();
	template<>				vector<string>	var();

	std::string& operator[](size_t i) { return args.at(i); }
	int				operator()(size_t i) { return get<int>(i); }

	size_t size() { return args.size(); }
	size_t nonvarsize() { return non_var_size; }
	size_t varsize();

	//get the argument at i and convert it to T
	template <typename T> T get(size_t i);
};

typedef std::function<void(CommandArguments&)> CommandFunc;
#define __COMMAND_ARGS__  CommandArguments& args

enum CommandNodeTypes
{
	CMD_NODE_ARG,
	CMD_NODE_STRING,
	CMD_NODE_INT,
	CMD_NODE_VAR
};

class CommandTreeNode //node that stores functions as command branches
{
private:
	vector<CommandTreeNode*> next;
	string _arg = "";
	CommandFunc _func;
	CommandNodeTypes _type = CMD_NODE_ARG;

	CommandTreeNode* get_node_literal(const string& arg);
	CommandTreeNode* get_node(const string& arg);
	void				make(const string& arg);
	bool				has_literal(const string& arg);
	bool				has(const string& arg);
	bool				validate(const string& arg);

public:
	CommandTreeNode() {}
	CommandTreeNode(const string& arg);
	virtual ~CommandTreeNode() {}

	void			explore(vector<string>& possibilities, bool is_first = true);
	void			add(vector<string> args, const CommandFunc& func);
	void			remove(vector<string> args);
	CommandFunc* get(vector<string> args, CommandTreeNode*& last, size_t& nonvarsize);
	CommandFunc* _get(vector<string>& args, CommandTreeNode*& last, size_t& nonvarsize);
};

class CommandTree
{
private:
	CommandTreeNode root;

public:
	//split commands into it's arguments
	vector<string>	split(const string& command);
	void			add(const string& command, CommandFunc func);
	void remove(const string& command);
	CommandFunc* get(const string& command, CommandTreeNode*& last, vector<string>& args, size_t& nonvarsize);
	CommandTreeNode& get_root() { return root; }
};

struct CommandSpace 
{
	CommandSpace(const string& name) : name(name) {}
	~CommandSpace() {}

	string name;
	vector<std::pair<string, CommandFunc>> temp;
	std::function<void()> callback;
};

#include <map>

using std::map;

class CommandPrompt
{
private:
	CommandTree tree;
	
	std::mutex mutex;
	bool poll = false;
	std::list<string> input;

	//std::map<string, Typoid*> dictionnary;

	


public:
	vector< std::function<void()>> call_once;

	map<string, CommandSpace> temp_spaces;
	ThreadPool pool;

	void enter_space(const CommandSpace& space) 
	{
		temp_spaces.emplace(space.name, space);

		for (auto& x : space.temp)
		{
			add(space.name + ": " + x.first, x.second);
		}
	}

	void exit_space(const CommandSpace& space)
	{
		CommandSpace& _space = temp_spaces.at(space.name);
		for (auto& x : _space.temp)
		{
			remove(_space.name + ": " + x.first);
		}
		temp_spaces.erase(_space.name);
	}

	CommandPrompt();
	~CommandPrompt() { if(poll)stopPolling(); }

	template<typename T>
	void addPointer(T* obj, string name);

	//stops the input thread
	void stopPolling();

	//starts a new thread that waits for user input
	void startPolling();

	void update(bool print = false);
	void add(const string& command, CommandFunc func);
	void remove(const string& command)
	{
		tree.remove(command);
	}

	bool isPolling() { return poll; }

};

class Commands //Command singleton
{
private:
	static CommandPrompt* cmd;
public:
	static CommandPrompt* get()
	{
		if (cmd == nullptr)
			cmd = new CommandPrompt();

		return cmd;
	}
};

template<size_t I = 0>
struct NCR_DefineCommand //Use this to define new commands
{
	NCR_DefineCommand(string cmd, CommandFunc func) { Commands::get()->add(cmd, func); }
	~NCR_DefineCommand() {}
};

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define __NEW_COMMAND__(a, func) NCR_DefineCommand TOKENPASTE2(cmd_, __COUNTER__)(a, func)

//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------

template<typename T>
inline void CommandPrompt::addPointer(T* obj, string name)
{
	//dictionnary.emplace(name, new ObjectTypoid<T*>(obj));
}

template<typename T>
inline vector<T> CommandArguments::var()
{
	vector<T> va_args;
	for (size_t i = non_var_size - 1; i < args.size(); i++)
	{
		va_args.push_back(get<T>(i));
	}
	return va_args;
}

template<>
inline vector<string> CommandArguments::var()
{
	vector<string> va_args;
	for (size_t i = non_var_size - 1; i < args.size(); i++)
	{
		va_args.push_back(args.at(i));
	}
	return va_args;
}

template<typename T>
inline T CommandArguments::get(size_t i)
{
	T obj;
	strings::destringify(obj, args.at(i));
	return obj;
}
