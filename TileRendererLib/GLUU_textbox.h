#pragma once
#include "GLUU_exprParser.h"

namespace GLUU {
	class TextboxWidget : public Widget
	{
		SeqVar<string> default_text;
		SeqVar<string> text;
		Expression expr;
		bool lock = false;
		int lock2 = 0;

		GLUU_Make(3, "textbox") 
		{
			auto ptr = make_shared<TextboxWidget>();
			ptr->default_text.set(args.at(0), parser);
			ptr->text.set(args.at(1), parser);


			ExpressionParser expression_parser(&parser);
			ptr->expr = expression_parser.parse(args.at(2));

			return ptr;
		}

		void update(Element& graphic, MouseInfo& mouse) override
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

			if (lock || (!lock && text.get().empty()))
				draw_text(default_text(), (int)graphic.last_dest.sz.x, (V2d_i)graphic.last_dest.pos, get_font(0));
			else 
				draw_text(text.get(), (int)graphic.last_dest.sz.x, (V2d_i)graphic.last_dest.pos, get_font(0));

			if (!lock)
			{
				for (auto& c : keyboard().getTextInput())
				{
					text.get().push_back(c);
				}
				keyboard().getTextInput().clear();

				if (lock2 && !key_pressed(SDL_SCANCODE_BACKSPACE))
				{
					lock2 = 0;
				}
				else if (lock2 && key_pressed(SDL_SCANCODE_BACKSPACE))
				{
					lock2++;

					if (lock2 > 100)
					{
						text.get().pop_back();
						lock2 = 97;
					}
				}

				if (key_pressed(SDL_SCANCODE_BACKSPACE) && !text.get().empty() && !lock2)
				{
					text.get().pop_back();
					lock2 = 1;
				}

				if (!text.get().empty())
				{
					if (text.get().back() == '\n') {
						text.get().pop_back();
						expr.evaluate();
					}
				}
			}
		}
	};

	class ButtonTextboxStyler : public Styler<TextboxWidget>
	{
		void render(Element& graphic, TextboxWidget& widget) override {}
		STYLER_COPY;
	};

	inline ImportStyler<ButtonTextboxStyler> import_textbox_styler("default");
}
