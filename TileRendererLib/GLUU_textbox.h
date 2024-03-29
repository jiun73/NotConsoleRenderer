#pragma once
#include "GLUU_parser.h"

namespace GLUU {
	class TextboxWidget : public Widget
	{
		SeqVar<string> default_text;
		Expression expr;

		GLUU_Make(2, "TEXTBOX") 
		{
			auto ptr = make_shared<TextboxWidget>();
			ptr->default_text.set(args.at(0), parser);

			string copy = args.at(1);
			ptr->expr = parser.parse_sequence(copy);

			return ptr;
		}

		void update(Element& graphic) override
		{
			if (point_in_rectangle(mouse_position(), graphic.last_dest))
			{
				if (mouse_left_released())
					keyboard().openTextInput();

				if (!keyboard().getTextInput().empty() && keyboard().getTextInput().back() == '\n')
				{
					keyboard().getTextInput().pop_back();
					vector<shared_generic> ref = { make_generic<string*>(&keyboard().getTextInput()) };
					expr.set_args(ref);
					expr.evaluate();
				}
			}
			else
			{
				if (mouse_left_pressed())
					keyboard().closeTextInput();
			}

			if (keyboard().getTextInput().empty())
				draw_text(default_text(), graphic.last_dest.sz.x, graphic.last_dest.pos, get_font(0));
			else
				draw_text(keyboard().getTextInput(), graphic.last_dest.sz.x, graphic.last_dest.pos, get_font(0));
		}
	};
}
