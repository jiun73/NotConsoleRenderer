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
	CommandFunc* get(const string& command, CommandTreeNode*& last, vector<string>& args, size_t& nonvarsize);
};

class CommandPrompt
{
private:
	CommandTree tree;
	
	std::mutex mutex;
	bool poll = false;
	std::list<string> input;

	//std::map<string, Typoid*> dictionnary;

public:
	std::function<void()> callback = nullptr;
	ThreadPool pool;


	CommandPrompt();
	~CommandPrompt() { if(poll)stopPolling(); }

	template<typename T>
	void addPointer(T* obj, string name);

	//stops the input thread
	void stopPolling();

	//starts a new thread that waits for user input
	void startPolling();

	void update();
	void add(const string& command, CommandFunc func);

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

struct DefineCommand //Use this to define new commands
{
	DefineCommand(string cmd, CommandFunc func) { Commands::get()->add(cmd, func); }
	~DefineCommand() {}
};

#define __NEW_COMMAND__(n, a, func) DefineCommand _##n##_cmd(a, func);



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
