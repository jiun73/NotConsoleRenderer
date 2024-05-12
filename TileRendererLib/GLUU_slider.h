#pragma once
#include "GLUU_parser.h"

namespace GLUU {
	class ButtonWidget : public Widget
	{
		GLUU_Make(2, "slider")
		{
			auto ptr = make_shared<ButtonWidget>();

			ptr->min.set(args[0], parser);
			ptr->max.set(args[1], parser);

			return ptr;
		}

		SeqVar<int> max;
		SeqVar<int> min;

		bool lock = false;

		void update(Element& graphic) override
		{
			
		}


	};
}
