#pragma once
#include <type_traits>
#include <iostream>
#include "SDL.h"
#include "Vec2D.h"

#ifdef max
#undef max
#endif


#ifdef min
#undef min
#endif

template<typename T>
struct Vector3D
{
	T x, y, z;

	Vector3D() : x(T(0)), y(T(0)), z(T(0)) {}
	Vector3D(const T& val) : x(val), y(val), z(val) {}
	Vector3D(const T& x, const T& y) : x(x), y(y), z(0) {}
	Vector3D(const T& x, const T& y, const T& z) : x(x), y(y), z(z) {}
	Vector3D(const Vector3D& v) : x(v.x), y(v.y), z(v.z) {}
	Vector3D(const Vector2D<T>& v) : x(v.x), y(v.y), z(0) {}

	Vector3D& operator=(const Vector3D& v) { x = v.x; y = v.y; z = v.z; return *this; }
	Vector3D& operator+=(const Vector3D& v) { x += v.x; y += v.y; z += v.z; return *this; }
	Vector3D& operator-=(const Vector3D& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	Vector3D& operator*=(const Vector3D& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
	Vector3D& operator/=(const Vector3D& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }

	Vector3D& operator=(const T& v) { x = v; y = v;	z = v; return *this; }
	Vector3D& operator+=(const T& v) { x += v; y += v; z += v; return *this; }
	Vector3D& operator-=(const T& v) { x -= v; y -= v; z -= v; return *this; }
	Vector3D& operator*=(const T& v) { x *= v; y *= v; z *= v; return *this; }
	Vector3D& operator/=(const T& v) { x /= v; y /= v; z /= v; return *this; }

	Vector3D operator-() const { return Vector3D(-x, -y, -z); }
	friend bool operator==(const Vector3D& v1, const Vector3D& v2) { return (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z); }
	friend bool operator!=(const Vector3D& v1, const Vector3D& v2) { return !(v1 == v2); }

	template<class C> operator Vector3D<C>() { return { static_cast<C>(x), static_cast<C>(y), static_cast<C>(z) }; }
	template<class C> operator Vector2D<C>() { return { static_cast<C>(x), static_cast<C>(y), 0}; }

	Vector3D xi() { return { x,T(0), T(0)}; }
	Vector3D yj() { return { T(0),y, T(0)}; }
	Vector3D zk() { return { T(0),T(0),z}; }

	Vector3D& floor() { x = std::floor(x); y = std::floor(y); z = std::floor(z); return *this; };
	Vector3D& ceil() { x = std::ceil(x); y = std::ceil(y); z = std::ceil(z);  return *this; };

	void min(const T& c) { x = (std::min)(x, c); y = (std::min)(y, c); z = (std::min)(z, c);}
	void max(const T& c) { x = (std::max)(x, c); y = (std::max)(y, c); z = (std::max)(z, c);}
	void min(const Vector3D& v) { x = (std::min)(x, v.x); y = (std::min)(y, v.y); z = (std::min)(z, v.z);}
	void max(const Vector3D& v) { x = (std::max)(x, v.x); y = (std::max)(y, v.y); z = (std::max)(z, v.z);}
	void range(const Vector3D& min, const Vector3D& max) { x = (std::min)((std::max)(min.x, x), max.x); y = (std::min)((std::max)(min.y, y), max.y); z = (std::min)((std::max)(min.z, z), max.z);}

	T		dot(const Vector3D& v) { return (x * v.x) + (y * v.y) + (z * v.z); };
	double	norm() { return sqrt(x * x + y * y + z * z); }
	Vector2D<T> flatten() { return { x,y }; }
	std::pair<double, double>	orientation() { return { asin(-y),atan2(x, z) }; }
	Vector3D& polar(double norm, double angle)
	{
		x = norm * cos(angle);
		y = norm * sin(angle);
		return *this;
	}

	void normalize() { T n = norm(); x = x / n; y = y / n; }

	friend std::ostream& operator<<(std::ostream& os, const Vector3D& v) { os << "{" << v.x << "," << v.y << "}"; return os; } //set we can output vectors
};

template<class T> Vector3D<T> operator+(const Vector3D<T>& L, const Vector3D<T>& R) { return Vector3D<T>(L) += R; }
template<class T> Vector3D<T> operator-(const Vector3D<T>& L, const Vector3D<T>& R) { return Vector3D<T>(L) -= R; }
template<class T> Vector3D<T> operator*(const Vector3D<T>& L, const Vector3D<T>& R) { return Vector3D<T>(L) *= R; }
template<class T> Vector3D<T> operator/(const Vector3D<T>& L, const Vector3D<T>& R) { return Vector3D<T>(L) /= R; }

template<class L, class R> Vector3D<L> operator+(const Vector3D<L>& l, R r) { return Vector3D<L>(l) += Vector3D<R>(r, r); }
template<class L, class R> Vector3D<L> operator-(const Vector3D<L>& l, R r) { return Vector3D<L>(l) -= Vector3D<R>(r, r); }
template<class L, class R> Vector3D<L> operator*(const Vector3D<L>& l, R r) { return Vector3D<L>(l) *= Vector3D<R>(r, r); }
template<class L, class R> Vector3D<L> operator/(const Vector3D<L>& l, R r) { return Vector3D<L>(l) /= Vector3D<R>(r, r); }

//IMPORTANT!! these operators don'T really make sense in practice: but they allow vectors to be used in maps
template<class L, class R> bool operator< (const Vector3D<L>& v1, const Vector3D<R>& v2)
{
	if (v1.y < v2.y)
		return true;
	else if (v1.y > v2.y)
		return false;
	else
		return (v1.x < v2.x);
}
template<class L, class R> bool operator> (const Vector3D<L>& v1, const Vector3D<R>& v2)
{
	if (v1.y > v2.y)
		return true;
	else if (v1.y < v2.y)
		return false;
	else
		return (v1.x > v2.x);
}

typedef Vector3D<double> V3d_d;
typedef Vector3D<float> V3d_f;
typedef Vector3D<int> V3d_i;

namespace std
{
	template<typename T> struct hash<Vector3D<T>>
	{
		std::size_t operator()(const Vector3D<T>& s) const noexcept
		{
			std::size_t h1 = std::hash<T>{}(s.x);
			std::size_t h2 = std::hash<T>{}(s.y);
			return h1 ^ (h2 << 1); // or use boost::hash_combine
		}
	};
}