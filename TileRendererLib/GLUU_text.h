#pragma once
#include "GLUU_parser.h"

namespace GLUU {
	class TextWidget : public Widget
	{
		GLUU_Make(1, "TEXT")
		{
			auto ptr = make_shared<TextWidget>();
			ptr->text.set(args.at(0), parser);
			return ptr;
		}

		SeqVar<string> text;

		void update(Element& graphic) override
		{
			draw_special_text(text(), (int)graphic.last_dest.sz.x, (V2d_i)graphic.last_dest.pos, get_font(0));
		}
	};
}