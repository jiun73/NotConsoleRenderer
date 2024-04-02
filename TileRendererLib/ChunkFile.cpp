#include "pch.h"
#include "ChunkFile.h"
#include <cstring>
#include <limits>

namespace NCR {
	Files::Chunk& Files::Chunk::operator()(size_t i)
	{
		if (mode != file->mode == FILE_READING) return *this;
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

	Files::Chunk& Files::Chunk::operator[](const string& name)
	{
		if (file->mode == FILE_READING)
		{
			Chunk& chunk = branches.at(index).at(name);
			file->file.seekg(chunk.offset);
			return chunk;
		}
		else
		{
			mode = FILE_CHUNK_BRANCH;

			branches.resize(index + 1);
			if (!branches[index].count(name))
				branches[index].emplace(name, Chunk(file));

			return branches[index].at(name);
		}
	}

	size_t Files::Chunk::get_index_size(size_t i)
	{
		if (mode != FILE_CHUNK_BRANCH) return 0;

		size_t ret = 0;
		for (auto& c : branches[i])
		{
			c.second.get_total_size();
			ret += c.second.size;
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
				for (auto& c : sub)
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
		case '\2': //header indexed
		{
			size_t size = file->read_encoded_size();
			branches.resize(size);
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
					branches.at(i).emplace(chunk_name, Chunk(file));
					size_t size = file->read_encoded_size(c);
					branches.at(i).at(chunk_name).size = size;

					i++;
				}
			}
			for (auto& sub : branches)
				for (auto& c : sub)
				{
					c.second.load();
				}
		}
			break;
		case '\1': //header
		{
			mode = FILE_CHUNK_BRANCH;
			size_t size = file->read_encoded_size();
			branches.resize(size);
			while (true)
			{
				string chunk_name = file->read_string();
				if (chunk_name.empty())
				{
					for (auto& sub : branches)
					{
						sub = branches[0];
					}

					break;
				}
				branches[0].emplace(chunk_name, Chunk(file));
				size_t size = file->read_encoded_size();
				branches[0].at(chunk_name).size = size;
			}
			for (auto& sub : branches)
				for (auto& c : sub)
				{
					c.second.load();
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
				file->write_encoded_size(branches.size()); //write number of indexed chunks
				for (auto& c : branches.at(0)) //write header for the first indexed chunk
				{
					const char* str = c.first.c_str();
					file->write_data(str, strlen(str) + 1);
					file->write_encoded_size(c.second.size);
				}
			}
			else
			{
				file->write_data("\2", 1);
				file->write_encoded_size(branches.size()); //write number of indexed chunks
				for (auto& sub : branches)
				{
					bool f = true;
					
					for (auto& c : sub)
					{
						if (f) //write the names of the chunks only for the first one (we assume all index wa the same chunks)
						{
							const char* str = c.first.c_str();
							file->write_data(str, strlen(str) + 1);
							f = false;
						}

						file->write_encoded_size(c.second.size);

					}
					file->write_data("\0", 1);
				}
			}

			file->write_data("\0", 1);

			for (auto& sub : branches)
				for (auto& c : sub)
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
		for (auto& sub : branches) for (auto& c : sub) c.second.clean();
		branches.clear();
		data.clear();
	}
}
