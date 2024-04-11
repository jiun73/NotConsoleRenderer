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

		
		draw_text(widget.text(), (int)graphic.last_dest.sz.x, (V2d_i)(graphic.last_dest.pos), get_font(0));
	}
};

inline GLUU::ImportStyler<ButtonWidgetStyler_Minecraft> import_button_styler("minecraft");

int main()
{
	set_window_size({ (int)(1920 * 0.75),(int)(1080 * 0.75) });
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
		menu->render({ 0,get_logical_size() });
		set_window_size({ (int)(1920 * 0.75),(int)(1080 * 0.75) });
		set_override_run(false);
	}
}