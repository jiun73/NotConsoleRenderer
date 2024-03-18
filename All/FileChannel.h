#pragma once
#include <typeindex>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <functional>
#include <iostream>

using std::unordered_map;
using std::map;
using std::type_index;
using std::function;
using std::string;
using std::vector;
using std::fstream;
using std::cout;

enum CFileMode 
{
	FILE_READING,
	FILE_WRITING,
	FILE_SUBREADING,
	FILE_SUBWRITING
};

class CFile 
{
private:
	unordered_map < type_index, function<void()>> readers;
	map<string, vector<char*>> channels;
	fstream file;

	CFileMode mode;

public:
	CFile(const string path, CFileMode mode) : mode(mode) {}
	~CFile() {}
	
	bool open(const string path, CFileMode mode) 
	{
		switch (mode)
		{
		case FILE_READING:
			file.open(path, std::ios::in | std::ios::binary);
			break;
		case FILE_WRITING:
			file.open(path, std::ios::out | std::ios::binary);
			break;
		default:
			return false;
		}

		if (file.fail())
		{
			std::ios_base::iostate state = file.rdstate();

			// Check for specific error bits
			if (state & std::ios_base::eofbit)
			{
				std::cout << "End of file reached." << std::endl;
			}
			if (state & std::ios_base::failbit)
			{
				std::cout << "Non-fatal I/O error occurred." << std::endl;
			}
			if (state & std::ios_base::badbit)
			{
				std::cout << "Fatal I/O error occurred." << std::endl;
			}

			// Print system error message
			std::perror("Error: ");
		}

		return false;
	}

	void close() {}

	template<typename T>
	void write(const string& channel, const T& obj)
	{

	}
};