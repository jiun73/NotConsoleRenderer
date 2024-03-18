#pragma once
#include "Rect.h"
#include "NotConsoleRenderer.h"

struct EditorPlane 
{
	Rect dest;
	V2d_d offset = 0;
	double scale = 1;

	V2d_i deproject(const V2d_i& pos)
	{
		return pos - dest.pos - (V2d_i)offset;
	}

	Rect project(const Rect& r) 
	{
		Rect rect = { r.pos + (V2d_i)offset + dest.pos, {(int)((double)r.sz.x * scale), (int)((double)r.sz.y * scale)} };
		return rect;
	}

	bool is_pressed(bool left = true)
	{
		if (point_in_rectangle(mouse_position(), dest))
		{
			if ((left && mouse_left_pressed()) || (!left && mouse_right_pressed()))
			{
				return true;
			}
		}
		return false;
	}

	void draw_ck_background_square(double x, double y, double square_sz = 50)
	{
		int sq_x = std::floor((x - (double)dest.pos.x - offset.x + 0.01) / (square_sz * scale));
		int sq_y = std::floor((y - (double)dest.pos.y - offset.y + 0.01) / (square_sz * scale));

		Color col;

		if ((sq_y % 2 && sq_x  % 2) || (!(sq_y  % 2) && !(sq_x % 2)))
			col = rgb(120, 120, 120);

		if ((sq_y % 2 && !(sq_x  % 2)) || (!(sq_y  % 2) && sq_x  % 2))
			col = rgb(80,80,80);

		//col = rgb(sq_x * 10, sq_y * 10, 0);

		int xmin = std::max((int)x, dest.pos.x);
		int ymin = std::max((int)y, dest.pos.y);
		int xmax = std::min((int)(((sq_x + 1) * square_sz * scale) + offset.x + dest.pos.x - 0.01), dest.pos.x + dest.sz.x);
		int ymax = std::min((int)(((sq_y + 1) * square_sz * scale) + offset.y + dest.pos.y - 0.01), dest.pos.y + dest.sz.y);

		Rect r;
		r.pos = { xmin,ymin };
		r.sz = { xmax - xmin + 1, ymax - ymin + 1};

		//Rect r;
		//r.pos = { (int)x, (int)y };
		//r.sz = square_sz * scale;

		pencil(col);
		draw_full_rect(r);
	}

	void draw_ck_background_line(double y, double square_sz = 50)
	{
		for (double x = dest.pos.x + offset.x - square_sz * scale; x < dest.pos.x + dest.sz.x; x += square_sz * scale)
			if (x > dest.pos.x - square_sz * scale)
				draw_ck_background_square(x, y, square_sz);

		for (double x = dest.pos.x + offset.x - square_sz * scale; x > dest.pos.x - square_sz * scale; x -= square_sz * scale)
			if (x < dest.pos.x + dest.sz.x - square_sz * scale)
				draw_ck_background_square(x, y, square_sz);
	}

	void draw_checkered_background(double square_sz = 50)
	{
		if (scale <= 0.01) return;

		for (double y = dest.pos.y + offset.y - square_sz * scale; y < dest.pos.y + dest.sz.y; y += square_sz * scale)
			if (y > dest.pos.y - square_sz * scale)
				draw_ck_background_line(y, square_sz);

		for (double y = dest.pos.y + offset.y; y > dest.pos.y - square_sz * scale; y -= square_sz * scale)
			if (y < dest.pos.y + dest.sz.y - square_sz * scale)
				draw_ck_background_line(y, square_sz);
	}

	void draw_background_grid()
	{
		if (scale <= 0.01) return;

		for (double y = dest.pos.y + offset.y; y < dest.pos.y + dest.sz.y; y += 50.0 * scale)
		{
			if (y > dest.pos.y)
			{
				V2d_i linest = { dest.pos.x, (int)y };
				V2d_i linend = { dest.pos.x + dest.sz.x, (int)y };
				draw_line(linest, linend);
			}
		}

		for (double y = dest.pos.y + offset.y; y > dest.pos.y; y -= 50.0 * scale)
		{
			if (y < dest.pos.y + dest.sz.y)
			{
				V2d_i linest = { dest.pos.x, (int)y };
				V2d_i linend = { dest.pos.x + dest.sz.x, (int)y };
				draw_line(linest, linend);
			}
		} 

		for (double x = dest.pos.x + offset.x; x < dest.pos.x + dest.sz.x; x += 50.0 * scale)
		{
			if (x > dest.pos.x)
			{
				V2d_i linest = { (int)x, dest.pos.y };
				V2d_i linend = { (int)x, dest.pos.y + dest.sz.y };
				draw_line(linest, linend);
			}
		}

		for (double x = dest.pos.x + offset.x; x > dest.pos.x; x -= 50.0 * scale)
		{
			if (x < dest.pos.x + dest.sz.x)
			{
				V2d_i linest = { (int)x, dest.pos.y };
				V2d_i linend = { (int)x, dest.pos.y + dest.sz.y };
				draw_line(linest, linend);
			}
		}
	}

	void update() 
	{
	
		if (mouse().scroll())
		{
			V2d_i pos = mouse_position();
			V2d_i pos_f = pos - dest.pos - (V2d_i)offset;
			V2d_d corner = (V2d_d)dest.pos - ((V2d_d)pos_f / scale);
			V2d_d corner_sc1 = corner * scale;

			if (scale + (0.1 *  mouse().scroll()) <= 0.01) return;
			
			scale += 0.1 * mouse().scroll();

			
			
			V2d_d corner_sc2 = corner * scale;
			offset -= corner_sc1 - corner_sc2;
			std::cout << pos_f << offset << std::endl;
		}

		if (key_held(SDL_SCANCODE_LEFT))
		{
			offset.x += 1.0 / scale;
		}
		if (key_held(SDL_SCANCODE_RIGHT))
		{
			offset.x -= 1.0 / scale;
		}
		if (key_held(SDL_SCANCODE_UP))
		{
			offset.y += 1.0 / scale;
		}
		if (key_held(SDL_SCANCODE_DOWN))
		{
			offset.y -= 1.0 / scale;
		}
		

	}
};