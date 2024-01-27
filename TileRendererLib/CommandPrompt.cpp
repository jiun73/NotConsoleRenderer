#include "CommandPrompt.h"
#include "StringManip.h"
#include "pch.h"

using std::vector;
using std::string;

CommandPrompt* Commands::cmd;

CommandPrompt::CommandPrompt()
{
	//startPolling();
}

//stops the input thread

void CommandPrompt::stopPolling()
{
	{
		std::unique_lock<std::mutex> lock(mutex);
		poll = false;
	}
	pool.join();
	pool.stop();
}

//starts a new thread that waits for user input

void CommandPrompt::startPolling()
{
	poll = true;
	pool.start(2);
	pool.queueJob([&](int)
		{
			while (poll)
			{
				if (callback != nullptr)
				{
					callback();
				}

				string in;
				std::getline(std::cin, in);
				system("cls");

				{
					std::unique_lock<std::mutex> lock(mutex);
					input.push_back(in);
				}


			}
		});
}

void CommandPrompt::update()
{
	while (!input.empty())
	{
		std::string in;
		{
			std::unique_lock<std::mutex> lock(mutex);
			in = input.front();
			input.pop_front();
		}

		CommandTreeNode* last = nullptr;

		vector<string> args;
		size_t nvsz = 0;
		CommandFunc* func = tree.get(in, last, args, nvsz);
		CommandArguments args_wrapper(args, nvsz);

		if (func != nullptr)
		{
			(*func)(args_wrapper);
		}
		else
		{
			if (last != nullptr)
			{
				std::cout << "Other possible branches: " << std::endl;

				vector<string> possible_continuations;
				last->explore(possible_continuations);
				for (auto& node : possible_continuations)
				{
					std::cout << node << std::endl;
				}
			}
		}
	}
}

void CommandPrompt::add(const string& command, CommandFunc func)
{
	tree.add(command, func);
}

vector<string> CommandTree::split(const string& command)
{
	vector<string> args;
	split2(args, command);
	return args;
}

void CommandTree::add(const string& command, CommandFunc func)
{
	root.add(split(command), func);
}

CommandFunc* CommandTree::get(const string& command, CommandTreeNode*& last, vector<string>& args, size_t& nonvarsize)
{
	args = split(command);
	return root.get(args, last, nonvarsize);
}

CommandTreeNode* CommandTreeNode::get_node_literal(const string& arg) //get next node with this arg
{
	for (auto node : next)
		if (node->_arg == arg)
			return node;
	std::cout << "returned nullptr!" << std::endl;
	return nullptr;
}

CommandTreeNode* CommandTreeNode::get_node(const string& arg) //get next node with a matching arg
{
	for (auto node : next)
		if (node->validate(arg))
			return node;
	std::cout << "returned nullptr!" << std::endl;
	return nullptr;
}

void CommandTreeNode::make(const string& arg) //makes a new next node
{
	CommandTreeNode* node = new CommandTreeNode(arg);
	next.push_back(node);
	//std::cout << "added '" << node->_arg << "' to '" << _arg << "'" << std::endl;
}

bool CommandTreeNode::has_literal(const string& arg) //if next nodes have this argument
{
	for (auto node : next)
		if (node->_arg == arg)
			return true;
	return false;
}

bool CommandTreeNode::has(const string& arg) //if next nodes have this argument
{
	for (auto node : next)
		if (node->validate(arg))
			return true;
	return false;
}

bool CommandTreeNode::validate(const string& arg)
{
	switch (_type)
	{
	case CMD_NODE_ARG:
		return (arg == _arg);
		break;
	case CMD_NODE_STRING:
	case CMD_NODE_VAR:
		return true;
		break;
	case CMD_NODE_INT:
		return isNum(arg);
		break;
	default:
		return false;
		break;
	}
}

CommandTreeNode::CommandTreeNode(const string& arg) : _arg(arg)
{
	if (_arg.compare("***") == 0)
	{
		_type = CMD_NODE_STRING;
	}
	else if (_arg.compare("[...]") == 0)
	{
		_type = CMD_NODE_VAR;
	}
	else if (_arg.compare("(int)") == 0)
	{
		_type = CMD_NODE_INT;
	}
}

void CommandTreeNode::explore(vector<string>& possibilities, bool is_first)
{
	if (next.empty()) //this is the end of the branch, return arg
	{
		possibilities = { _arg };
		return;
	}

	for (auto node : next) //get all possibilities in next nodes, and add _arg to it
	{
		vector<string> next_possibilities;
		node->explore(next_possibilities, false);

		for (auto& branch : next_possibilities)
			if (!is_first)
				possibilities.push_back(_arg + " " + branch);
			else
				possibilities.push_back(branch);
	}
}

void CommandTreeNode::add(vector<string> args, const CommandFunc& func) //add argument chain to nodes
{
	if (args.empty())
	{
		_func = func;
		return;
	}

	string arg = args.at(0);

	if (!has_literal(arg))
		make(arg);

	args.erase(args.begin()); //we added the first arg, remove it
	get_node_literal(arg)->add(args, func); //add the rest to the next node (note: if it is empty, it will return)
}

CommandFunc* CommandTreeNode::get(vector<string> args, CommandTreeNode*& last, size_t& nonvarsize)
{
	return _get(args, last, nonvarsize); //makes a copy of the arguments
}

CommandFunc* CommandTreeNode::_get(vector<string>& args, CommandTreeNode*& last, size_t& nonvarsize)
{
	//std::cout << "'" << _arg << "'";

	if (next.empty()) //we hit the end of a branch -> return the func
	{
		//std::cout << " end of branch" << std::endl;
		last = nullptr;
		return &_func;
	}

	if (args.empty() || _type == CMD_NODE_VAR) //we've hit the end of the args (or this is variable) but there is more branches ahead -> return all the possible branches
	{
		//std::cout << " further branches" << std::endl;

		last = this;
		if (_func)
			return &_func; //we return a func if there is one here
		else
			return nullptr;
	}

	string arg = args.at(0);
	//std::cout << " -> " << arg << std::endl;

	if (!has(arg)) //we don't have the current argument = there is no matching branch -> return empty list
	{
		//std::cout << " no branch" << std::endl;
		last = nullptr;
		return nullptr;
	}

	//std::cout << " exploring further" << std::endl;

	args.erase(args.begin()); //we check the first arg, remove it
	nonvarsize++;
	return get_node(arg)->_get(args, last, nonvarsize);
}

size_t CommandArguments::varsize()
{
	if (non_var_size >= args.size())
		return 0;
	return args.size() - non_var_size;
}
