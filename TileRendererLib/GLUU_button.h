#pragma once
#include "GLUU_parser.h"

namespace GLUU {
	class ButtonWidget : public Widget
	{
		GLUU_Make(2, "BUTTON")
		{
			auto ptr = make_shared<ButtonWidget>();

			ptr->text.set(args.at(0), parser);
			ptr->expr = parser.parse_sequence_next(args.at(1));
			return ptr;
		}

		SeqVar<string> text;
		Expression expr;

		void update(Element& graphic) override
		{
			if (point_in_rectangle(mouse_position(), graphic.last_dest))
			{
				if (mouse_left_held())
					pencil(COLOR_PINK);
				else
					pencil(COLOR_GREEN);
				if (mouse_left_released())
					expr.evaluate();
			}
			else
				pencil(COLOR_BLACK);
			draw_full_rect(graphic.last_dest);
			pencil(rgb(100, 100, 100));
			draw_rect(graphic.last_dest);
			draw_text(text(), graphic.last_dest.sz.x, graphic.last_dest.pos, get_font(0));
		}
	};
}
