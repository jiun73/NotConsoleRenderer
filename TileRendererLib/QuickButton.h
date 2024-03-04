#pragma once
#include "Rect.h"
#include "Controls.h"
#include "NotConsoleRenderer.h"

struct QuickButton
{
	Rect box = 0;
	bool hover = false;
	bool held = false;
	bool not_held = false;
	bool lock = false;
	bool lock2 = false;
	bool focus = false;

	//works only once, then activates a lock
	bool is_press_once()
	{
		/*if (held && !lock)
		{
			lock = true;
			return true;
		}
		return false;*/

		return mouse_left_released() && hover;
	}

	//works only once, then activates a lock
	bool is_not_press_once()
	{
		if (not_held && !lock2)
		{
			lock2 = true;
			return true;
		}
		return false;
	}

	void update()
	{
		V2d_i mpos = mouse_position();
		hover = point_in_rectangle(mpos, box);
		held = mouse_left_held() && hover;
		not_held = mouse_left_held() && !hover;

		if (!held && lock && !mouse_left_held())
		{
			lock = false;
		}

		if (!not_held && lock2 && !mouse_left_held())
		{
			lock2 = false;
		}

		if (held)
		{
			focus = true;
		}

		if (not_held)
		{
			focus = false;
		}
	}
};