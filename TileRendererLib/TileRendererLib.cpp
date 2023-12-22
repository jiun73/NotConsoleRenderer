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

	const int fps = 60;
	Uint32 frameStart;
	Uint32 frameTime;
	Uint32 frameDelay = 1000 / fps;

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

		frameTime = SDL_GetTicks() - frameStart;

		if (frameDelay > frameTime)
			SDL_Delay(frameDelay - frameTime);
	}

	while (SDL_PollEvent(&sdl_event)) 
	{
		if (sdl_event.type == SDL_QUIT)
			return false;

		_keyboard.events(sdl_event);
		_mouse.events(sdl_event);
		_joystick.events(sdl_event);
	}

	_keyboard.update();
	_mouse.update();
	_joystick.update();

	frameStart = SDL_GetTicks();

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

