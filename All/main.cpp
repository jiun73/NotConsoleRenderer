#include "NotConsoleRenderer.h"
#include "CommandStandard.h"

int main() 
{
	set_window_size({ (int)(1920 * 0.75),(int)(1080 * 0.75) });
	set_window_resizable();
	init();

	add_regular_command_set();

	Server server;
	std::stringstream ss;

	ss << "Start!" << std::endl;

	GLUU::import_function<void()>(":chat_host", [&]()
		{
			server.open_session([](Server& server)
				{
					auto& buffers = server.get_buffers();

					map<size_t, string> usernames;

					for (auto& b : buffers)
					{
						ReadBuffer& rb = b.second;
						if (rb.has(0))
						{
							string s;
							rb.rdc(0, s);
							std::cout << s << std::endl;

							server.broadcast(1).wrtc(s) << net::send;
						}
					}
				});

			p2p().join();
		});

	GLUU::import_function<void()>(":chat_wait", [&]()
		{
			server.wait_for_peer();
		});

	GLUU::import_function<bool(string&)>(":chat_join", [&](string& s)
		{
			bool success = p2p().join(s);
			if (success)
			{

			}
			return success;
		});

	GLUU::import_function<void()>(":chat_disconnect", [&]()
		{

		});

	GLUU::import_function<string()>(":chat_msg", [&]()
		{
			return ss.str();
		});

	GLUU::import_function<int()>(":peercnt", [&]()
		{
			return server.peer_count();
		});

	GLUU::import_function<void(string&)>(":chat_send", [&](string& s)
		{
			p2p(0).wrtc(s) << net::send;
		});

	File file("GLUU/chat_menu.gluu", FILE_READING_STRING);

	GLUU::Compiled_ptr menu = GLUU::parse_copy(file.getString());

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
		menu->render({ 0,get_logical_size() });

		if (p2p().has_stream(1))
		{
			string s;
			p2p(1).rdc(s);

			ss << s << std::endl;
		}
	}
}