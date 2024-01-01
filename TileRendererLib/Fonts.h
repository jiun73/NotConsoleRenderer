#pragma once
#include "Rect.h"
#include "SDL_ttf.h"
#include "File.h"

#include <array>
#include <vector>

using std::array;
using std::string;
using std::vector;

//Describes each individual glyph of a font
struct Glyph
{
	size_t font_index = 0;
	char character = ' ';
	Rect source = 0;
	int minx = 0;
	int maxx = 0;
	int miny = 0;
	int maxy = 0;
	int advance = 0;
};

class Font
{
private:
	size_t font_index;
	TTF_Font* ttf = nullptr;
	SDL_Texture* atlas = nullptr;
	array<Glyph, 256> glyphs;
	string path;
	int base_size = 0;

	V2d_i max;

public:
	Font(size_t index) : font_index(index) {}
	~Font() {}

	void load(SDL_Renderer* renderer, const string& path, int size = 16)
	{
		SDL_Surface* glyph, * font;
		Rect dest = 0;
		char character[2];

		ttf = TTF_OpenFont(path.c_str(), size);
		SDL_assert(ttf);
		font = SDL_CreateRGBSurface(0, 512, 512, 32, 0, 0, 0, 0xff);
		SDL_assert(font);

		SDL_SetColorKey(font, SDL_TRUE, SDL_MapRGBA(font->format, 0, 0, 0, 0));

		int maxyt = 0;
		max = 0;
		bool fail = false;
		for (int n = 1; n < 6; n++)
		{
			for (int i = 32; i < 256; i++)
			{
				character[0] = i;
				character[1] = 0;

				glyph = TTF_RenderUTF8_Solid(ttf, character, Color(COLOR_WHITE).SDL());
				if (!glyph)
					continue;

				//std::cout << << std::endl;
				dest.sz = { glyph->w, glyph->h };

				//dest.sz = getGlyphSize(character[0]);

				if (dest.sz.y > maxyt)
					maxyt = dest.sz.y;

				if (dest.pos.x + dest.sz.x >= 512 * n)
				{
					dest.pos.x = 0;
					dest.pos.y += maxyt;
					maxyt = 0;
					if (dest.pos.y + dest.sz.y >= 512 * n)
					{
						if (n == 5)
						{
							SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, "Out of glyph space in %dx%d font atlas texture map.", 512 * n, 512 * n);
							exit(1);
						}
						else
						{
							std::cout << "Failed with n = " << n << std::endl;
						}
						fail = true;
						break;

					}
				}

				SDL_BlitSurface(glyph, NULL, font, dest.SDL());

				int minxm, maxxm, minym, maxym, advance;
				TTF_GlyphMetrics(ttf, i, &minxm, &maxxm, &minym, &maxym, &advance);
				glyphs[i] = { font_index, (char)i, dest, minxm, maxxm, minym, maxym, advance };

				if (dest.sz.x > max.x)
					max.x = dest.sz.x;

				if (dest.sz.y > max.y)
					max.y = dest.sz.y;

				SDL_FreeSurface(glyph);
				dest.pos.x += dest.sz.x;
			}
			if (fail)
			{
				fail = false;
				SDL_FreeSurface(font);
				font = SDL_CreateRGBSurface(0, 512 * (n + 1), 512 * (n + 1), 32, 0, 0, 0, 0xff);
				SDL_SetColorKey(font, SDL_TRUE, SDL_MapRGBA(font->format, 0, 0, 0, 0));
				dest = 0;

				continue;
			}
			break;
		}
		atlas = SDL_CreateTextureFromSurface(renderer, font);
		SDL_FreeSurface(font);
	}

	const Glyph& get(const char& character)
	{
		return glyphs.at(character);
	}
};

#include "StringManip.h"

class FontsManager 
{
private:
	vector<Font> fonts;

public:
	void add_font(SDL_Renderer* renderer, const string& path, int size = 16)
	{
		Font font(fonts.size());
		font.load(renderer, path, size);
		fonts.push_back(font);
	}

	const Font& get(size_t i) 
	{
		return fonts.at(i);
	}

	void read_hint_file(SDL_Renderer* renderer, const string& path = "Fonts/fonts.hint")
	{
		File hint_file(path, FILE_READING_STRING);
		string content = hint_file.getString();

		vector<string> fonts_paths = split(content, '\n');
		for (auto& paths : fonts_paths)
		{
			vector<string> settings = split(content, ';');
			add_font(renderer, settings.at(0), stoi(settings.at(1)));
		}
	}
};