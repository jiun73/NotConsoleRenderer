#include "NotConsoleRenderer.h"

#include <set>
using std::set;

struct Node 
{
	vector<vector<int>> instructions;
};

struct NodeInfo 
{
	set<int> used_inputs;
	set<int> used_outputs;

	bool jro_check = false;
	bool jez_check = false;
	bool jnz_check = false;
	bool jgz_check = false;
	bool jlz_check = false;
};

int errors = 0;

NodeInfo get_node_info(const Node& node) 
{
	NodeInfo r;

	for (auto& i : node.instructions)
	{
		switch (i.at(0))
		{
		case 2:
			r.jro_check = true;
			break;
		case 3:
			r.jez_check = true;
			break;
		case 4:
			r.jnz_check = true;
			break;
		case 5:
			r.jgz_check = true;
			break;
		case 6:
			r.jlz_check = true;
			break;
		case 1:
		case 7:
		case 8:
		{
			if (i.at(1) < 0)
			{
				r.used_inputs.emplace(i.at(1));
			}

			if (i.at(2) < 0)
			{
				r.used_outputs.emplace(i.at(2));
			}
		}
		default:
			break;
		}
	}

	return r;
}

vector<int> translate_val(const string& str)
{
	if (str == "UP") return { -1 };
	if (str == "DOWN") return { -2 };
	if (str == "LEFT") return { -3 };
	if (str == "RIGHT") return { -4 };
	if (str == "ACC") return { -5 };
	if (str == "LAST") return { -6 };
	if (str == "ANY") return { -7 };
	
	if (isNum(str))
	{
		int ret;
		destringify(str, ret);
		int sign;
		if (ret >= 0)
			sign = 1;
		else
			sign = -1;
		return {abs(ret), sign};
	}

	return {};
}

int translate_instr(const string& str)
{
	if (str == "NOP") return 0;
	if (str == "MOV") return 1;
	if (str == "JRO") return 2;
	if (str == "JEZ") return 3;
	if (str == "JNZ") return 4;
	if (str == "JGZ") return 5;
	if (str == "JLZ") return 6;
	if (str == "ADD") return 7;
	if (str == "SUB") return 8;
	if (str == "NEG") return 9;
	if (str == "SWP") return 10;
	if (str == "SAV") return 11;

	if (str == "JMP") return -2;

	return -1;
}

void get_labels(const string& str, map<string, size_t>& labels, size_t current)
{
	vector<string> label_split = split(str, ':');
	if (label_split.size() > 1)
	{
		string label = label_split.at(0);
		trim(label, ' ');
		labels.emplace(label, current);
	}
}

vector<int> translate_line(string str, const map<string, size_t>& labels, size_t current)
{
	trim(str, ' ');
	if (isNum(str)) return {};
	if (str.empty()) return {};

	if (str.front() == '#')
	{
		std::cout << "Found comment: " << str << std::endl;
		return {};
	}

	if (str.front() == '!')
	{
		str = str.substr(1);
	}

	vector<int> n;

	string instr;
	vector<string> label_split = split(str, ':');

	instr = label_split.back();
	trim(instr, ' ');

	if (instr.empty()) return {};

	vector<string> args = split(instr, ' ');

	int instr_code = translate_instr(args.at(0));
	if (instr_code == -1) { std::cout << "Could not tranlate " << str << std::endl; string buff;
	std::getline(std::cin, buff); errors++; return {}; }
	if (instr_code == -2)
	{
		int val =  (int)labels.at(args.at(1)) - current;

		int sign;
		if (val >= 0)
			sign = 1;
		else
			sign = -1;

		return { 2, abs(val), sign };
	}

	switch (instr_code)
	{
	case 3:
	case 4:
	case 5:
	case 6:
	{
		int val = (int)labels.at(args.at(1)) - current;

		int sign;
		if (val >= 0)
			sign = 1;
		else
			sign = -1;

		return { instr_code, abs(val), sign };
	}

	default:
		break;
	}

	n.push_back(instr_code);
	
	if (args.size() > 1)
		for (auto it = args.begin() + 1; it != args.end(); it++)
			for (auto& val : translate_val(*it))
				n.push_back(val);

	return n;
}

template<size_t I>
struct rec 
{

};


template<typename T>
using t_ = tuple<T>;

#define TEST(t) using t_##__COUNTER__

int main() 
{
	//vector<string> files = get_all_files_names_within_folder("TIS-100");
	//std::cout << "Found " << files.size() << " available save files" << std::endl;

	//vector<Node> collected_nodes;
	//
	//std::stringstream ss;
	//bool ffront = true;
	//for(auto& file : files)
	//{
	//	Random random;
	//	std::cout << "TIS-100/" << file << std::endl;
	//	File ffile("TIS-100/" + file, FILE_READING_STRING);
	//	string source = ffile.getString();
	//	vector<string> nodes = split(source, '@');
	//	std::cout << "Found " << nodes.size() - 1 << " nodes in file" << std::endl;

	//	size_t f = 0;
	//	size_t in = 0;
	//	
	//	for (auto& node : nodes)
	//	{
	//		vector<string> lines = split(node, '\n');
	//		if (lines.size() < 10) continue;

	//		map<string, size_t> labels;

	//		size_t i = 0;
	//		for (auto& line : lines)
	//		{
	//			get_labels(line, labels, i);
	//			i++;
	//		}

	//		if (!labels.empty())
	//		{
	//			std::cout << labels.size() <<" labels found" << std::endl;
	//		}

	//		Node ret;
	//		i = 0;
	//		for (auto& line : lines)
	//		{
	//			vector<int> translated = translate_line(line, labels, i);

	//			if (!translated.empty())
	//				ret.instructions.push_back(translated);
	//			in++;
	//			i++;
	//		}

	//		
	//		if (ret.instructions.size() != 0)
	//		{
	//			if (!ffront)
	//				ss << ",";
	//			else
	//				ffront = false;
	//			ss << "{";
	//			bool fffront = true;
	//			for (auto& ins : ret.instructions)
	//			{
	//				if (!fffront)
	//					ss << ",";
	//				else
	//					fffront = false;
	//				ss << "{";
	//				bool front = true;
	//				for (auto& a : ins)
	//				{
	//					if (!front)
	//						ss << ",";
	//					else
	//						front = false;
	//					ss << a;
	//				}
	//				ss << "}";
	//			}
	//			ss << "}";
	//			collected_nodes.push_back(ret);
	//			f++;
	//		}
	//	}	
	//	

	//	std::cout << in << " Instructions translated" << std::endl;
	//	std::cout << f << " Nodes translated" << std::endl;

	//	/*string buff;
	//	std::getline(std::cin, buff);*/
	//} 

	//std::cout << "Translating done! errors: " << errors << std::endl;
	//std::cout << "Generating inputs... " << std::endl;

	//std::stringstream ss2;

	//ss2 << std::endl << std::endl;
	//ss2 << "{";



	//for (auto& n : collected_nodes)
	//{
	//	NodeInfo info = get_node_info(n);


	//}

	//ss2 << "}";


	//File write("output.txt", FILE_WRITING_STRING);
	//write.setString(ss.str());
}