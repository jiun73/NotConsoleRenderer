// TileRendererLib.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"

#include "TileRenderer.h"
#include "SDL_image.h"
#include "EntityX.h"
#include "Fonts.h"
#include "Camera.h"
#include "CommandPrompt.h"
#include "Controls.h"

#define _WINSOCKAPI_

//#include "ConsoleRenderer.h"

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
	Uint32 frameDelay = (Uint32)(1000.0 / fps);

	//KeyboardInput _keyboard;
	//MouseInput _mouse;
	//JoystickInput _joystick;
	MultiInput _inputs;

	SDL_Renderer* sdl_ren;
	SDL_Window* sdl_win;
	V2d_i window_size = { 1920,1080 };
	Vector2D<int> draw_offset = 0;
	SDL_WindowFlags window_flags;
	SDL_Event sdl_event;

	SoundManager _sound;
	Random _random;

	FontsManager _fonts;
	map<string, SDL_Texture* > textures;

	Peer2Peer _net;
	ConsoleApp cApp; //lol
}

using namespace hidden;

Camera hidden::_camera;

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
	return mpos + V2d_d(draw_offset);
}

V2d_d game_mouse_position()
{
	return _camera.inverse(mouse_position());
}

Camera& camera() { return _camera; }

#include <Windows.h>

void init() 
{
	if (!__init__)
	{
		Threads::get()->start();

		SDL_Init(SDL_INIT_EVERYTHING);
		TTF_Init();
		SDL_CreateWindowAndRenderer(window_size.x, window_size.y, window_flags, &sdl_win, &sdl_ren);
		SDL_RenderSetLogicalSize(sdl_ren, window_size.x, window_size.y);
		sound().init();	
		fonts().read_hint_file(sdl_ren);
		track_variable(net::verbose_net, "net_debug");
		__init__ = true;

		__NEW_COMMAND__(test, "test", [](__COMMAND_ARGS__)
			{
				std::cout << "You tested alright!" << std::endl;
			});

		__NEW_COMMAND__(help, "help", [](__COMMAND_ARGS__)
			{
				std::cout << "This is the command center!" << std::endl;
				std::cout << "you can enter commands or define your own!" << std::endl;
				std::cout << "This is somewhat useful if you want to peek quickly at some variables or something" << std::endl;
				std::cout << "Or maybe you need to setup a specific situation for bug testing... ?" << std::endl;
				std::cout << "Idk you do what you what!" << std::endl;
				std::cout << "special: *** means that the argument can be any string" << std::endl;
				std::cout << "special: [...] means that the argument has additionnal (and optionnal) arguments at the end" << std::endl;
				std::cout << "special: (int) means... huh... let me check..." << std::endl;
			});

		__NEW_COMMAND__(list_entries, "list all", [](__COMMAND_ARGS__)
			{
				for (auto& reg : variable_dictionnary()->all())
					for (auto& e : reg->all())
						std::cout << e.second->type().name() << " " << e.first << " = " << e.second->stringify() << std::endl;
			});

		__NEW_COMMAND__(ff3, "! *** = ***", [&](__COMMAND_ARGS__)
			{
				std::cout << variable_dictionnary()->get(args[1])->destringify(args[3]) << std::endl;
				std::cout << variable_dictionnary()->get(args[1])->stringify() << std::endl;

			});

		DefineCommand ff4("pause", [](__COMMAND_ARGS__)
			{
				int* b = new int(0);

				CommandSpace space(":");

				space.temp.push_back({ "step (int)", [b](CommandArguments& args)
					{
						*b = args(2);
					} });

				space.temp.push_back({ "close", [b, space](__COMMAND_ARGS__)
					{
						
						Commands::get()->call_once.push_back([b, space]()
							{
								std::cout << "Unpausing..." << std::endl;
								delete b;
								Commands::get()->exit_space(space);
							});
					} });

				space.callback = [b]()
					{
						std::cout << "Pausing... " << *b << std::endl;
						auto& call_ref = Commands::get()->temp_spaces.at(":").callback;
						auto old = call_ref;
						call_ref = nullptr;
						if(*b == 0)
							Commands::get()->update(true);
						CommandPrompt* p = Commands::get();
						while (*b == 0) 
						{ 
							Commands::get()->update();
							if (!Commands::get()->temp_spaces.count(":")) return;
						};
						
						call_ref = old;
						(*b)--;

						if (*b < 0)
							*b = 0;
					};

				Commands::get()->enter_space(space);
			});

		DefineCommand _ff_cmd("open watch", [&](CommandArguments& args)
			{
				//PROCESS_INFORMATION processInformation;
				//STARTUPINFO         startupInfo;
				////wchar_t  lpAppName =;

				//ZeroMemory(&startupInfo, sizeof(startupInfo));
				//startupInfo.cb = sizeof(startupInfo);

				//BOOL creationResult = CreateProcess(
				//	L"c:\\windows\\system32\\cmd.exe",                   // No module name (use command line)
				//	NULL,                // Command line
				//	NULL,                   // Process handle not inheritable
				//	NULL,                   // Thread handle not inheritable
				//	FALSE,                  // Set handle inheritance to FALSE
				//	CREATE_NEW_CONSOLE , // creation flags
				//	NULL,                   // Use parent's environment block
				//	NULL,                   // Use parent's starting directory 
				//	&startupInfo,           // Pointer to STARTUPINFO structure
				//	&processInformation);   // Pointer to PROCESS_INFORMATION structure

				//FreeConsole();
				//AttachConsole(processInformation.dwProcessId);

				//assert(false);
				//std::cout << creationResult << std::endl;

				vector<std::pair<string, shared_generic>>* watching = new vector<std::pair<string, shared_generic>>();

				CommandSpace space("watch");

				space.temp.push_back({ "set ***", [watching](__COMMAND_ARGS__)
					{
						string name = args[1];
						shared_generic ptr = variable_dictionnary()->get(name);
						if (ptr != nullptr)
							watching->push_back({ name,ptr });
						else
							std::cout << "couldn't find the variable lol" << std::endl;
					} });

				space.temp.push_back({ "all", [watching](__COMMAND_ARGS__)
					{
						for (auto& reg : variable_dictionnary()->all())
						{
							for (auto& var : reg->all())
							{
								watching->push_back({var.first, var.second});
							}
						}
					} });

				space.temp.push_back({ "close", [watching, space](__COMMAND_ARGS__)
					{
						Commands::get()->call_once.push_back([space]()
							{
								Commands::get()->exit_space(space);
							});
						
					} });

				space.callback = [watching]()
					{
						std::string output;

						if (watching->size() == 0) 
						{
							output = "Nothing to watch :(\n";
						}
						else
						{
							output = "Watching \\/ \n";
						}

						for (auto& w : *watching)
						{
							output += w.first + " = " + w.second->stringify() + "\n";
						}

						

						V2d_i old = cApp.getCursor();
						cApp.pencil(BLACK, BG_WHITE, '#');
						cApp.text(output, {50,0});
						cApp.color(WHITE, BG_BLACK);
						cApp.present();
						cApp.color(WHITE, BG_BLACK);

						cApp.setCursor(old.xi());
						
					};

				Commands::get()->enter_space(space);
			});

		add_font_effect("end", [](__FontEffectArgs__)
			{
				_fonts.remove_glyph_effect();
			});

		add_font_effect("wave", [](__FontEffectArgs__)
			{
				_fonts.set_glyph_effect([](SDL_Rect& dest)
					{
						int xdis = (int)sin((SDL_GetTicks() / 100.0) + dest.x * 5) * 5;
						int ydis = (int)cos((SDL_GetTicks() / 100.0) + dest.x * 5) * 5;
						dest.x += xdis;
						dest.y += ydis;
					});
			});

		add_font_effect("rainbow", [](__FontEffectArgs__)
			{
				_fonts.set_glyph_effect([&](SDL_Rect& dest)
					{
						font.set_color(rainbow(1000, dest.x * 2));
					});
			});

		add_font_effect("shake", [](__FontEffectArgs__)
			{
				_fonts.set_glyph_effect([&](SDL_Rect& dest)
					{
						const double force = 5.0;
						dest.x += (int)random().frange(-force, force);
						dest.y += (int)random().frange(-force, force);
					});
			});


		add_font_effect("color", [](__FontEffectArgs__)
			{

			});
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
		if (_inputs.keyboard.pressed(SDL_SCANCODE_F3))
		{
			if (!Commands::get()->isPolling())
			{
				std::cout << "Debug mode started!" << std::endl;
				Commands::get()->startPolling();
			}
			else
			{
				std::cout << "Exiting Debug mode!" << std::endl;
				Commands::get()->stopPolling();
			}
		}

		EntX::get()->update();
		Commands::get()->update();

		SDL_RenderPresent(sdl_ren);

		frameTime = SDL_GetTicks() - frameStart;

		if (frameDelay > frameTime)
			SDL_Delay(frameDelay - frameTime);

		_inputs.keyboard.update();
	}

	if (!__run__)
	{
		SDL_DestroyRenderer(sdl_ren);
		SDL_DestroyWindow(sdl_win);
	}

	while (SDL_PollEvent(&sdl_event)) 
	{
		if (sdl_event.type == SDL_QUIT) return false;

		_inputs.events(sdl_event);

		//_keyboard.events(sdl_event);
		//_mouse.events(sdl_event);
		//_joystick.events(sdl_event);
	}

	
	_inputs.update();

	frameStart = SDL_GetTicks();

	return __run__;
}

void close() { __run__ = false; }

MultiInput& inputs()
{
	return _inputs;
}

KeyboardInput&	keyboard()	{ return _inputs.keyboard; }
MouseInput&		mouse()		{ return _inputs.mouse; }
JoystickInput&	joystick()	{ return _inputs.joystick; }

bool key_pressed(SDL_Scancode code) { return keyboard().pressed(code); }
bool key_held(SDL_Scancode code) { return keyboard().held(code); }
bool key_released(SDL_Scancode code) { return keyboard().released(code); }
bool mouse_left_pressed() { return mouse().pressed(SDL_BUTTON_LEFT); }
bool mouse_left_held() { return mouse().held(SDL_BUTTON_LEFT); }
bool mouse_left_released() { return mouse().released(SDL_BUTTON_LEFT); }
bool mouse_right_pressed() { return mouse().pressed(SDL_BUTTON_RIGHT); }
bool mouse_right_held() { return mouse().held(SDL_BUTTON_RIGHT); }
bool mouse_right_released() { return mouse().released(SDL_BUTTON_RIGHT); }

bool input(const string& str) { return _inputs.check(str); }

//j'ai la flemme de faire des meilleures fonctions, mais tu peux regarder dans Sound.h si tu veut jouer des sons 
//C'est pas suuuper compliqué
//Tu veux juste t'assurer de 'load' un son avant de le jouer
//Et que tes sons soit en format .wav
SoundManager& sound() { return _sound; } 

//les valeurs vont de 0 à 255
Color rgb(int red, int green, int blue) {return { (uint8_t)red,(uint8_t)green,(uint8_t)blue,255 };}

Color rainbow(int speed, int delay)
{
	double ratio = (((int)SDL_GetTicks() + delay) % (int)speed) / (double)speed;

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
void draw_pix(V2d_i position) { position += draw_offset;  SDL_RenderDrawPoint(sdl_ren, position.x, position.y); }
void draw_rect(Rect rectangle) { rectangle.pos += draw_offset; SDL_RenderDrawRect(sdl_ren, rectangle.SDL()); }
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

bool point_in_rectangle(const V2d_i& point, const Rect& rectangle)
{
	return	((point.x >= rectangle.pos.x) && (point.x <= rectangle.pos.x + rectangle.sz.x)) && 
			((point.y >= rectangle.pos.y) && (point.y <= rectangle.pos.y + rectangle.sz.y));
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

Peer2Peer& p2p() { return hidden::_net; }

Peer2Peer& p2p(size_t i) { return hidden::_net[i]; }

void draw_image(const string& path, Rect destination) 
{
	if (!textures.count(path))
	{
		load_texture(path);
	}

	SDL_RenderCopy(sdl_ren, textures.at(path), NULL, destination.SDL());
}

void draw_image_from_source(const string& path, Rect source, Rect destination)
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

void add_font_effect(const string& name, FontEffect_f effect)
{
	_fonts.add_effect(name, effect);
}

FontID load_font(const string& path)
{
	return _fonts.add_font(sdl_ren, path);
}

const Font& get_font(FontID i)
{
	return _fonts.get(i);
}

void set_font_pencil(const Color& color, const Font& font)
{
	font.set_color(color);
}

int draw_glyph(const char& character, const V2d_i& pos, const Font& font, int kerning)
{
	const Glyph& glyph = font.get(character);
	SDL_Rect dest = glyph.sdl_dest;
	dest.x += pos.x + kerning;
	dest.y += pos.y;
	_fonts.apply_effect(dest);
	SDL_RenderCopy(sdl_ren, font.atlas, &glyph.sdl_src, &dest);
	return glyph.advance;
}

void draw_simple_text(const string& text, const V2d_i& pos, const Font& font)
{
	TextDrawCounter counter(font, pos);
	for (auto& c : text)
	{
		if (c == '\n')
		{
			//xcounter = pos.x;
			//ycounter += font.lineskip;
			counter.linebreak();
			continue;
		}
		//xcounter += draw_glyph(c, { xcounter, ycounter }, font);

		counter.addx(draw_glyph(c, counter.pos, font));
	}
}

void draw_text(const string& text, const int& max_width, const V2d_i& pos, const Font& font)
{
	string_range range = { text.cbegin(), text.cend() };

	TextDrawCounter counter(font, pos);

	for (auto lines : all_ranges(range, get_text_range_until, '\n'))
	{
		for (auto max : all_ranges<int, const Font&>(lines, get_text_range_max_size, max_width, font))
		{
			draw_line_range(max.first, max.second, counter, font);
			counter.linebreak();
		}
	}
}

void draw_special_text(const string& text, const int& max_width, const V2d_i& pos, const Font& font)
{
	string_range range = { text.cbegin(), text.cend() };

	TextDrawCounter counter(font, pos);

	for (auto lines : all_ranges(range, get_text_range_until, '\n'))
	{
		std::vector<bool> isSpecial;
		size_t i = 0;
		for (auto special : all_ranges<std::vector<bool>&>(lines, get_text_range_special, isSpecial))
		{
			if (special.first == special.second) { i++; continue; }

			if (isSpecial.at(i))
			{
				std::string cmd(special.first, special.second);
				_fonts.set_font_effect(font, cmd, counter);
			}
			else
			{
				for (auto max = special.first; max != special.second; max++)
				{
					counter.linebreak_if(*max, max_width);
					counter.addx(draw_glyph(*max, counter.pos, font, 0));
				}	
			}
			i++;
		}
		
	}

	_fonts.remove_glyph_effect();
}

void hidden::draw_line_range(string::const_iterator begin, string::const_iterator end, TextDrawCounter& counter, const Font& font)
{
	/*char prev = 0;*/
	for (auto it = begin; it != end; it++) 
	{
		counter.addx(draw_glyph(*it, counter.pos, font));
		//int kerning = 0;
		///*if (prev != 0)
		//{
		//	kerning = font.get_kerning((Uint16)prev, (Uint16)*it); TODO: kerning?
		//}*/
		//prev = *it;
	}
}

void draw_line(const string& text, const V2d_i& pos, const Font& font)
{
	TextDrawCounter counter(font, pos);
	draw_line_range(text.cbegin(), text.cend(), counter, font);
}

int hidden::get_text_draw_size(string::const_iterator begin, string::const_iterator end, const Font& font)
{
	int counter = 0;
	for (auto it = begin; it != end; it++)
	{
		const Glyph& glyph = font.get(*it);
		counter += glyph.advance;
	}
	return counter;
}

string_range hidden::get_text_range_max_size(string_range range, int max_size, const Font& font)
{
	int counter = 0;
	for (auto it = range.first; it != range.second; it++)
	{
		const Glyph& glyph = font.get(*it);
		counter += glyph.advance;

		if (counter > max_size)
			return { range.first, it };
	}

	return range;
}

string_range hidden::get_text_range_until(string_range range, char c)
{
	for (auto it = range.first; it != range.second; it++)
	{
		if (*it == c)
			return { range.first, it + 1 };
	}
	return range;
}

string_range hidden::get_text_range_special(string_range range, vector<bool>& is_special)
{
	const char sp_char = '\\';
	const char end_char = '.';

	if (range.first == range.second) 
	{
		is_special.push_back(0);
		return range; 
	}

	if (*range.first == sp_char)
	{
		for (auto it = std::next(range.first); it != range.second; it++)
		{


			if (*it == end_char)
			{
				is_special.push_back(1);
				return { std::next(range.first), std::next(it) };
			}
		}

		is_special.push_back(0);

		return range;
	}

	for (auto it = range.first; it != range.second; it++)
	{
		if (it != range.first && *it == sp_char && std::next(it) != range.second)
		{
			is_special.push_back(0);
			return { range.first, it };
		}
	}
	is_special.push_back(0);
	return range;
}

int get_text_draw_size(const string& text, const Font& font)
{
	return hidden::get_text_draw_size(text.cbegin(), text.cend(), font);
}

