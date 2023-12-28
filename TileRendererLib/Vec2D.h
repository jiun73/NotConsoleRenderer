#pragma once
#include <type_traits>
#include <iostream>
#include "SDL.h"

#ifdef max
#undef max
#endif


#ifdef min
#undef min
#endif

//Vecteur 2D comme tu la deja utilisé probablement

template<typename T>
struct Vector2D
{
	T x, y;

	Vector2D() : x(T(0)), y(T(0)) {}
	Vector2D(const T& val) : x(val), y(val) {}
	Vector2D(const T& x, const T& y) : x(x), y(y) {}
	Vector2D(const Vector2D& v) : x(v.x), y(v.y) {}

	Vector2D& operator=(const Vector2D& v) { x = v.x; y = v.y; return *this; }
	Vector2D& operator+=(const Vector2D& v) { x += v.x; y += v.y; return *this; }
	Vector2D& operator-=(const Vector2D& v) { x -= v.x; y -= v.y; return *this; }
	Vector2D& operator*=(const Vector2D& v) { x *= v.x; y *= v.y; return *this; }
	Vector2D& operator/=(const Vector2D& v) { x /= v.x; y /= v.y; return *this; }

	Vector2D& operator=(const T& v) { x = v; y = v;		return *this; }
	Vector2D& operator+=(const T& v) { x += v; y += v; return *this; }
	Vector2D& operator-=(const T& v) { x -= v; y -= v; return *this; }
	Vector2D& operator*=(const T& v) { x *= v; y *= v; return *this; }
	Vector2D& operator/=(const T& v) { x /= v; y /= v; return *this; }

	Vector2D operator-() const { return Vector2D(-x, -y); }
	friend bool operator==(const Vector2D& v1, const Vector2D& v2) { return (v1.x == v2.x && v1.y == v2.y); }
	friend bool operator!=(const Vector2D& v1, const Vector2D& v2) { return !(v1 == v2); }

	template<class C> operator Vector2D<C>() { return { static_cast<C>(x), static_cast<C>(y) }; }

	Vector2D xi() { return { x,T(0) }; }
	Vector2D yj() { return { T(0),y }; }

	Vector2D& floor() { x = std::floor(x); y = std::floor(y); return *this; };
	Vector2D& ceil() { x = std::ceil(x); y = std::ceil(y);  return *this; };

	void min(const T& c) { x = (std::min)(x, c); y = (std::min)(y, c); }
	void max(const T& c) { x = (std::max)(x, c); y = (std::max)(y, c); }
	void min(const Vector2D& v) { x = (std::min)(x, v.x); y = (std::min)(y, v.y); }
	void max(const Vector2D& v) { x = (std::max)(x, v.x); y = (std::max)(y, v.y); }
	void range(const Vector2D& min, const Vector2D& max) { x = (std::min)((std::max)(min.x, x), max.x); y = (std::min)((std::max)(min.y, y), max.y); }

	SDL_Point SDL() { return { x, y }; }

	T		dot(const Vector2D& v) { return (x * v.x) + (y * v.y); };
	double	norm() { return sqrt(x * x + y * y); }
	double	orientation() const { return atan2(y, x); }
	Vector2D& polar(double norm, double angle)
	{
		x = norm * cos(angle);
		y = norm * sin(angle);
		return *this;
	}

	void normalize() { T n = norm(); x = x / n; y = y / n; }

	friend std::ostream& operator<<(std::ostream& os, const Vector2D& v) { os << "{" << v.x << "," << v.y << "}"; return os; } //set we can output vectors
};

template<class T> Vector2D<T> operator+(const Vector2D<T>& L, const Vector2D<T>& R) { return Vector2D<T>(L) += R; }
template<class T> Vector2D<T> operator-(const Vector2D<T>& L, const Vector2D<T>& R) { return Vector2D<T>(L) -= R; }
template<class T> Vector2D<T> operator*(const Vector2D<T>& L, const Vector2D<T>& R) { return Vector2D<T>(L) *= R; }
template<class T> Vector2D<T> operator/(const Vector2D<T>& L, const Vector2D<T>& R) { return Vector2D<T>(L) /= R; }

template<class L, class R> Vector2D<L> operator+(const Vector2D<L>& l, R r) { return Vector2D<L>(l) += Vector2D<R>(r, r); }
template<class L, class R> Vector2D<L> operator-(const Vector2D<L>& l, R r) { return Vector2D<L>(l) -= Vector2D<R>(r, r); }
template<class L, class R> Vector2D<L> operator*(const Vector2D<L>& l, R r) { return Vector2D<L>(l) *= Vector2D<R>(r, r); }
template<class L, class R> Vector2D<L> operator/(const Vector2D<L>& l, R r) { return Vector2D<L>(l) /= Vector2D<R>(r, r); }

//IMPORTANT!! these operators don'T really make sense in practice: but they allow vectors to be used in maps
template<class L, class R> bool operator< (const Vector2D<L>& v1, const Vector2D<R>& v2)
{
	if (v1.y < v2.y)
		return true;
	else if (v1.y > v2.y)
		return false;
	else
		return (v1.x < v2.x);
}
template<class L, class R> bool operator> (const Vector2D<L>& v1, const Vector2D<R>& v2)
{
	if (v1.y > v2.y)
		return true;
	else if (v1.y < v2.y)
		return false;
	else
		return (v1.x > v2.x);
}

typedef Vector2D<double> V2d_d;
typedef Vector2D<float> V2d_f;
typedef Vector2D<int> V2d_i;
typedef Vector2D<int> Coords;
typedef Vector2D<int> v2di;

namespace std
{
	template<typename T> struct hash<Vector2D<T>>
	{
		std::size_t operator()(const Vector2D<T>& s) const noexcept
		{
			std::size_t h1 = std::hash<T>{}(s.x);
			std::size_t h2 = std::hash<T>{}(s.y);
			return h1 ^ (h2 << 1); // or use boost::hash_combine
		}
	};
}