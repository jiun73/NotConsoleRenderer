#pragma once
#include "Vec2D.h"
#include "Rect.h"

struct Camera 
{
	V2d_i offset;
	
	template<typename T>
	Vector2D<T> find(const Vector2D<T>& position)
	{
		return position - (Vector2D<T>)offset;
	}

	template<typename T>
	Vector2D<T> inverse(const Vector2D<T>& position)
	{
		return position + (Vector2D<T>)offset;
	}

	template <typename T>
	Rectangle<T> find(const Rectangle<T>& rectangle)
	{
		Rectangle<T> rect = rectangle;
		rect.pos = find(rect.pos);
		return rect;
	}



};