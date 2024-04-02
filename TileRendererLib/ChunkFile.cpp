#include "pch.h"
#include "ChunkFile.h"
#include <cstring>
#include <limits>

namespace NCR {

	Files::Chunk& Files::Chunk::operator[](const string& name)
	{
		if (file->mode == FILE_READING)
		{
			Chunk& chunk = branches.at(name);
			file->file.seekg(chunk.offset);
			return chunk;
		}
		else
		{
			mode = FILE_CHUNK_BRANCH;
			if (!branches.count(name))
				branches.emplace(name, Chunk(file));

			return branches.at(name);
		}
	}

	void Files::Chunk::get_total_size()
	{
		switch (mode)
		{
		case FILE_CHUNK_DATA:

			break;
		case FILE_CHUNK_BRANCH:
			for (auto& c : branches)
			{
				c.second.get_total_size();
				size += c.second.size;
			}
			break;
		default:
			break;
		}
	}

	void Files::Chunk::load()
	{
		switch (file->read_byte())
		{
		case '\0': //data
			mode = FILE_CHUNK_DATA;
			offset = file->file.tellg();
			file->skip(size);
			break;
		case '\1': //header
			mode = FILE_CHUNK_BRANCH;
			while (true)
			{
				string chunk_name = file->read_string();
				if (chunk_name.empty()) break;
				branches.emplace(chunk_name, Chunk(file));
				size_t size = file->read_encoded_size();
				branches.at(chunk_name).size = size;
			}
			for (auto& c : branches)
			{
				c.second.load();
			}
			break;
		}
	}

	void Files::Chunk::write()
	{
		if (file->mode != FILE_WRITING) return;

		get_total_size();

		switch (mode)
		{
		case FILE_CHUNK_DATA:
			file->write_data("\0", 1);
			for (auto& d : data)
			{
				file->write_data(d.first, d.second);
			}
			break;
		case FILE_CHUNK_BRANCH:
			file->write_data("\1", 1);
			for (auto& c : branches)
			{
				const char* str = c.first.c_str();
				file->write_data(str, strlen(str) + 1);
				file->write_encoded_size(c.second.size);
			}
			file->write_data("\0", 1);
			for (auto& c : branches)
			{
				c.second.write();
			}
			break;
		default:
			break;
		}
	}

	void Files::Chunk::clean()
	{
		for (auto& d : data) delete[] d.first;
		for (auto& c : branches) c.second.clean();
		branches.clear();
		data.clear();
	}
}
