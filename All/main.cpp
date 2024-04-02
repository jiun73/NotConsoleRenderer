#include "pch.h"
#include "NotConsoleRenderer.h"
#include "CommandStandard.h"
#include "main_chatroom.h"
#include "ChunkFile.h"

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

	NCR::File file("file.dat", NCR::Files::FILE_WRITING);
	file["string"] << "hello" << " ... hello again";
	file["test"]["sub"] << 789 << 1 << 6542 << 543534;
	file["test"]["sub2"] << 1234.534<< 564645.34 << 543543.655344;
	file["test"] << NCR::Files::next;
	file["test"]["sub"] << 543 << 654  << 865;
	file["test"]["sub2"] << 2.3 << 5.9 << 64.9;
	file["test"] << NCR::Files::next;
	file["zzz"] << "this will be the last chunk";
	file.close();

	NCR::File in("file.dat", NCR::Files::FILE_READING);
	int i = 0;
	size_t sz;
	int* list = in["test"](1)["sub"].list<int>(sz);

	for (size_t i = 0; i < sz; i++)
	{
		std::cout << list[i] << " ";
	}

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