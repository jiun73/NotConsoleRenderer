#pragma once
#include "GLUU_import.h"

namespace GLUU {
	struct ButtonWidget : public Widget
	{
		GLUU_Make(2, "button")
		{
			auto ptr = make_shared<ButtonWidget>();

			ptr->text.set(args.at(0), parser);

			ExpressionParser expression_parser(&parser);
			ptr->expr = expression_parser.parse(args.at(1));
			return ptr;
		}

	public:

		SeqVar<string> text;
		Expression expr;
		bool lock = false;

		bool is_hover = false;
		bool is_held = false;

		void update(Element& graphic) override
		{
			if (lock)
			{
				if(mouse_left_released()) return;
				lock = false;
				return;
			}

			is_hover = point_in_rectangle(mouse_position(), graphic.last_dest);

			if (is_hover)
			{
				is_held = mouse_left_held();
				if (mouse_left_released())
				{
					expr.evaluate();
					lock = true;
				}
			}
			else
			{
				is_held = false;
				pencil(COLOR_BLACK);
			}
			
		}
	};

	class ButtonWidgetStyler : public Styler<ButtonWidget>
	{
		void render(Element& graphic, ButtonWidget& widget)
		{
			if (widget.is_hover)
			{
				if (widget.is_held)
					pencil(COLOR_PINK);
				else
					pencil(COLOR_GREEN);
			}
			else
				pencil(COLOR_BLACK);

			draw_full_rect(graphic.last_dest);
			pencil(rgb(100, 100, 100));
			draw_rect(graphic.last_dest);
			draw_text(widget.text(), (int)graphic.last_dest.sz.x, (V2d_i)(graphic.last_dest.pos), get_font(0));
		}
	};

	inline ImportWidget<ButtonWidget> import_button;
	inline ImportStyler<ButtonWidgetStyler> import_button_styler("default");
}
