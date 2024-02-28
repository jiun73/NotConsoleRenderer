#include "NotConsoleRenderer.h"
#include "CommandStandard.h"
#include "main_chatroom.h"

int main() 
{
	set_window_size({ (int)(1920 * 0.75),(int)(1080 * 0.75) });
	set_window_resizable();
	init();

	add_regular_command_set();

	GLUU::Compiled_ptr menu = GLUU::parse_file("GLUU/main.gluu");

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
		menu->render({ 0,get_logical_size() });
	}
}