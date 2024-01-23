#pragma once
#include "Rect.h"
#include "SDL_ttf.h"
#include "File.h"

#include <array>
#include <functional>
#include <map>
#include <vector>
#include <unordered_map>

using std::array;
using std::map;
using std::unordered_map;
using std::string;
using std::vector;

//Describes each individual glyph of a font
struct Glyph
{
	size_t	font_index = 0;
	Uint16	character = 0;
	Rect	source = 0;
	SDL_Rect sdl_dest = { 0,0,0,0 };
	SDL_Rect sdl_src = {0,0,0,0};
	V2d_i	min = 0;
	V2d_i	max = 0;
	int		advance = 0;
};

inline uint32_t next_2pow(uint32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

class Font
{
public:
	TTF_Font* ttf = nullptr;
	size_t font_index;
	string path;	
	string name;

	SDL_Texture* atlas = nullptr;
	unordered_map<Uint16, Glyph> glyphs;

	int hinting = TTF_HINTING_NONE;
	int style = TTF_STYLE_NORMAL;

	int lineskip = 0;
	int height = 0;
	int ascent = 0;
	int descent = 0;
	bool fixedWidth = false;

	bool kerning = false;

	Font(size_t index) : font_index(index), ttf(nullptr) {}
	~Font() {}

	void set_color(const Color& color) const
	{
		SDL_SetTextureColorMod(atlas, color.r, color.g, color.b);
	}

	int get_kerning(Uint16 prev, Uint16 ch) const
	{
		return TTF_GetFontKerningSizeGlyphs(ttf, prev, ch);
	}

	int count_valid_glyphs(size_t max)
	{
		int ret = 0;
		for (Uint32 character = 0; character < max; character++)
		{
			if (!TTF_GlyphIsProvided(ttf, character)) continue;
			ret++;
		}
		return ret;
	}

	void load(SDL_Renderer* renderer, const string& path, int size = 8, int quality = 1)
	{
		Rect current_dest;
		string current_character;

		size *= quality;

		ttf = TTF_OpenFont(path.c_str(), size);
		height = TTF_FontHeight(ttf);

		const size_t max_char = 0x10000;

		size_t estimated_area = count_valid_glyphs(max_char) * (size_t)height * (size_t)height;
		V2d_i surface_size = next_2pow((uint32_t)sqrt(estimated_area));

		//std::cout << "estimated size for atlas: " << surface_size << std::endl;

		SDL_Surface* atlas_sur = SDL_CreateRGBSurface(0, surface_size.x, surface_size.y, 32, 0, 0, 0, 0xff);

		{
			SDL_assert(ttf);
			SDL_assert(atlas_sur);
		}

		if (atlas_sur)
		{
			SDL_SetColorKey(atlas_sur, SDL_TRUE, SDL_MapRGBA(atlas_sur->format, 0, 0, 0, 0));

			lineskip = TTF_FontLineSkip(ttf) / quality;
			hinting = TTF_GetFontHinting(ttf);
			fixedWidth = TTF_FontFaceIsFixedWidth(ttf);
			ascent = TTF_FontAscent(ttf);
			descent = TTF_FontDescent(ttf);
			style = TTF_GetFontStyle(ttf);
			kerning = TTF_GetFontKerning(ttf);
			name = TTF_FontFaceStyleName(ttf);

			int maxy = 0;
			for (Uint32 character = 0; character < max_char; character++)
			{
				if (!TTF_GlyphIsProvided(ttf, character)) continue;

				Glyph new_glyph;
				TTF_GlyphMetrics(ttf, character, &new_glyph.min.x, &new_glyph.min.y, &new_glyph.max.x, &new_glyph.max.y, &new_glyph.advance);
				new_glyph.font_index = font_index;
				new_glyph.character = character;

				SDL_Surface* glyph_sur = TTF_RenderGlyph_Solid(ttf, character, Color(COLOR_WHITE).SDL());
				if (!glyph_sur)
				{
					continue;
				}


				current_dest.sz = { glyph_sur->w, glyph_sur->h };
				maxy = std::max(maxy, current_dest.sz.y);

				if (current_dest.pos.x + current_dest.sz.x >= surface_size.x)
				{
					current_dest.pos.x = 0;
					current_dest.pos.y += maxy;
					maxy = 0;

					if (current_dest.pos.y > surface_size.y)
					{
						SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, "Out of glyph space in %dx%d font '%s' atlas texture map.", surface_size.x, surface_size.y, name.c_str());
						//exit(0);
					}
				}

				SDL_Rect sdl_dest = { current_dest.pos.x, current_dest.pos.y, current_dest.sz.x, current_dest.sz.y };

				SDL_UpperBlit(glyph_sur, NULL, atlas_sur, &sdl_dest);

				new_glyph.source = current_dest;
				new_glyph.sdl_src = sdl_dest;

				if (fixedWidth)
					new_glyph.sdl_dest.x = 0;
				else
					new_glyph.sdl_dest.x = new_glyph.min.x / quality;
				new_glyph.sdl_dest.y = 0;
				new_glyph.sdl_dest.w = new_glyph.sdl_src.w / quality;
				new_glyph.sdl_dest.h = new_glyph.sdl_src.h / quality;

				new_glyph.advance /= quality;

				current_dest.pos.x += current_dest.sz.x;

				glyphs.emplace(character, new_glyph);
				SDL_FreeSurface(glyph_sur);
			}
			atlas = SDL_CreateTextureFromSurface(renderer, atlas_sur);
			SDL_FreeSurface(atlas_sur);
		}
	}

	const Glyph& get(const char& character) const
	{
		if(glyphs.count(character))
			return glyphs.at(character);
		return glyphs.at(' ');
	}
};

#include "StringManip.h"

typedef size_t FontID;

class TextDrawCounter
{
private:
	const Font* font;
	V2d_i base;

public:
	V2d_i pos = 0;

	TextDrawCounter(const Font& font, V2d_i base) : font(&font), base(base), pos(base) {}
	~TextDrawCounter() {}

	void addx(char c) { pos.x += font->get(c).advance; }
	void addx(int x) { pos.x += x; }
	void linebreak_if(char c, int max) 
	{
		int advance = font->get(c).advance;
		if (pos.x + advance > base.x + max)
		{
			linebreak();
		}
	}
	void addy() { pos.y += font->lineskip; }
	void linebreak() { pos.x = base.x; addy(); }
};

#define __FontEffectArgs__ const string& command, const Font& font, TextDrawCounter& counter
typedef std::function<void(__FontEffectArgs__)> FontEffect_f;
typedef std::function<void(SDL_Rect&)> GlyphEffect_f;

class FontsManager 
{
private:
	vector<Font> fonts;
	map<string, FontEffect_f> effects;
	vector< GlyphEffect_f> glyph_effects;

public:
	FontID add_font(SDL_Renderer* renderer, const string& path, int size = 64)
	{
		Font font(fonts.size());
		font.load(renderer, path, size);
		fonts.push_back(font);
		std::cout << "Font " << font.name << " loaded" << std::endl;
		return fonts.size() - 1;
	}

	const Font& get(FontID i)
	{
		return fonts.at(i);
	}

	void read_hint_file(SDL_Renderer* renderer, const string& path = "Fonts/fonts.hint")
	{
		File hint_file(path, FILE_READING_STRING);
		string content = hint_file.getString();

		size_t sz = 0;
		vector<string> fonts_paths = split(content, '\n');
		for (auto& paths : fonts_paths)
		{
			vector<string> settings = split(paths, ';');
			if (settings.size() != 2)
			{
				std::cout << "font at line " << sz << " has no size!" << std::endl;
				continue;
			}
			add_font(renderer, settings.at(0), stoi(settings.at(1)));
			sz++;
		}

		std::cout << sz << " fonts loaded from hint file" << std::endl;
	}

	void add_effect(const string& name, FontEffect_f callback)
	{
		effects.emplace(name, callback);
	}

	void remove_glyph_effect()
	{
		glyph_effects.clear();
	}

	void set_glyph_effect(GlyphEffect_f effect)
	{
		glyph_effects.push_back(effect);
	}

	void set_font_effect(const Font& font, const string& command, TextDrawCounter& counter)
	{
		vector<string> args = split(command, '|');

		if (args.size() == 0) return;
		if (args.size() > 2) return;

		args.at(0).pop_back(); //removes the . at the end
		
		if(args.size() == 1)
			effects.at(args.at(0))("", font, counter);
		else
			effects.at(args.at(0))(args.at(1), font, counter);
	}

	void apply_effect(SDL_Rect& dest)
	{
		for (auto& func : glyph_effects)
		{
			func(dest);
		}
	}
};