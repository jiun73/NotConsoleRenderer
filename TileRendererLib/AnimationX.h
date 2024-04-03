#pragma once
#include "Rect.h"
#include <map>
#include <vector>
#include <string>

#include "ChunkFile.h"
#include "NotConsoleRenderer.h"

#include "SDL.h"

using std::map;
using std::vector;
using std::string;
using NCR::Files::raw;

#undef min
#undef max

struct AnimationFrameX
{
	size_t tex;
	Rect source;
	bool full_source = true;
	uint32_t time = 1000;
	V2d_i origin = 0;
	map<string, V2d_i> anchors;
};

class AnimationX
{
public:
	vector<SDL_Texture*> textures;
	vector<AnimationFrameX> frames;
	string name;
	uint32_t last = 0;
	uint32_t overshoot = 0;
	size_t currentFrame = 0;

	V2d_d scale = { 1,1 };
	V2d_i position = 0;

public:
	void readwrite(NCR::File& file, string s) 
	{
		auto& f_anim = file("data").value_as(s, name);
		auto& f_fram = f_anim("frames");
		auto& f_text = f_anim("textures");
		for (auto& f : f_fram.iterate(frames))
		{
			f_fram("frame_data") + f.tex + f.full_source + f.origin + f.source + f.time;
			f_fram("anchors").make_list(f.anchors);
			f_fram.next_index();
		}
		for (auto& t : f_text.iterate(textures))
		{
			size_t size = 0;
			int w = 0;
			int h = 0;
			char* data;

			if (file.is_writing())
			{
				data = (char*)get_texture_data(t, size, w, h);
			}
			else if (file.is_reading())
			{
				data = f_text("texture_data").list<char>(size);
			}

			f_text("texture_data") + raw(data, size);
			f_text("size") + w + h;
			f_text.next_index();
		}

		//if (file.current_mode() == NCR::Files::FILE_WRITING)
		//{
		//	auto& f_anim = file("data")(name);
		//	auto& f_fram = f_anim("frames");
		//	auto& f_text = f_anim("textures");
		//	for (auto& f : frames)
		//	{
		//		f_fram("frame_data") << f.tex << f.full_source << f.origin << f.source << f.time;
		//		//f_fram("frame_data") << 'A';
		//		f_fram("anchors");
		//		for (auto& a : f.anchors) f_fram("anchors") << a;
		//		f_fram << NCR::Files::next;
		//	}
		//	for (auto& t : textures)
		//	{
		//		size_t size = 0;
		//		int w = 0;
		//		int h = 0;
		//		char* data = (char*)get_texture_data(t, size, w, h);
		//		auto tex = get_texture_from_data(data, w, h);
		//		f_text("texture_data") << raw(data, size);
		//		f_text("size") << w << h;
		//		f_text << NCR::Files::next;
		//	}
		//}
		//else if (file.current_mode() == NCR::Files::FILE_READING)
		//{
		//	name = s;
		//	auto& f_anim = file("data")(s);
		//	auto& f_fram = f_anim("frames");
		//	auto& f_text = f_anim("textures");
		//	for (size_t i = 0; i < f_fram.index_size(); i++)
		//	{
		//		AnimationFrameX f;
		//		f_fram[i]("frame_data") >> f.tex >> f.full_source >> f.origin >> f.source >> f.time;
		//		size_t size = 0;
		//		auto list = f_fram[i]("anchors").list<typename decltype(f.anchors)::value_type>(size);
		//		for (size_t i = 0; i < size; i++)
		//		{
		//			f.anchors.emplace(list[i].first, list[i].second);
		//		}
		//		frames.push_back(f);
		//	}
		//	for (size_t i = 0; i < f_text.index_size(); i++)
		//	{
		//		size_t size = 0;
		//		int w = 0;
		//		int h = 0;
		//		char* data = f_text[i]("texture_data").list<char>(size);
		//		f_text[i]("size") >> w >> h;
		//		auto tex = get_texture_from_data(data, w, h);
		//		textures.push_back(tex);
		//	}
			
		//}
	}

	SDL_Texture* frame_texture(const AnimationFrameX& frame)
	{
		return textures.at(frame.tex);
	}

	V2d_i frame_size(const AnimationFrameX& frame)
	{
		if (frame.full_source)
		{
			int x, y;
			SDL_QueryTexture(frame_texture(frame), NULL, NULL, &x, &y);
			return { x,y };
		}

		return frame.source.sz;
	}

	V2d_i max_frame_size()
	{
		V2d_i max = 0;
		for (auto& f : frames)
		{
			V2d_i size = frame_size(f);
			max.max(size);
		}
		return max;
	}

	V2d_i maxpos()
	{
		V2d_i posmax = 0;
		bool t = false;

		for (auto& f : frames)
		{
			Rect dest;
			dest.pos = f.origin;
			dest.sz = frame_size(f);

			if (!t)
			{
				posmax = dest.pos + dest.sz;
			}
			else
			{
				posmax.max(dest.pos + dest.sz);
			}
		}

		return posmax;
	}

	V2d_i minpos()
	{
		V2d_i posmin = 0;
		bool t = false;

		for (auto& f : frames)
		{
			Rect dest;
			dest.pos = f.origin;
			dest.sz = frame_size(f);

			if (!t)
			{
				posmin = dest.pos;
			}
			else
			{
				posmin.min(dest.pos);
			}
		}

		return posmin;
	}

	V2d_i max_animation_size()
	{
		V2d_i posmin = minpos();
		V2d_i posmax = maxpos();

		return { abs(posmin.x - posmax.x), abs(posmin.y - posmax.y) };
	}

	void next_frame()
	{
		currentFrame++;
		if (currentFrame >= frames.size())
			currentFrame = 0;
	}

	AnimationFrameX& current_frame()
	{
		return frames.at(currentFrame);
	}

	void update()
	{
		uint32_t time = SDL_GetTicks();
		int64_t elapsed = int64_t(time - last + overshoot);
		overshoot = 0;

		while (elapsed > (int64_t)current_frame().time)
		{
			elapsed -= (int64_t)current_frame().time;
			next_frame();
		}

		overshoot = (uint32_t)elapsed;

		last = SDL_GetTicks();
	}

	void render(SDL_Renderer* ren, bool always_top_left = false)
	{
		update();

		if (frames.empty()) return;

		AnimationFrameX& frame = current_frame();
		SDL_Texture* tex = frame_texture(frame);
		Rect dest;
		if(always_top_left)
			dest.pos = V2d_d(position);
		else
			dest.pos = V2d_d(position) - (V2d_d(frame.origin) * scale);
		dest.sz = V2d_d(frame_size(frame)) * scale;

		SDL_RenderCopy(ren, tex, NULL, dest.SDL());
	}
};