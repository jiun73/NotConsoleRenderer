#include "NotConsoleRenderer.h"
#include "CommandStandard.h"

enum MessageTypes
{
	MESSAGE_IN,
	MESSAGE_OUT,
	SET_NAME, //string
	NAME_UPDATE
};

int main() 
{
	set_window_size({ (int)(1920 * 0.75),(int)(1080 * 0.75) });
	set_window_resizable();
	init();

	add_regular_command_set();

	Server server;
	std::stringstream ss;

	ss << "Start!" << std::endl;

	map<size_t, string> usernames;

	GLUU::import_function<void()>(":chat_host", [&]()
		{
			server.open_session([](Server& server)
				{
					auto& buffers = server.get_buffers();

					map<size_t, string> usernames;

					for (auto& b : buffers)
					{
						ReadBuffer& rb = b.second;
						if (rb.has(MESSAGE_IN))
						{
							string s;
							rb.rdc(MESSAGE_IN, s);
							std::cout << s << std::endl;

							server.broadcast(MESSAGE_OUT).wrtc(s) << net::send;
						}

						if (rb.has(SET_NAME))
						{
							string username;
							rb.rdc(SET_NAME, username);
							std::cout << "Added user " << username;
							usernames.emplace(b.first, username);

							server.broadcast(NAME_UPDATE) << usernames.size();

							for (auto& name : usernames)
							{
								server.broadcast(NAME_UPDATE) << name.first;
								server.broadcast(NAME_UPDATE).wrtc(name.second);
							}

							server.broadcast(NAME_UPDATE) << net::send;
						}
					}
				});

			p2p().join();
			string s = "Admin";
			p2p(SET_NAME).wrtc(s) << net::send;

		});

	GLUU::import_function<void()>(":chat_wait", [&]()
		{
			server.wait_for_peer();
		});

	GLUU::import_function<bool(string&, string&)>(":chat_join", [&](string& b, string& a)
		{
			bool success = p2p().join(a);
			if (success)
			{
				p2p(SET_NAME).wrtc(b) << net::send;
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
			p2p(MESSAGE_IN).wrtc(s) << net::send;
		});

	File file("GLUU/chat_menu.gluu", FILE_READING_STRING);

	GLUU::Compiled_ptr menu = GLUU::parse_copy(file.getString());

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
		menu->render({ 0,get_logical_size() });

		if (p2p().has_stream(MESSAGE_OUT))
		{
			string s;
			p2p(MESSAGE_OUT).rdc(s);

			ss << s << std::endl;
		}

		if (p2p().has_stream(NAME_UPDATE))
		{
			usernames.clear();

			size_t sz = p2p().read<size_t>(NAME_UPDATE);
			for (size_t i = 0; i < sz; i++)
			{
				size_t id = p2p().read<size_t>(NAME_UPDATE);
				string s;
				p2p().rdc(s);
				usernames.emplace(id, s);
			}

		}
	}
}