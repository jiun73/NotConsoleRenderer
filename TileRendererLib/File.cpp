#include "File.h"
#include "pch.h"
#include <filesystem>

#include <WinSock2.h>
#define no_init_all deprecated
#include <Windows.h>

vector<string> get_all_files_names_within_folder(string folder)
{
	vector<std::wstring> wide_names;
	vector<std::string> names;
	std::wstring folder_wide(folder.begin(), folder.end());
	std::wstring search_path = folder_wide + L"/*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				wide_names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	for (auto n : wide_names)
	{
		names.push_back(string(n.begin(), n.end()));
	}
	return names;
}

inline void File::writeString(const std::string& s)
{
	size_t sz = s.length(); //write the size of the string
	writeToFile(sz);

	for (size_t i = 0; i < sz; i++) //then write the characters
	{
		char c = s.at(i);
		writeToFile(c);
	}
}

inline std::string File::readString()
{
	size_t sz;
	readFile(sz);
	std::string a;
	for (size_t i = 0; i < sz; i++)
		a.push_back(readFileChar());
	return a;
}

inline void File::writeToFile(raw_data* data, size_t size)
{
	file.write(data, size); //reinterpret the data as char
	moveCursor(size);

	if (file.fail())
	{
		std::cerr << "Error while writing to file" << std::endl;
		assert(0);
	}
}

inline char File::readFileChar()
{
	char data;

	setCursor();
	file.read(&data, sizeof(char));
	moveCursor<char>();

	if (file.fail())
	{
		std::cerr << "Error while reading to file" << std::endl;
		assert(0);
	}

	return data;
}

inline void File::writeBufferToFile()
{
	cursor = 0;
	std::map<std::string, size_t> headerTemp;
	size_t currentIndex = 0;
	size_t headerSize = 0;
	headerSize += sizeof(size_t);
	for (auto& data : writingBuffer)
	{
		headerTemp.emplace(data.first, currentIndex);
		currentIndex += data.second.second;
		headerSize += sizeof(size_t);
		headerSize += sizeof(size_t);
		headerSize += sizeof(char) * data.first.size();
	}

	writeToFile(headerSize);

	for (auto& data : headerTemp)
	{
		writeString(data.first);
		writeToFile(data.second + headerSize);
	}

	for (auto& data : writingBuffer)
	{
		writeToFile(std::get<0>(data.second), std::get<1>(data.second));
		delete[] std::get<0>(data.second);
	}
}

void File::setCursor()
{
	file.seekg(cursor, std::ios::beg);
}

inline void File::getHeaderData()
{
	size_t headerSize;
	readFile(headerSize);

	while (cursor < headerSize)
	{
		std::string name = readString();
		size_t index;
		readFile(index);
		std::cerr << name << ": " << index << std::endl;
		header.emplace(name, index);
	}
}

void File::open(const std::string& path, FileXMode mode)
{
	if (currentMode != FILE_NONE)
	{
		std::cerr << "Error! file was already opened!" << std::endl;
		return;
	}

	currentMode = mode;

	switch (currentMode)
	{
	case FILE_NONE:
		close();
		break;
	case FILE_READING:
		file.open(path, std::ios::in | std::ios::binary);
		getHeaderData();
		break;
	case FILE_WRITING:
		file.open(path, std::ios::out | std::ios::binary);
		break;
	case FILE_READING_STRING:
		file.open(path, std::ios::in);
		break;
	case FILE_WRITING_STRING:
		file.open(path, std::ios::out);
		break;
	default:
		break;
	}

	if (file.fail())
	{
		// Get the error code
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
		std::filesystem::path p = std::filesystem::current_path();

		std::cout << "The current path " << p << " decomposes into:\n"
			<< "root name " << p.root_name() << '\n'
			<< "root directory " << p.root_directory() << '\n'
			<< "relative path " << p.relative_path() << '\n';
		std::cerr << "Error while opening file (Wrong path? : " << path << ") " << cursor << " SDL says:" << SDL_GetError() << std::endl;
		assert(0);
	}
}

void File::close()
{
	switch (currentMode)
	{
	case FILE_NONE:
		break;
	case FILE_READING:

		break;
	case FILE_WRITING:
		writeBufferToFile();
		break;
	default:
		break;
	}

	currentMode = FILE_NONE;
	cursor = 0;

	if (file.is_open())
		file.close();
}
