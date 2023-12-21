#pragma once
#include "SDL.h"
#include <iostream>

enum Colors
{
	COLOR_WHITE,
	COLOR_PINK,
	COLOR_BLACK,
	COLOR_AP22BLUE,
	COLOR_CYAN,
	COLOR_GREEN
};

struct Color
{
	uint8_t r, g, b, a = 0;

	Color() : r(255), b(255), g(255), a(255) {}
	~Color() {}
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
	Color(Colors c) : r(255), b(255), g(255), a(255)
	{
		switch (c)
		{
		case COLOR_PINK:
			r = 255;
			g = 0;
			b = 100;
			a = 255;
			break;
		case COLOR_CYAN:
			r = 100;
			g = 100;
			b = 255;
			a = 255;
			break;
		case COLOR_WHITE:
			r = 255;
			g = 255;
			b = 255;
			a = 255;
			break;
		case COLOR_AP22BLUE:
			r = 15;
			g = 10;
			b = 120;
			a = 255;
			break;
		case COLOR_GREEN:
			r = 0;
			g = 255;
			b = 45;
			a = 255;
			break;
		case COLOR_BLACK:
			r = 0; g = 0; b = 0; a = 0;
			break;
		}
	}

	SDL_Color SDL() {
		SDL_Color color = { r, g, b, a };
		return color;
	}


	bool operator==(Color col)
	{
		return (
			col.r == r &&
			col.g == g &&
			col.b == b &&
			col.a == a);
	}

	bool operator!=(Color col)
	{
		return !(*this == col);
	}


	friend std::ostream& operator<<(std::ostream& os, const Color& c)
	{
		os << "[" << (int)c.r << "," << (int)c.g << "," << (int)c.b << "," << (int)c.a << "]";
		return os;
	}
};
