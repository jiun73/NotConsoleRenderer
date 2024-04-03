#include "pch.h"
#include "ChunkFile.h"
#include <cstring>
#include <limits>

namespace NCR {
	Files::Chunk& Files::Chunk::operator[](size_t i)
	{
		if (file->mode != FILE_READING) return *this;
		if (mode != FILE_CHUNK_BRANCH) return *this;
		index = i;
		return *this;
	}

	Files::Chunk& Files::Chunk::operator<<(const Files::ChunkNext& next)
	{
		if (mode != FILE_CHUNK_BRANCH) return *this;

		if (index == 0)
		{
			last_size = get_index_size(index);
			index++;
			return *this;
		}

		if (last_size != get_index_size(index))
		{
			compress_size = false;
		}

		index++;		

		return *this;
	}

	Files::Chunk& Files::Chunk::operator<<(const raw& raw)
	{
		if (file->mode != FILE_WRITING) return *this;

		mode = FILE_CHUNK_DATA;
		size += raw.size;
		data.push_back(std::make_pair(raw.data, raw.size));
		return *this;
	}

	Files::Chunk& Files::Chunk::operator()(const string& name)
	{
		if (file->mode == FILE_READING)
		{
			Chunk& chunk = branches.at(name).at(index);
			file->file.seekg(chunk.offset);
			return chunk;
		}
		else
		{
			mode = FILE_CHUNK_BRANCH;

			if (!branches.count(name))
				branches.emplace(name, vector<Chunk>());
			while (branches.at(name).size() <= index) 
			{
				branches.at(name).push_back(Chunk(file));
			}
			return branches.at(name).at(index);
		}
	}

	size_t Files::Chunk::get_index_size(size_t i)
	{
		if (mode != FILE_CHUNK_BRANCH) return 0;

		size_t ret = 0;
		for (auto& sub : branches)
		{
			while (sub.second.size() <= index)
			{
				sub.second.push_back(Chunk(file));
			}
			sub.second.at(i).get_total_size();
			ret += sub.second.at(i).size;
		}

		return ret;
	}

	void Files::Chunk::get_total_size()
	{
		switch (mode)
		{
		case FILE_CHUNK_DATA:

			break;
		case FILE_CHUNK_BRANCH:
			for (auto& sub : branches)
				for (auto& c : sub.second)
				{
					c.get_total_size();
					size += c.size;
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
		case '\2': //header indexed
		{
			mode = FILE_CHUNK_BRANCH;
			size_t size = file->read_encoded_size();
			while (true)
			{
				string chunk_name = file->read_string();
				if (chunk_name.empty())
				{
					break;
				}

				size_t i = 0;
				while (true)
				{
					char c = file->read_byte();
					if (c == '\0') break;
					branches.emplace(chunk_name, vector<Chunk>());
					branches.at(chunk_name).push_back(Chunk(file));
					size_t size = file->read_encoded_size(c);
					branches.at(chunk_name).back().size = size;

					i++;
				}
			}
			for (auto& sub : branches)
				for (auto& c : sub.second)
				{
					c.load();
				}
		}
			break;
		case '\1': //header
		{
			mode = FILE_CHUNK_BRANCH;
			size_t index_size = file->read_encoded_size();
			for (auto& sub : branches)
				sub.second.resize(size);
			while (true)
			{
				string chunk_name = file->read_string();
				if (chunk_name.empty()) break;
				size_t size = file->read_encoded_size();

				branches.emplace(chunk_name, vector<Chunk>());
				for (size_t i = 0; i < index_size; i++)
				{
					branches.at(chunk_name).push_back(Chunk(file));
					branches.at(chunk_name).back().size = size;
				}
			}
			for (auto& sub : branches)
				for (auto& c : sub.second)
				{
					c.load();
				}
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
			if (index < branches.size() && branches.size()>1)
				if (last_size != get_index_size(index))
				{
					compress_size = false;
				}

			

			if (compress_size) //size is the same for every index, so we can avoid writing sizes for every index
			{
				file->write_data("\1", 1);
				file->write_encoded_size(branches.begin()->second.size()); //write number of indexed chunks
				for (auto& c : branches) //write header for the first indexed chunk
				{
					const char* str = c.first.c_str();
					file->write_data(str, strlen(str) + 1);
					file->write_encoded_size(c.second.back().size);
				}
			}
			else
			{
				file->write_data("\2", 1);
				file->write_encoded_size(branches.begin()->second.size()); //write number of indexed chunks
				for (auto& c : branches)
				{
					const char* str = c.first.c_str();
					file->write_data(str, strlen(str) + 1);

					for (auto& sub : c.second)
					{
						file->write_encoded_size(sub.size);
					}
					file->write_data("\0", 1);
				}
			}

			file->write_data("\0", 1);

			for (auto& sub : branches)
				for (auto& c : sub.second)
				{
					c.write();
				}
			break;
		default:
			break;
		}
	}

	void Files::Chunk::clean()
	{
		for (auto& d : data) delete[] d.first;
		for (auto& sub : branches) for (auto& c : sub.second) c.clean();
		branches.clear();
		data.clear();
	}
}
