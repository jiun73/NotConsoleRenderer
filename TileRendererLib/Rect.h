#pragma once
#include "Vec2D.h"
#include "SDL.h"

//Classe rectangle

template<class T>
struct Rectangle
{
	SDL_Rect* sdl = nullptr;
	SDL_FRect* fsdl = nullptr;
	Vector2D<T> pos;
	Vector2D<T> sz;

	Rectangle() : pos(Vector2D<T>()), sz(Vector2D<T>()) {}
	Rectangle(const T px, const T py, const T sx, const T sy) : pos(px, py), sz(sx, sy) {}
	Rectangle(const T _r) : pos(_r), sz(_r) {}
	Rectangle(const Vector2D<T> _pos, const Vector2D<T> _sz) : pos(_pos), sz(_sz) {}
	Rectangle(const SDL_Rect _r) : pos(_r.x, _r.y), sz(_r.w, _r.h) {}
	Rectangle(const Rectangle& _r) : pos(_r.pos), sz(_r.sz) { sdl = nullptr; fsdl = nullptr; }
	~Rectangle() { if (sdl != nullptr)delete sdl; if (fsdl != nullptr)delete fsdl; }

	SDL_FRect* fSDL()
	{
		if (fsdl == nullptr)
			fsdl = new SDL_FRect();

		fsdl->x = static_cast<float>(pos.x);
		fsdl->y = static_cast<float>(pos.y);
		fsdl->w = static_cast<float>(sz.x);
		fsdl->h = static_cast<float>(sz.y);
		return fsdl;
	}

	SDL_Rect* SDL()
	{
		if (sdl == nullptr)
			sdl = new SDL_Rect();

		sdl->x = static_cast<int>(pos.x);
		sdl->y = static_cast<int>(pos.y);
		sdl->w = static_cast<int>(sz.x);
		sdl->h = static_cast<int>(sz.y);

		return sdl;
	}

	friend bool operator==(const Rectangle& v1, const Rectangle& v2) { return (v1.pos == v2.pos && v1.sz == v2.sz); }

	friend std::ostream& operator<<(std::ostream& os, const Rectangle& r)
	{
		os << "{[" << r.pos.x << "," << r.pos.y << "][" << r.sz.x << "," << r.sz.y << "]}";
		return os;
	}

	void Draw(SDL_Renderer* ren)
	{
		SDL_RenderFillRect(ren, SDL());
	}

	template<class T> operator Rectangle<T>() const { return Rectangle<T>(static_cast<T>(pos.x), static_cast<T>(pos.y), static_cast<T>(sz.x), static_cast<T>(sz.y)); }
};

typedef Rectangle<double> Rect_d;
typedef Rectangle<float> Rect_f;
typedef Rectangle<int> Rect;


