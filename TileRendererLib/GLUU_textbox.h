#pragma once
#include "GLUU_parser.h"

namespace GLUU {
	class TextboxWidget : public Widget
	{
		SeqVar<string> default_text;
		Expression expr;
		bool lock = false;

		GLUU_Make(2, "TEXTBOX") 
		{
			auto ptr = make_shared<TextboxWidget>();
			ptr->default_text.set(args.at(0), parser);
			ptr->expr = parser.parse_sequence_base(args.at(1));

			return ptr;
		}

		void update(Element& graphic) override
		{
			if (point_in_rectangle(mouse_position(), graphic.last_dest))
			{
				if (mouse_left_released())
				{
					lock = false;
					keyboard().openTextInput();
				}
			}
			else
			{
				if (mouse_left_pressed())
				{
					lock = true;
					keyboard().closeTextInput();
				}
			}

			if (keyboard().getTextInput().empty())
				draw_text(default_text(), graphic.last_dest.sz.x, graphic.last_dest.pos, get_font(0));
			else
				draw_text(keyboard().getTextInput(), graphic.last_dest.sz.x, graphic.last_dest.pos, get_font(0));

			if (!lock)
			{
				if (!keyboard().getTextInput().empty() && keyboard().pressed(SDL_SCANCODE_RETURN))
				{
					keyboard().getTextInput().pop_back();
					vector<shared_generic> ref = { make_generic<string*>(&keyboard().getTextInput()) };
					expr.set_args(ref);
					expr.evaluate();
				}
			}
		}
	};
}
