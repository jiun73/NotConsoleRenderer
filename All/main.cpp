#include "pch.h"
#include "NotConsoleRenderer.h"
#include "CommandStandard.h"
#include "main_chatroom.h"
#include "ChunkFile.h"

#include "SDL_image.h"

class ButtonWidgetStyler_Minecraft : public GLUU::Styler<GLUU::ButtonWidget>
{
	void render(GLUU::Element& graphic, GLUU::ButtonWidget& widget)
	{
		if (widget.is_hover)
		{
			if (widget.is_held)
				pencil(COLOR_PINK);
			else
				draw_image_from_source("widgets.png", Rect(0, 66 + 20, 200, 20), graphic.last_dest);
		}
		else
		{
			draw_image_from_source("widgets.png", Rect(0, 66, 200, 20), graphic.last_dest);
		}

		int w = get_text_draw_size(widget.text(), get_font(2));
		int h = get_font(2).height;
		int centerposx = graphic.last_dest.pos.x + (((int)graphic.last_dest.sz.x - w) / 2);
		int centerposy = graphic.last_dest.pos.y + (((int)graphic.last_dest.sz.y - h) / 2);

		get_font(2).set_color(Color(0, 0, 0, 255));
		draw_text(widget.text(), (int)w, { centerposx + 1, centerposy + 1 }, get_font(2));
		get_font(2).set_color(COLOR_WHITE);
		draw_text(widget.text(), (int)w, { centerposx, centerposy }, get_font(2));
		
	}

	STYLER_COPY;
};

inline GLUU::ImportStyler<ButtonWidgetStyler_Minecraft> import_button_styler("minecraft");

class ButtonWidgetStyler_GLUU : public GLUU::Styler<GLUU::ButtonWidget>
{
	char style_id = '1';

	bool set_arg(const string& arg) override
	{
		if (arg == "rounded")
		{
			style_id = '2';
			return true;
		}
		else if (arg == "key") {
			style_id = '3';
			return true;
		}
	}

	void render(GLUU::Element& graphic, GLUU::ButtonWidget& widget)
	{
		if (widget.is_hover && widget.is_held)
		{
			switch (style_id)
			{
			case '1':
				draw_image_9patch("GLUU/GUI.png", { {24,0}, 24 }, graphic.last_dest);
				break;
			case '2':
				draw_image_9patch("GLUU/GUI.png", { {0,0}, 24 }, graphic.last_dest);
				break;
			case '3':
				draw_image_9patch("GLUU/GUI.png", { {24 * 4,0}, 24 }, graphic.last_dest, 2);
				break;
			default:
				break;
			}
			
			pencil(COLOR_PINK);
		}
		else
			switch (style_id)
			{
			case '1':
				draw_image_9patch("GLUU/GUI.png", { 0, 24 }, graphic.last_dest);
				break;
			case '2':
				draw_image_9patch("GLUU/GUI.png", { {0, 24}, 24 }, graphic.last_dest);
				break;
			case '3':
				draw_image_9patch("GLUU/GUI.png", { {24 * 3,0}, 24 }, graphic.last_dest, 2);
				break;
			default:
				break;
			}
			
		if (mouse_left_pressed() && widget.is_held)
		{
			sound().playSound("Sounds/button.wav");
		}

		int w = get_text_draw_size(widget.text(), get_font(0));
		int h = get_font(0).height;
		int centerposx = graphic.last_dest.pos.x + (((int)graphic.last_dest.sz.x - w) / 2);
		int centerposy = graphic.last_dest.pos.y + (((int)graphic.last_dest.sz.y - h) / 2);

		if (style_id == '3' && !widget.is_held)
		{
			centerposy -= 3;
		}

		get_font(0).set_color(Color(0, 0, 0, 255));
		draw_text(widget.text(), (int)w, { centerposx + 1, centerposy + 1 }, get_font(0));
		get_font(0).set_color(COLOR_WHITE);
		draw_text(widget.text(), (int)w, { centerposx, centerposy }, get_font(0));

	}

	STYLER_COPY;
};

inline GLUU::ImportStyler<ButtonWidgetStyler_GLUU> import_button_grstyler("GLUU");

int main()
{
	set_window_size({ (int)(1920),(int)(1080) });
	set_window_spawn({ (int)(1920 ),(int)(1080 ) });
	set_window_logical_rescaling(true);
	set_window_resizable();
	init();

	add_regular_command_set();

	GLUU::Compiled_ptr menu = GLUU::parse_file("GLUU/main.gluu");

	set_callback([]()
		{
			if (key_pressed(SDL_SCANCODE_F1))
			{
				close();
			}
		});

	auto tex = get_sdl_texture("alexandre.png");
	size_t sz = 0;
	int w = 0;
	int h =0;
	void* data = get_texture_data(tex, sz, w, h);

	NCR::File f("data.data", NCR::Files::FILE_WRITING);
	f("test") << NCR::Files::raw((char*)data, sz);
	f.close();
	
	SDL_RWops* ops = SDL_RWFromMem(data, sz);
	IMG_LoadPNG_RW(ops);

	while (run())
	{
		set_override_run(true);
		pencil(COLOR_BLACK);
		draw_clear();
		menu->render({ 0,get_window_size() });
		set_window_size({ (int)(1920),(int)(1080) });
		set_override_run(false);
	}
}