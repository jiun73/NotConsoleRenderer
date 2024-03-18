#pragma once
#include "Rect.h"
#include <map>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::string;

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
	map<string, Rect> colliders;
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
	void readwrite(File& file) 
	{
		
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