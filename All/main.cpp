#include "NotConsoleRenderer.h"
#include "CommandStandard.h"

int main() 
{
	set_window_size({ (int)(1920 * 0.75),(int)(1080 * 0.75) });
	set_window_resizable();
	init();

	add_regular_command_set();

	Server server;

	GLUU::import_function<void()>(":chat_host", [&]()
		{
			server.open_session([](Server& server)
				{

				});

			p2p().join();
		});

	GLUU::import_function<void()>(":chat_wait", [&]()
		{
			server.wait_for_peer();
		});

	GLUU::import_function<bool(string)>(":chat_join", [&](string s)
		{
			return p2p().join(s);
		});

	GLUU::import_function<int()>(":peercnt", [&]()
		{
			return server.peer_count();
		});

	File file("GLUU/chat_menu.gluu", FILE_READING_STRING);

	GLUU::Compiled_ptr menu = GLUU::parse_copy(file.getString());

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
		menu->render({ 0,get_logical_size() });
	}
}