// TileRendererLib.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"

#include "TileRenderer.h"
#include "SDL_image.h"
#include "Sound.h"
#include "EntityX.h"
#include "Fonts.h"

#include <map>

using std::map;
using std::string;

namespace hidden {
	bool __init__ = false;
	bool __lock__ = false;
	bool __run__ = true;

	const int fps = 120;
	Uint32 frameStart = 0;
	Uint32 frameTime = 0;
	Uint32 frameDelay = 1000.0 / fps;

	KeyboardInput _keyboard;
	MouseInput _mouse;
	JoystickInput _joystick;

	SDL_Renderer* sdl_ren;
	SDL_Window* sdl_win;
	V2d_i window_size = { 1920,1080 };
	SDL_WindowFlags window_flags;
	SDL_Event sdl_event;

	SoundManager _sound;
	Random _random;

	FontsManager _fonts;
	map<string, SDL_Texture* > textures;
}

using namespace hidden;

void set_window_size(V2d_i size) { window_size = size; }
void set_window_resizable() { window_flags = SDL_WindowFlags(window_flags | SDL_WINDOW_RESIZABLE); }

V2d_d get_renderer_scale()
{
	float x, y;
	SDL_RenderGetScale(sdl_ren, &x, &y);
	return { (double)x,(double)y };
}

V2d_d get_window_size()
{
	int x, y;
	SDL_GetWindowSize(sdl_win, &x, &y);
	return { (double)x,(double)y };
}

V2d_d mouse_position()
{
	V2d_d mpos = mouse().position();
	mpos += (((V2d_d)window_size * get_renderer_scale()) - get_window_size()) / 2;
	mpos /= get_renderer_scale();
	return mpos;
}

void init() 
{
	if (!__init__)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
		TTF_Init();
		SDL_CreateWindowAndRenderer(window_size.x, window_size.y, window_flags, &sdl_win, &sdl_ren);
		SDL_RenderSetLogicalSize(sdl_ren, window_size.x, window_size.y);
		sound().init();	
		__init__ = true;
	}
}

bool run()
{
	
	if (!__lock__)
	{
		init();
		__lock__ = true;
	}
	else 
	{
		EntX::get()->update();

		SDL_RenderPresent(sdl_ren);

		frameTime = SDL_GetTicks() - frameStart;

		if (frameDelay > frameTime)
			SDL_Delay(frameDelay - frameTime);

		_keyboard.update();
	}

	if (!__run__)
	{
		SDL_DestroyRenderer(sdl_ren);
		SDL_DestroyWindow(sdl_win);
	}

	while (SDL_PollEvent(&sdl_event)) 
	{
		if (sdl_event.type == SDL_QUIT)
			return false;

		_keyboard.events(sdl_event);
		_mouse.events(sdl_event);
		_joystick.events(sdl_event);
	}

	
	_mouse.update();
	_joystick.update();

	frameStart = SDL_GetTicks();

	return __run__;
}

void close() { __run__ = false; }

KeyboardInput&	keyboard()	{ return _keyboard; }
MouseInput&		mouse()		{ return _mouse; }
JoystickInput&	joystick()	{ return _joystick; }

bool key_pressed(SDL_Scancode code) { return keyboard().pressed(code); }
bool key_held(SDL_Scancode code) { return keyboard().held(code); }
bool key_released(SDL_Scancode code) { return keyboard().released(code); }
bool mouse_left_pressed() { return mouse().pressed(SDL_BUTTON_LEFT); }
bool mouse_left_held() { return mouse().held(SDL_BUTTON_LEFT); }
bool mouse_left_released() { return mouse().released(SDL_BUTTON_LEFT); }
bool mouse_right_pressed() { return mouse().pressed(SDL_BUTTON_RIGHT); }
bool mouse_right_held() { return mouse().held(SDL_BUTTON_RIGHT); }
bool mouse_right_released() { return mouse().released(SDL_BUTTON_RIGHT); }

//j'ai la flemme de faire des meilleures fonctions, mais tu peut regarder dans Sound.h si tu veut jouer des sons 
//C'est pas suuuper compliqué
//Tu veut juste t'assurer de 'load' un son avant de le jouer
//Et que tes sons soit en format .wav
SoundManager& sound() { return _sound; } 

//les valeurs vont de 0 à 255
Color rgb(int red, int green, int blue) {return { (uint8_t)red,(uint8_t)green,(uint8_t)blue,255 };}

Color rainbow(int speed)
{
	double ratio = ((int)SDL_GetTicks() % (int)speed) / (double)speed;
	//we want to normalize ratio so that it fits in to 6 regions
	//where each region is 256 units long
	int normalized = int(ratio * 256 * 6);

	//find the distance to the start of the closest region
	int x = normalized % 256;

	int red = 0, grn = 0, blu = 0;
	switch (normalized / 256)
	{
	case 0: red = 255;      grn = x;        blu = 0;       break;//red
	case 1: red = 255 - x;  grn = 255;      blu = 0;       break;//yellow
	case 2: red = 0;        grn = 255;      blu = x;       break;//green
	case 3: red = 0;        grn = 255 - x;  blu = 255;     break;//cyan
	case 4: red = x;        grn = 0;        blu = 255;     break;//blue
	case 5: red = 255;      grn = 0;        blu = 255 - x; break;//magenta
	}

	Color rain = { static_cast<uint8_t>(red),static_cast<uint8_t>(grn),static_cast<uint8_t>(blu), 255 };
	return rain;
}

void pencil(Color color) { SDL_SetRenderDrawColor(sdl_ren, color.r, color.g, color.b, color.a); }
void draw_pix(V2d_i position) { SDL_RenderDrawPoint(sdl_ren, position.x, position.y); }
void draw_rect(Rect rectangle) { SDL_RenderDrawRect(sdl_ren, rectangle.SDL()); }
void draw_full_rect(Rect rectangle) { SDL_RenderFillRect(sdl_ren, rectangle.SDL()); }
void draw_line(V2d_i start_position, V2d_i end_position) { SDL_RenderDrawLine(sdl_ren, start_position.x, start_position.y, end_position.x, end_position.y); }
void draw_clear() { SDL_RenderClear(sdl_ren); }

void load_texture(const string& path) 
{
	SDL_Surface* sur = IMG_Load(path.c_str());

	SDL_assert(sur);

	SDL_Texture* tex = SDL_CreateTextureFromSurface(sdl_ren, sur);

	SDL_assert(tex);

	textures.emplace(path, tex);
}

V2d_i get_image_size(const string& path)
{
	SDL_Texture* tex = textures.at(path);

	V2d_i ret;
	SDL_QueryTexture(tex, NULL, NULL, &ret.x, &ret.y);

	return ret;
}

Random& random()
{
	return _random;
}

double lerp(double a, double b, double t)
{
	return a + t * (b - a);
}

double distance_square(const V2d_d& pos1, const V2d_d& pos2)
{
	return (pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y);
}

double distance(const V2d_d& pos1, const V2d_d& pos2)
{
	return sqrt(distance_square(pos1, pos2));
}

void draw_image(const string& path, Rect destination) 
{
	if (!textures.count(path))
	{
		load_texture(path);
	}

	SDL_RenderCopy(sdl_ren, textures.at(path), NULL, destination.SDL());
}

void draw_image_form_source(const string& path, Rect source, Rect destination)
{
	if (!textures.count(path))
	{
		load_texture(path);
	}

	SDL_RenderCopy(sdl_ren, textures.at(path), source.SDL(), destination.SDL());
}

void draw_circle(V2d_i pos, int radius)
{
	const int32_t diameter = (radius * 2);

	int32_t centreX = pos.x;
	int32_t centreY = pos.y;

	int32_t x = (radius - 1);
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = (tx - diameter);

	while (x >= y)
	{
		//  Each of the following renders an octant of the circle
		SDL_RenderDrawPoint(sdl_ren, centreX + x, centreY - y);
		SDL_RenderDrawPoint(sdl_ren, centreX + x, centreY + y);
		SDL_RenderDrawPoint(sdl_ren, centreX - x, centreY - y);
		SDL_RenderDrawPoint(sdl_ren, centreX - x, centreY + y);
		SDL_RenderDrawPoint(sdl_ren, centreX + y, centreY - x);
		SDL_RenderDrawPoint(sdl_ren, centreX + y, centreY + x);
		SDL_RenderDrawPoint(sdl_ren, centreX - y, centreY - x);
		SDL_RenderDrawPoint(sdl_ren, centreX - y, centreY + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}

FontsManager& fonts() 
{
	return _fonts;
}

void load_font(const string& path)
{
	_fonts.add_font(sdl_ren, path);
}

const Font& get_font(size_t i)
{
	return _fonts.get(i);
}

int draw_glyph(const char& character, const V2d_i& pos, const Font& font)
{
	const Glyph& glyph = font.get(character);
	Rect dest = { pos, glyph.source.sz };
	SDL_RenderCopy(sdl_ren, font.atlas, &glyph.sdl_src, dest.SDL());
	return glyph.advance;
}

void draw_line(const string& text, const V2d_i& pos, const Font& font)
{
	int counter = pos.x;
	for (auto& c : text)
	{
		counter += draw_glyph(c, { counter, pos.y }, font);
	}
}

