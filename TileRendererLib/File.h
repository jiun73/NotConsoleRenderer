#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <tuple>
#include <iostream>
#include <cassert>
#include <unordered_map>

#include "SDL.h"

enum DataMode
{
	DATA_RAW,
	DATA_ARRAY
};

enum FileXMode
{
	FILE_NONE,
	FILE_READING,
	FILE_WRITING,
	FILE_READING_STRING,
	FILE_WRITING_STRING
};

typedef char raw_data;

class File
{
private:
	size_t cursor = 0;
	std::fstream file;
	std::string path;

	std::map<std::string, std::pair<raw_data*, size_t>> writingBuffer;
	std::map<std::string, size_t> header;
	FileXMode currentMode = FILE_NONE;

	void writeString(const std::string& s);
	std::string readString();

	template <typename T>
	void writeToFile(const T& data);
	void writeToFile(raw_data* data, size_t size);

	char readFileChar();

	template <typename T>
	void readFile(T& data);

	template <typename C>
	void readContainerFile(C& container);

	void writeBufferToFile();

	void setCursor();
	void moveCursorTo(size_t pos) { cursor = pos; }
	void moveCursor(size_t pos) { cursor += pos; }
	template<typename T> void moveCursor() { moveCursor(sizeof(T)); }

	void getHeaderData();

public:
	File(const std::string& path, FileXMode mode) { open(path, mode); }
	~File() { close(); }

	void open(const std::string& path, FileXMode mode);
	void close();

	template<typename T> void write(const T& data, const std::string& name);
	template<typename C> void writeContainer(const C& data, const std::string& name);
	template<typename T> T read(const std::string& name);
	std::string getString()
	{
		if (!(currentMode == FILE_READING_STRING))
		{
			std::cout << "Warning! this function is to be called only when mode is reading string!";
			return "";
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}
	void setString(const std::string& s)
	{
		if (!(currentMode == FILE_WRITING_STRING))
		{
			std::cout << "Warning! this function is to be called only when mode is witing string!";
			return;
		}

		file << s;
	}
	template<typename T> void readContainer(T& container, const std::string& name);
};

template<typename T>
inline void File::writeToFile(const T& data)
{
	file.write(reinterpret_cast<const char*>(&data), sizeof(T)); //reinterpret the data as char
	moveCursor<T>();

	if (file.fail())
	{
		std::cerr << "Error while writing to file" << std::endl;
		assert(0);
	}
}

template<typename T>
inline void File::readFile(T& data)
{
	setCursor();
	file.read((raw_data*)&data, sizeof(T));
	moveCursor<T>();

	//Logs::get<File>() << "<File> reading " << path.c_str() << " at " << cursor << std::endl;

	if (file.fail())
	{
		std::cerr << "Error while reading to file" << std::endl;
		assert(0);
	}
}

template<typename C>
inline void File::readContainerFile(C& container)
{
	size_t size;
	readFile(size);

	for (size_t i = 0; i < size; i++)
	{
		typename C::value_type data;
		readFile<typename C::value_type>(data);
		container.emplace(container.end(), data);
	}
}

template<typename T>
inline void File::write(const T& data, const std::string& name)
{
	raw_data* bytes = new raw_data[sizeof(data)]; //copy the raw data of the object to the buffer
	memcpy(bytes, &data, sizeof(data));

	writingBuffer.emplace(name, std::make_pair(bytes, sizeof(data)));
}

template<typename C>
inline void File::writeContainer(const C& container, const std::string& name)
{
	size_t size = container.size();
	size_t dataSize = sizeof(C::value_type);
	size_t fullSize = (dataSize * size) + sizeof(size_t);

	raw_data* bytes = new raw_data[fullSize]; //copy the raw data of the object to the buffer + the size of the container
	size_t i = 0;
	memcpy(bytes, &size, sizeof(size_t));
	for (auto& data : container)
	{
		memcpy(bytes + (((i * dataSize) + (sizeof(size_t)))), &data, dataSize);
		i++;
	}

	writingBuffer.emplace(name, std::make_pair(bytes, fullSize));
}

template<typename T>
inline T File::read(const std::string& name)
{
	moveCursorTo(header.at(name)); //Go to index
	T data;
	readFile(data);
	return data;
}

template<typename T>
inline void File::readContainer(T& container, const std::string& name)
{
	moveCursorTo(header.at(name)); //Go to index
	readContainerFile<T>(container);
}

using std::string;
using std::vector;

vector<string> get_all_files_names_within_folder(string folder); 
