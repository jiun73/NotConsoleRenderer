#pragma once
#include <stdlib.h>
#include <iostream>
#include <map>
#include "Vec2D.h"
#include "Rect.h"
#include "Random.h"

enum fgConsoleColors
{
	BLACK = 0,
	DARK_BLUE = 0x0001,
	DARK_RED = 0x0004,
	DARK_GREEN = 0x0002,
	DARK_CYAN = DARK_BLUE | DARK_GREEN,
	DARK_PURPLE = DARK_BLUE | DARK_RED,
	DARK_YELLOW = DARK_GREEN | DARK_RED,
	GRAY = DARK_RED | DARK_GREEN | DARK_BLUE,
	DARK_GRAY = 0x0008,
	BLUE = DARK_BLUE | 0x0008,
	RED = DARK_RED | 0x0008,
	GREEN = DARK_GREEN | 0x0008,
	CYAN = DARK_CYAN | 0x0008,
	PURPLE = DARK_PURPLE | 0x0008,
	YELLOW = DARK_YELLOW | 0x0008,
	WHITE = GRAY | 0x0008,
};

enum bgConsoleColors
{
	BG_BLACK = 0,
	BG_DARK_BLUE = 0x0010,
	BG_DARK_RED = 0x0040,
	BG_DARK_GREEN = 0x0020,
	BG_DARK_CYAN = BG_DARK_BLUE | BG_DARK_GREEN,
	BG_DARK_PURPLE = BG_DARK_BLUE | BG_DARK_RED,
	BG_DARK_YELLOW = BG_DARK_GREEN | BG_DARK_RED,
	BG_GRAY = BG_DARK_RED | BG_DARK_GREEN | BG_DARK_BLUE,
	BG_DARK_GRAY = 0x0080,
	BG_BLUE = BG_DARK_BLUE | 0x0080,
	BG_RED = BG_DARK_RED | 0x0080,
	BG_GREEN = BG_DARK_GREEN | 0x0080,
	BG_CYAN = BG_DARK_CYAN | 0x0080,
	BG_PURPLE = BG_DARK_PURPLE | 0x0080,
	BG_YELLOW = BG_DARK_YELLOW | 0x0080,
	BG_WHITE = BG_GRAY | 0x0080,
};

#include <array>

struct ConsolePixel
{
	bgConsoleColors bg = BG_BLACK;
	fgConsoleColors fg = BLACK;
	char glyph = (char)219;
};

class ConsoleApp;

class RunBool
{
	ConsoleApp* app = nullptr;
	bool isRunning = false;

public:
	RunBool(ConsoleApp* app, bool isRunning) : app(app), isRunning(isRunning) {}
	~RunBool();

	operator bool() const { return isRunning; };
};

#include <chrono>

class ConsoleApp
{
private:;
	void* console;
	void* input;
	unsigned long Events;

	std::chrono::milliseconds frame_start;


	std::map<V2d_i, ConsolePixel> screenBuffer;

	ConsolePixel _pencil;
	V2d_i cursor = { 0,0 };
	V2d_i oldSize = 0;

	void setConsoleToCursor();
	void clearConsole() { system("cls"); }

	void setConsoleSettings(const ConsolePixel& pixel);
	
	void setDrawPencil(const ConsolePixel& pixel) { _pencil = pixel; }
	
	void hideCursor();

	std::array<bool, 255> last;
	std::array<bool, 255> keys;
	std::array<int, 255> cooldown;
	int base_cooldown = 10;

	bool isRunning = true;

	void update();
	

public:
	void setCursor(const V2d_i& pos);
	V2d_i getCursor();
	void setConsoleColor(const fgConsoleColors& fg, const bgConsoleColors& bg);

	void present();

	V2d_i _mouse;
	Random random;

	ConsoleApp();
	~ConsoleApp() {}

	void pencil(const fgConsoleColors& fgcolor, const bgConsoleColors& bgcolor, const char& glyph);
	void color(const fgConsoleColors& fg, const bgConsoleColors& bg) { setConsoleColor(fg, bg); _pencil.fg = fg; _pencil.bg = bg; }
	void glyph(const char& _glyph) { _pencil.glyph = _glyph; }

	void vertical(const V2d_i& pos, const int& length);
	void horizontal(const V2d_i& pos, const int& length);
	void rect(const Rect& dest);
	void pix(const V2d_i& pos) { if (!pos_in_screen(pos)) return; screenBuffer[pos] = _pencil; }
	void text(const std::string& text, V2d_i pos);

	void clear() { clearConsole(); screenBuffer.clear(); }

	bool pos_in_screen(const V2d_i& pos);
	V2d_i size();



	bool held_code(int code);
	bool held(char character);
	bool pressed_code(int code);
	bool pressed(char character);

	/*Rect getMaxWindow(HWND window)
	{
		RECT rectWorkArea;
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		::GetMonitorInfo(::MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &mi);
		rectWorkArea = mi.rcWork;
		return { {rectWorkArea.left, rectWorkArea.top},{rectWorkArea.bottom - rectWorkArea.top, rectWorkArea.right - rectWorkArea.left} };
	}*/

	void set_cooldown(int s)
	{
		base_cooldown = s;
	}

	/*bool mouse_left_held() { return held_code(VK_LBUTTON); }
	bool mouse_left_press() { return pressed_code(VK_LBUTTON); }
	bool mouse_click_on(V2d_i pos) { return (pressed_code(VK_LBUTTON) && _mouse == pos); }*/

	const RunBool& run()
	{
		frame_start = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		);
		return RunBool(this, isRunning);
	}
	void			close() { isRunning = false; }
	friend RunBool;

};

typedef Singleton<ConsoleApp> CApp;
