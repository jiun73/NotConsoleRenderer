#include "ConsoleRenderer.h"



#include "pch.h"

#include <Windows.h>
#define _WINSOCKAPI_

void ConsoleApp::setConsoleToCursor()
{
	COORD pos = { (short)cursor.x, (short)cursor.y };
	SetConsoleCursorPosition(console, pos);
}

//met le curseur de la console a une position donnée
void ConsoleApp::setCursor(const V2d_i& pos)
{
	cursor = pos;
	setConsoleToCursor();
}

V2d_i ConsoleApp::getCursor()
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(console, &info);

	return {info.dwCursorPosition.X, info.dwCursorPosition.Y};
}

//change le pinceau
void ConsoleApp::setConsoleSettings(const ConsolePixel& pixel)
{
	_pencil = pixel;
	setConsoleColor(pixel.fg, pixel.bg);
}

 void ConsoleApp::setConsoleColor(const fgConsoleColors& fg, const bgConsoleColors& bg) { SetConsoleTextAttribute(console, fg | bg); }

//constructeur de ConsoleApp
ConsoleApp::ConsoleApp()
{
	console = GetStdHandle(STD_OUTPUT_HANDLE);
	input = GetStdHandle(STD_INPUT_HANDLE);
	hideCursor();

	//SetConsoleMode(input, ENABLE_EXTENDED_FLAGS);
	//SetConsoleMode(input, ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);
}

//change la couleur du caractère et du fond, et change le caractère utilisé
void ConsoleApp::pencil(const fgConsoleColors& fgcolor, const bgConsoleColors& bgcolor, const char& glyph) { _pencil = { bgcolor, fgcolor, glyph }; }

//dessine une ligne verticale qui commence à 'pos', de longeur 'length'
void ConsoleApp::vertical(const V2d_i& pos, const int& length)
{
	for (int y = 0; y < length; y++)
		pix({ pos.x, pos.y + y });
}

//dessine une ligne horizontale qui commence à 'pos', de longeur 'length'
void ConsoleApp::horizontal(const V2d_i& pos, const int& length)
{
	for (int x = 0; x < length; x++)
		pix({ x + pos.x, pos.y });
}

//dessine un rectangle
void ConsoleApp::rect(const Rect& dest)
{
	for (int y = 0; y < dest.sz.y; y++)
		for (int x = 0; x < dest.sz.x; x++)
			pix({ x + dest.pos.x,y + dest.pos.y });
}

//dessine le texte donné
void ConsoleApp::text(const std::string& text, V2d_i pos)
{
	int spos = pos.x;
	for (auto& s : text)
	{
		if (s == '\n')
		{
			pos.x = spos;
			pos.y++;
			continue;
		}

		glyph(s);
		pix(pos);
		pos.x++;
	}
}

void ConsoleApp::hideCursor()
{
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(console, &info);
}

//vérifie si la position est en dedans de la fenêtre de la console
bool ConsoleApp::pos_in_screen(const V2d_i& pos)
{
	if (pos.x < 0) return false;
	if (pos.y < 0) return false;

	V2d_i consoleSize = size();

	if (pos.x >= consoleSize.x) return false;
	if (pos.y >= consoleSize.y) return false;

	return true;
}

//donne la taille de la fenêtre de la console
V2d_i ConsoleApp::size()
{
	V2d_i size;
	CONSOLE_SCREEN_BUFFER_INFO buff;
	GetConsoleScreenBufferInfo(console, &buff);
	size.x = buff.srWindow.Right - buff.srWindow.Left + 1;
	size.y = buff.srWindow.Bottom - buff.srWindow.Top + 1;

	return size;
}

//déplace les élements en mémoire vers la console
void ConsoleApp::present()
{
	/*V2d_i newSize = size() - 1;
	if (oldSize != newSize)
	{
		hideCursor();
		clearConsole();
		oldSize = newSize;
	}*/

	setCursor(0);
	for (auto& o : screenBuffer)
	{
		if (pos_in_screen(o.first))
		{
			setCursor(o.first);
			setConsoleSettings(o.second);
			std::cout << o.second.glyph;
		}
	}
	screenBuffer.clear();
}

/*
si une touche du clavier est tenu, ou 'code' est un 'Virtual-key code' de Windows
* (voir: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)
*/
bool ConsoleApp::held_code(int code)
{
	return keys.at(code);
}

/*
si une touche du clavier est tenu, selon le caractère
*(note: il est probablement préférable d'utiliser des caractères alphanumériques)
*/
bool ConsoleApp::held(char character)
{
	int i = toupper(character);
	bool b = keys.at(i) && (cooldown.at(i) == 0);
	if (cooldown.at(i) == 0)
	{
		cooldown.at(i) = base_cooldown;
	}
	return b;
}

/*
si une touche du clavier est pesé, ou 'code' est un 'Virtual-key code' de Windows
* (voir: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)
*/
bool ConsoleApp::pressed_code(int code)
{
	return (!last.at(code) && keys.at(code));
}

/*
si une touche du clavier est pesé, selon le caractère
*(note: il est probablement préférable d'utiliser des caractères alphanumériques)
*/
bool ConsoleApp::pressed(char character)
{
	character = toupper(character);
	return (!last.at((int)character) && keys.at((int)character));
}

#include <sstream>

void ConsoleApp::update()
{
	//std::cout << "update event" << _mouse << std::endl;
	/*ReadConsoleInput(input, InputRecord, 128, &Events);

	for (size_t i = 0; i < Events; i++)
	{
		switch (InputRecord[i].EventType)
		{
		case MOUSE_EVENT:
		{
			_mouse.x = InputRecord[i].Event.MouseEvent.dwMousePosition.X;
			_mouse.y = InputRecord[i].Event.MouseEvent.dwMousePosition.Y;
		}

		break;
		default:
			break;
		}
	}
	FlushConsoleInputBuffer(input);*/
}

#include <chrono>
#include <thread>

RunBool::~RunBool()
{
	//app->keyboard_update();
	app->present();


	std::chrono::milliseconds frame_end = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	);

	std::chrono::milliseconds frame_time = frame_end - app->frame_start;
	std::chrono::milliseconds frame_min{ 1000 / 60000 };

	if (frame_time < frame_min)
	{
		std::chrono::milliseconds frame_diff = frame_min - frame_time;
		std::this_thread::sleep_for(frame_diff);
	}
}
