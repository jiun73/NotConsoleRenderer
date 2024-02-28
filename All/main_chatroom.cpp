#include "main_chatroom.h"
#include "GLUU.h"

enum MessageTypes
{
	MESSAGE_IN,
	MESSAGE_OUT,
	SET_NAME, //string
	NAME_UPDATE
};

std::stringstream ss;
map<size_t, string> usernames;
GLUU::Compiled_ptr menu;

void set_chatroom()
{
	Server server;

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

	menu = GLUU::parse_file("GLUU/chat_menu.gluu");
}

void update_chatroom()
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

 GLUU::ImportFunction<decltype(set_chatroom)> gluu_set_chatroom_import("$" + string("set_chatroom"), set_chatroom);
 GLUU::ImportFunction<decltype(update_chatroom)> gluu_update_chatroom_import("$" + string("update_chatroom"), update_chatroom);
