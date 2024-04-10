#pragma once
#include "GLUU_parser.h"
#include "QuickButton.h"

namespace GLUU {
	struct CheckboxWidget : public Widget
	{
		QuickButton but;
		V2d_i text_pos;
		V2d_i text_size;
		V2d_i offset;

		SeqVar < string> text;
		SeqVar <bool> checked;

		GLUU_Make(2, "checkbox")
		{
			auto ptr = make_shared<CheckboxWidget>();

			ptr->checked.set(args.at(0), parser);
			ptr->text.set(args.at(1), parser);
			ptr->but.box.sz = 40;
			return ptr;
		}

	public:
		void update(Element& graphic) override
		{
			
			Rect dest = graphic.last_dest;
			but.box.sz = (V2d_d)dest.sz * 0.75;

			if (but.box.sz.y < but.box.sz.x)
				but.box.sz.x = but.box.sz.y;
			else
				but.box.sz.y = but.box.sz.x;

			offset = ((dest.sz.y - but.box.sz.y) / 2);
			but.box.pos = dest.pos + offset;
			text_pos = dest.pos;
			text_size = dest.sz;

			text_pos.x += but.box.sz.x + offset.x * 2;

			but.update();

			if (but.is_press_once())
			{
				checked = !checked;
			}
		}
	};

	class CheckboxWidgetStyler : public Styler<CheckboxWidget>
	{
		void render(Element& graphic, CheckboxWidget& widget)
		{
			pencil(COLOR_BLACK);
			draw_full_rect(widget.but.box);
			Rect box2 = { widget.but.box.pos + 1, widget.but.box.sz - 2 };
			pencil(widget.checked ? COLOR_GREEN : COLOR_BLACK);
			draw_full_rect(box2);
			draw_text(widget.text, widget.text_size.x, widget.text_pos, get_font(0));
		}
	};

	inline ImportWidget<ButtonWidget> import_button;
	inline ImportStyler<CheckboxWidgetStyler> import_button_styler("default");
}
