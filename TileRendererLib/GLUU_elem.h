#pragma once
#include "GLUU_seqvar.h"
#include "GLUU_styler.h"

#include "NotConsoleRenderer.h"

namespace GLUU {
	struct Element
	{
		vector<Element> nested;
		shared_ptr<VariableRegistry> scope;
		SeqVar<size_t> size = 10;
		SeqVar<bool> condition = true;
		SeqVar<bool> popup = false;
		bool focus = false;
		SeqVar<Rect> destination = Rect(0, 0);
		SeqVar<bool> fit = true;
		Rect_d last_dest;
		

		bool is_row = false;

		int get_size_max()
		{
			int i = 0;
			for (auto& n : nested)
			{
				if (n.condition())
					i += (int)n.size();
			}

			return i;
		}

		double map_size(double sz, double max)
		{
			return sz * (size() / max);
		}

		double get_size(const Rect_d& dest, Element& other, bool vert)
		{
			if (!other.condition()) return 0;
			if (other.popup()) return 0;

			if (fit)
			{
				if (!vert)
					return other.map_size(dest.sz.x, get_size_max());
				else
					return other.map_size(dest.sz.y, get_size_max());
			}
			else
			{
				return (double)other.size();
			}

		}

		void set_size(Rect_d dest)
		{
			double pencil = is_row ? dest.pos.x : dest.pos.y;
			for (auto& row_col : nested)
			{
				double sz = get_size(dest, row_col, !is_row);

				Rect_d sub = { is_row ? V2d_d(pencil, dest.pos.y) : V2d_d(dest.pos.x, pencil),
								is_row ? V2d_d(sz, dest.sz.y) : V2d_d(dest.sz.x, sz) };
				draw_rect(sub);
				row_col.set(sub);
				pencil += sz;
			}
		}

		bool set_popup(Rect_d dest, V2d_i mouse, bool& mouse_click)
		{
			if (!condition()) return false;
			if (!popup()) return false;

			bool ret = false;

			if (mouse_click && point_in_rectangle(mouse, destination()))
			{
				focus = true;
				ret = true;
			}

			set(destination(), true);
			return ret;
		}

		void set(Rect_d dest, bool popup_mode = false)
		{
			if (!condition()) return;
			if (popup() && !popup_mode) return;
			set_size(dest);
			last_dest = dest;
		}

		void get_popups(vector<Element*>& list)
		{
			if (popup()) list.push_back(this);
			for (auto& n : nested)
			{
				n.get_popups(list);
			}
		}

		shared_ptr < Widget> widget = nullptr;

		void render(bool popup_mode = false)
		{
			if (!condition()) return;
			if (popup() && !popup_mode) return;
			if (widget != nullptr)
				widget->render(*this);

			for (auto& n : nested)
			{
				n.render();
			}
		}

		void update(MouseInfo& mouse, bool popup_mode = false)
		{
			if (!condition()) return;
			if (popup() && !popup_mode) return;

			for (auto& n : nested)
			{
				n.update(mouse);
			}

			if (widget != nullptr)
				widget->update(*this, mouse);
		}

		void update_l2(bool popup_mode = false)
		{
			if (!condition()) return;
			if (popup() && !popup_mode) return;

			for (auto& n : nested)
			{
				n.update_l2();
			}

			if (widget != nullptr)
				widget->update_l2(*this);
		}
	};
}