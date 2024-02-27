#include "NotConsoleRenderer.h"
#include "CommandStandard.h"

int main() 
{
	set_window_size({ 1920,1080 });
	set_window_resizable();
	init();

	add_regular_command_set();

	File file("chat_menu.gluu", FILE_READING_STRING);
	GLUU::Compiled_ptr menu = GLUU::parse_copy(file.getString());

	while (run())
	{
		//menu->render();
	}
}