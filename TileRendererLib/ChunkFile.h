#pragma once
#include <typeindex>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <functional>
#include <iostream>
#include <filesystem>
#include <cassert>

using std::unordered_map;
using std::map;
using std::pair;
using std::type_index;
using std::function;
using std::string;
using std::vector;
using std::fstream;
using std::cout;

namespace NCR {
	class File;

	namespace Files {
		enum ChunkMode
		{
			FILE_CHUNK_DATA,
			FILE_CHUNK_BRANCH,
		};

		enum FileMode
		{
			FILE_READING,
			FILE_WRITING
		};

		struct ChunkNext {};
		inline ChunkNext next;

		struct raw 
		{
			char* data;
			size_t size = 0;
			raw(char* data, size_t size) : data(data), size(size) {}
		};

		struct Chunk
		{
			size_t offset = 0;
			size_t size = 0;
			File* file;
			ChunkMode mode = FILE_CHUNK_DATA;
			map<string, vector <Chunk>> branches;
			size_t index = 0;
			size_t last_size = 0;
			bool compress_size = true;
			vector<pair<char*, size_t>> data;

			Chunk() : file(nullptr) {}
			Chunk(File* file) : file(file) {}
			~Chunk() {}

			bool has_chunk(const string& name) { return branches.count(name) != 0; }

			size_t get_index_size(size_t i);
			void get_total_size();

			Chunk& operator[](size_t i);

			Chunk& operator()(const string& name);

			template<typename T>
			Chunk& operator<<(const T& obj);

			Chunk& operator<<(const ChunkNext& next);
			Chunk& operator<<(const raw& next);

			template<typename T>
			Chunk& operator>>(T& obj);

			template<typename T>
			Chunk& operator+(T& obj);

			void load();
			void write();
			void clean();

			Chunk& next_index();

			size_t index_size() 
			{
				return branches.begin()->second.size();
			}

			vector<string> get_chunks() 
			{
				vector<string> ret;
				for (auto& s : branches)
					ret.push_back(s.first);
				return ret;
			}

			Chunk& value_as(const string& name_reading, string& write_from);

			template<typename C>
			C& iterate(C& container);

			template<typename C>
			Chunk& make_list(C& container);
			

			template<typename T>
			T* list(size_t& size);
		};
	}

	class File
	{
		friend Files::Chunk;

	private:
		Files::FileMode mode;
		fstream file;
		bool is_open = false;

	public:
		Files::Chunk chunk;

		File(const string& path, Files::FileMode mode) : mode(mode), chunk(this) { open(path, mode); }
		~File() { close(); }

		Files::Chunk& operator()(const string& name)
		{
			return chunk(name);
		}

		bool is_reading() { return mode == FILE_READING; }
		bool is_writing() { return mode == FILE_WRITING; }

		Files::FileMode current_mode() { return mode; }

		bool open(const string path, Files::FileMode new_mode)
		{
			if (is_open) close();

			mode = new_mode;

			switch (mode)
			{
			case Files::FILE_READING:
				file.open(path, std::ios::in | std::ios::binary);
				break;
			case Files::FILE_WRITING:
				file.open(path, std::ios::out | std::ios::binary);
				break;
			}

			if (file.fail())
			{
				// Print system error message
				std::perror("Error: ");
				std::filesystem::path p = std::filesystem::current_path();

				std::cout << "The current path " << p << " decomposes into:\n"
					<< "root name " << p.root_name() << '\n'
					<< "root directory " << p.root_directory() << '\n'
					<< "relative path " << p.relative_path() << '\n';
				return false;
			}

			if (mode == Files::FILE_READING)
			{
				chunk.load();
			}

			is_open = true;
			return true;
		}

		void close()
		{
			if (!is_open) return;

			switch (mode)
			{
			case Files::FILE_READING:

				break;
			case Files::FILE_WRITING:
				chunk.write();
				break;
			}

			file.close();
			chunk.clean();
			is_open = false;
		}

		char read_byte()
		{
			char c;
			file.read(&c, 1);
			return c;
		}

		string read_string()
		{
			string ret = "";
			while (true)
			{
				char c = read_byte();
				if (c == '\0') return ret;
				ret.push_back(c);
			}
		}

		template<typename T>
		void write(const T& data)
		{
			file.write(reinterpret_cast<const char*>(&data), sizeof(T));
		}

		template<typename T>
		T read()
		{
			T obj;
			file.read((char*)&obj, sizeof(T));
			return obj;
		}

		void write_data(const char* data, size_t size)
		{
			file.write(data, size);
		}

		void skip(size_t i)
		{
			file.seekg(i, std::ios_base::cur);
		}

		size_t read_encoded_size()
		{
			char c = read_byte();

			return read_encoded_size(c);
		}

		size_t read_encoded_size(char c)
		{
			switch (c)
			{
			case '1': return read<uint8_t>();
			case '2': return read<uint16_t>();
			case '4': return read<uint32_t>();
			case '8': return read<uint64_t>();
			default: return 0;
			}
		}

		void write_encoded_size(size_t size)
		{
			if (size <= UINT8_MAX)
			{
				write_data("1", 1);
				write((uint8_t)(size));
			}
			else if (size <= UINT16_MAX)
			{
				write_data("2", 1);
				write((uint16_t)(size));
			}
			else if (size <= UINT32_MAX)
			{
				write_data("4", 1);
				write((uint32_t)(size));
			}
			else
			{
				write_data("8", 1);
				write((uint64_t)(size));
			}
		}


	};

	template<typename T>
	Files::Chunk& Files::Chunk::operator<<(const T& obj)
	{
		if (file->mode != FILE_WRITING) return *this;

		mode = FILE_CHUNK_DATA;
		size += sizeof(obj);
		char* bytes = new char[sizeof(obj)]; //copy the raw data of the object to the buffer
		memcpy(bytes, &obj, sizeof(obj));
		data.push_back(std::make_pair(bytes, sizeof(obj)));
		return *this;
	}

	template<typename T>
	Files::Chunk& Files::Chunk::operator>>(T& obj)
	{
		size_t pos = file->file.tellg();

		if (pos - offset >= size)
		{
			assert(false);
			return *this;
		}

		obj = file->read<T>();
		return *this;
	}

	template<typename T>
    Files::Chunk& Files::Chunk::operator+(T& obj)
	{
		if (file->mode == FILE_WRITING) return operator<<(obj);
		else if (file->mode == FILE_READING) return operator>>(obj);
		return *this;
	}

	template<typename C>
	 C& Files::Chunk::iterate(C& container)
	{
		if (file->mode == FILE_READING)
		{
			container.clear();
			container.resize(index_size());
			return container;
		}
		else if (file->mode == FILE_WRITING)
		{
			return container;
		}
		return container;
	}

	 template<typename C>
	 Files::Chunk& Files::Chunk::make_list(C& container)
	 {
		 if (file->mode == FILE_READING)
		 {
			 size_t size = 0;
			 using type = typename C::value_type;
			 type* l = list<type>(size);
			 for (size_t i = 0; i < size; i++)
			 {
				 container.insert(container.end(), l[i]);
			 }
		 }
		 else if (file->mode == FILE_WRITING)
		 {
			 for (auto& a : container) operator<<(a);
		 }
		 return *this;
	 }

	template<typename T>
	inline T* Files::Chunk::list(size_t& list_size)
	{
		if (size % sizeof(T) != 0)
		{
			assert(false);
			return nullptr;
		}

		list_size = size / sizeof(T);
		T* list = new T[list_size];
		file->file.read((char*)list, size);
		return list;
	}
}