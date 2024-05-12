#pragma once
#include <memory>

namespace GLUU
{
	class Element;
	class Widget;

	struct StylerInterface
	{
		virtual bool set_arg(const string& arg) { return false; };
		virtual void render_base( Element& graphic, Widget* widget) = 0;
		virtual string widget_name() = 0;
		virtual shared_ptr<StylerInterface> copy() = 0;
	};

	template<typename T>
	struct Styler : public StylerInterface
	{
		static_assert(std::is_base_of_v<Widget, T>);

		void render_base( Element& graphic, Widget* widget) override;

		string widget_name() override
		{
			return T::keyword();
		}

		virtual void render( Element& graphic, T& widget) = 0;
	};

}

#define STYLER_COPY shared_ptr<StylerInterface> copy() override { return std::make_shared<typename std::remove_reference_t<decltype(*this)>>(*this);}

#include "GLUU_wint.h"

namespace GLUU
{
	template<typename T>
	inline void Styler<T>::render_base( Element& graphic, Widget* widget)
	{
		if (std::type_index(typeid(T)) == widget->type())
		{
			render(graphic, *(T*)widget);
		}
	}
}