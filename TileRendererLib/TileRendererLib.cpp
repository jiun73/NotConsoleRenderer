// TileRendererLib.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"

#include "TileRenderer.h"
#include "SDL_image.h"
#include "Sound.h"

#include <map>

using std::map;
using std::string;

namespace hidden {
	bool __init__ = false;
	bool __run__ = true;
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

	map<string, SDL_Texture* > textures;
}

using namespace hidden;

void set_window_size(V2d_i size) { window_size = size; }
void set_window_resizable() { window_flags = SDL_WindowFlags(window_flags | SDL_WINDOW_RESIZABLE); }

bool run()
{
	if (!__init__)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
		SDL_CreateWindowAndRenderer(window_size.x, window_size.y, window_flags, &sdl_win, &sdl_ren);
		SDL_RenderSetLogicalSize(sdl_ren, window_size.x, window_size.y);
		__init__ = true;
	}
	else
	{
		SDL_RenderPresent(sdl_ren);
	}

	if (!__run__)
	{
		SDL_DestroyRenderer(sdl_ren);
		SDL_DestroyWindow(sdl_win);
	}

	while (SDL_PollEvent(&sdl_event)) 
	{
		_keyboard.events(sdl_event);
		_mouse.events(sdl_event);
		_joystick.events(sdl_event);
	}

	_keyboard.update();
	_mouse.update();
	_joystick.update();

	return __run__;
}

void close() { __run__ = false; }

KeyboardInput&	keyboard()	{ return _keyboard; }
MouseInput&		mouse()		{ return _mouse; }
JoystickInput&	joystick()	{ return _joystick; }

//j'ai la flemme de faire des meilleures fonctions, mais tu peut regarder dans Sound.h si tu veut jouer des sons 
//C'est pas suuuper compliqué
//Tu veut juste t'assurer de 'load' un son avant de le jouer
//Et que tes sons soit en format .wav
SoundManager& sound() { return _sound; } 

//les valeurs vont de 0 à 255
Color rgb(int red, int green, int blue) {return { (uint8_t)red,(uint8_t)green,(uint8_t)blue,255 };}

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

Random& random()
{
	return _random;
}

void draw_image(const string& path, Rect destination) 
{
	if (!textures.count(path))
	{
		load_texture(path);
	}

	SDL_RenderCopy(sdl_ren, textures.at(path), NULL, destination.SDL());
}

