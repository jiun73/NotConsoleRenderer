#include "NotConsoleRenderer.h"
#include "CommandStandard.h"

int main() 
{
	set_window_size({ (int)(1920 * 0.75),(int)(1080 * 0.75) });
	set_window_resizable();
	init();

	add_regular_command_set();

	File file("GLUU/chat_menu.gluu", FILE_READING_STRING);
	GLUU::Compiled_ptr menu = GLUU::parse_copy(file.getString());

	Server server;

	GLUU::import_function<void()>(":net_server_host", [&]()
		{
			server.open_session([](Server& server)
				{

				});

			p2p().join();
		});

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
		menu->render({ 0,get_logical_size() });
	}
}