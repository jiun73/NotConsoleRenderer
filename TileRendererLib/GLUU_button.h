#pragma once
#include "GLUU_parser.h"

namespace GLUU {
	class ButtonWidget : public Widget
	{
		GLUU_Make(2, "BUTTON")
		{
			auto ptr = make_shared<ButtonWidget>();

			ptr->text.set(args.at(0), parser);

			ExpressionParser expression_parser(&parser);
			ptr->expr = expression_parser.parse(args.at(1));
			return ptr;
		}

		SeqVar<string> text;
		Expression expr;
		bool lock = false;

		void update(Element& graphic) override
		{
			if (lock)
			{
				if(mouse_left_released()) return;
				lock = false;
				return;
			}

			if (point_in_rectangle(mouse_position(), graphic.last_dest))
			{
				if (mouse_left_held())
					pencil(COLOR_PINK);
				else
					pencil(COLOR_GREEN);
				if (mouse_left_released())
				{
					expr.evaluate();
					lock = true;
				}
			}
			else
				pencil(COLOR_BLACK);
			draw_full_rect(graphic.last_dest);
			pencil(rgb(100, 100, 100));
			draw_rect(graphic.last_dest);
			draw_text(text(), (int)graphic.last_dest.sz.x, (V2d_i)graphic.last_dest.pos, get_font(0));
		}
	};
}
