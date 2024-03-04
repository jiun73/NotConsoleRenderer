#pragma once
#include "GLUU_parser.h"

#include "QuickButton.h"


namespace GLUU {
	class DropboxWidget : public Widget
	{
		QuickButton but;
		vector<QuickButton> choices;
		Rect box;
		bool open = false;
		bool lock = false;
		string text;

		InputLock ilock;

		GLUU_Make(2, "DROPBOX")
		{
			auto ptr = make_shared<DropboxWidget>();

			ptr->list.set(args.at(0), parser);
			ptr->index.set(args.at(1), parser);
			ptr->box.sz.x = 40;
			return ptr;
		}

		SeqVar<vector<string>> list;
		SeqVar<int> index;

		void update(Element& graphic) override
		{
			if (index() >= 0 && index() < list().size() && !list().empty())
				text = list().at(index());
			box.pos.x = graphic.last_dest.sz.x + graphic.last_dest.pos.x - box.sz.x;
			box.pos.y = graphic.last_dest.pos.y;
			box.sz.y = graphic.last_dest.sz.y;
			but.box = box;
			but.update();

			ilock.unlock();

			if (but.is_press_once())
			{
				choices.clear();
				int i = 0;
				for (auto& l : list())
				{
					QuickButton but2;
					but2.box.sz = graphic.last_dest.sz;
					but2.box.pos.y = (i + 1) * graphic.last_dest.sz.y + graphic.last_dest.pos.y;
					but2.box.pos.x = graphic.last_dest.pos.x;
					choices.push_back(but2);
					i++;
				}
				open = !open;
			}

			if (open)
			{
				int i = 0;
				for (auto& but2 : choices)
				{
					but2.update();
					if (but2.is_press_once())
					{
						index() = i;
						open = false;
						break;
					}
					i++;
				}

				ilock.lock();
			}


			pencil(COLOR_BLACK);
			draw_full_rect(graphic.last_dest);
			pencil(but.hover ? COLOR_GREEN : COLOR_BLACK);
			draw_full_rect(box);
			pencil(COLOR_WHITE);
			draw_rect(box);

			draw_text(text, graphic.last_dest.sz.x, graphic.last_dest.pos, get_font(0));

			if (open)
			{
				int i = 0;
				for (auto& but2 : choices)
				{
					pencil(but2.hover ? (but2.held ? COLOR_PINK : COLOR_GREEN) : COLOR_BLACK);
					draw_full_rect(but2.box);
					string text = list().at(i);
					draw_text(text, but2.box.sz.x, but2.box.pos, get_font(0));

					i++;
				}
			}
		}
	};
}
