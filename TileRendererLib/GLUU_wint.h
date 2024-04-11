#pragma once
#include <string>
#include <vector>
#include <memory>

namespace GLUU {
	class StylerInterface;

	using std::string;
	using std::pair;
	using std::vector;
	using std::shared_ptr;

	struct Element;
	class Parser;

	struct MouseInfo 
	{
		V2d_i pos;
		bool click;
		bool lock = false;

		bool is_over(Rect mask)
		{
			return point_in_rectangle(pos, mask) && !lock;
		}

		bool is_click()
		{
			return click && !lock;
		}

		void mask(Rect mask)
		{
			if (point_in_rectangle(pos, mask))
			{
				click = false;
				lock = true;
			}
		}
	};

	struct Widget
	{
		shared_ptr<StylerInterface> styler = nullptr;

		virtual pair<size_t, string> fetch_keyword() = 0;
		virtual shared_ptr<Widget> make(vector<string_ranges>& args, Parser& parser) = 0;

		void render(Element& graphic) 
		{
			if (styler != nullptr)
			{
				styler->render_base(graphic, this);
			}
		}

		virtual void update(Element& graphic, MouseInfo& mouse_inf) = 0;
		virtual void update_l2(Element& graphic) {}

		virtual std::type_index type() = 0;
	};
}
