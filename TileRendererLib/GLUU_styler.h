#pragma once

namespace GLUU
{
	class Element;
	class Widget;

	struct StylerInterface
	{
		virtual void render_base(const Element& graphic, Widget* widget) = 0;
		virtual string widget_name() = 0;
	};

	template<typename T>
	struct Styler : public StylerInterface
	{
		void render_base(const Element& graphic, Widget* widget) override;

		string widget_name() override
		{
			return T::keyword();
		}

	private:
		virtual void render(const Element& graphic, Widget* widget) = 0;
	};

}

#include "GLUU_wint.h"

namespace GLUU
{

	template<typename T>
	inline void Styler<T>::render_base(const Element& graphic, Widget* widget)
	{
		if (std::type_index(typeid(T)) == widget->type())
		{
			render(graphic, widget);
		}
	}
}