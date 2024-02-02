#include "Networking.h"

void Networking::handle_events(ENetEvent& event)
{
	switch (event.type)
	{
	case ENET_EVENT_TYPE_CONNECT:
		std::cout << "A new client connected from " << event.peer->address.host << ":" << event.peer->address.port << "\n";
		event.peer->data = (void*)("Peer");
		peer = event.peer;
		break;

	case ENET_EVENT_TYPE_RECEIVE:
	{
		//std::cout << "<" << (const char*)event.peer->data << "> " << event.packet->data << std::endl;

		enet_uint8* data;
		bool is_flag = read_packet(event.packet, &data);

		if (is_flag)
		{	
			DataStream* stream = new DataStream();
			stream->flag = *(size_t*)(data);
			stream->data = data_buffer;
			read_buffer[stream->flag].push_back(stream);
			data_buffer.clear();
			std::cout << "End of stream " << stream->flag << " (" << read_buffer.size() << " read)" << std::endl;
		}
		else
		{
			std::cout << "Received packet... " << std::endl;
			data_buffer.push_back(data);
		}

		enet_packet_destroy(event.packet);
	}
	break;

	case ENET_EVENT_TYPE_DISCONNECT:
		std::cout << "Peer disconnected" << std::endl;
		event.peer->data = NULL;
	}
}
