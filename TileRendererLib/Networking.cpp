#include "pch.h"
#include "Networking.h"

bool net::verbose_net = true;

bool ReadBuffer::has(ChannelID channel)
{
	if (read_buffer.count(channel) == 0) return false;
	return !read_buffer[channel].empty();
}

void ReadBuffer::end(ChannelID channel)
{
	DataStream* stream = new DataStream();
	stream->flag = channel;
	stream->data = data_buffer;
	read_buffer[stream->flag].push(stream);
	if (net::verbose_net)
		std::cout << "End of stream " << stream->flag << " (" << data_buffer.size() << " read)" << std::endl;
	data_buffer.clear();
}

void ReadBuffer::add(enet_uint8* data)
{
	if (net::verbose_net)
		std::cout << "Received packet... " << std::endl;
	data_buffer.push_back(data);
}

void Peer2Peer::handle_events(ENetEvent& event) {
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
		
		bool is_flag = read_packet(event.packet, data);

		if (is_flag)
			buffer.end(*(ChannelID*)(data));
		else
			buffer.add(data);

		enet_packet_destroy(event.packet);
	}
	break;

	case ENET_EVENT_TYPE_DISCONNECT:
		std::cout << "Peer disconnected" << std::endl;
		event.peer->data = NULL;
		break;
	}
}

void Peer2Peer::listen()
{
	ENetEvent event;
	while (enet_host_service(client, &event, 1000) > 0)
		handle_events(event);
}

void Peer2Peer::setup_listener()
{
	Threads::get()->queueJob([&](int i)
		{
			while (true)
				listen();
		});
}

Peer2Peer::Peer2Peer() : address({0,0}), peer(nullptr), current_peer(0), write_flag(0)
{
	if (enet_initialize() != 0)
		std::cerr << "An error occurred while initializing ENet.\n" << std::endl;
	atexit(enet_deinitialize);
}

void Peer2Peer::start_stream(ChannelID channel)
{
	if (net::verbose_net)
	std::cout << "Start of stream " << channel << std::endl;
	write_flag = channel;
}

void Peer2Peer::end_stream()
{
	if (net::verbose_net)
	std::cout << "End of stream " << write_flag << std::endl;
	send<ChannelID>(write_flag, true);
}

bool Peer2Peer::has_stream(ChannelID flag) { return buffer.has(flag); }

void Peer2Peer::wait_for_stream(ChannelID channel)
{
	while (!has_stream(channel)) { if (net::verbose_net) std::cout << "Waiting for stream " << channel << "\r"; }
}

/*
* Attends qu'un utilisateur se connecte
*/

void Peer2Peer::wait_for_peer()
{
	while (peer == NULL) { if (net::verbose_net) std::cout << "Waiting for peer \r"; };
}

/*
* Opens a networking session
*/

void Peer2Peer::host()
{
	address.host = ENET_HOST_ANY;
	address.port = 7777;

	client = enet_host_create(&address, 32, 1, 0, 0);

	if (client == NULL)
		printf("An error occurred while trying to create an ENet server host.");

	std::cout << "Server successfully created" << std::endl;

	setup_listener();

	connected = true;
}

/*
* Tries to join the given ip
*
* return TRUE if successfull
* Note that you need to port forward and open firewall and stuff to make this work
*/

bool Peer2Peer::join(std::string ip)
{
	client = enet_host_create(NULL, 1, 1, 0, 0);

	if (client == NULL)
		fprintf(stderr, "An error occurred while trying to create an ENet client host!\n");

	ENetEvent event;

	enet_address_set_host(&address, ip.c_str());
	address.port = 7777;

	peer = enet_host_connect(client, &address, 1, 0);
	if (peer == NULL)
		fprintf(stderr, "No available peers for initiating an ENet connection!\n");

	if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
	{
		std::cout << "Connection to " << ip << ":7777 succeeded" << std::endl;

		event.peer->data = (void*)"Host";
		enet_host_flush(client);

		setup_listener();

		connected = true;

		return true;
	}
	else
	{
		enet_peer_reset(peer);
		std::cout << "Connection to " << ip << ":7777 failed" << std::endl;
		return false;
	}
}

void Server::handle_events(ENetEvent& event) {
	switch (event.type)
	{
	case ENET_EVENT_TYPE_CONNECT:
		std::cout << "A new client (" << counter << ") connected from " << event.peer->address.host << ":" << event.peer->address.port << "\n";
		event.peer->data = (void*)(counter);
		peers.emplace(counter, event.peer);
		counter++;
		break;

	case ENET_EVENT_TYPE_RECEIVE:
	{
		//std::cout << "<" << (const char*)event.peer->data << "> " << event.packet->data << std::endl;

		enet_uint8* data;
		size_t peerID = get_id_from_address(event.peer->address);
		bool is_flag = read_packet(event.packet, data);

		if (is_flag)
		{
			buffers[peerID].end(*(ChannelID*)(data));
		}
		else
			buffers[peerID].add(data);

		if (net::verbose_net)
			std::cout << "Read from: " << peerID << std::endl;

		enet_packet_destroy(event.packet);
	}
	break;

	case ENET_EVENT_TYPE_DISCONNECT:
		std::cout << "Peer " << *(size_t*)event.peer->data << " disconnected" << std::endl;
		event.peer->data = NULL;
	}
}

void Server::listen()
{
	ENetEvent event;
	while (enet_host_service(client, &event, 1000) > 0)
		handle_events(event);
}

void Server::setup_listener()
{
	Threads::get()->queueJob([&](int i)
		{
			while (true)
				listen();
		});
	Threads::get()->queueJob([&](int i)
		{
			while (true)
				server_func(*this);
		});
}

void Server::open_session(function<void(Server&)> callback)
{
	address.host = ENET_HOST_ANY;
	address.port = 7777;

	client = enet_host_create(&address, 32, 1, 0, 0);

	if (client == NULL)
		printf("An error occurred while trying to create an ENet server host.");

	std::cout << "Server successfully created" << std::endl;

	server_func = callback;
	setup_listener();
}

void Server::start_stream(ChannelID channel)
{
	if (net::verbose_net)
	std::cout << "Start of stream " << channel << std::endl;
	write_flag = channel;
}

void Server::end_stream()
{
	if(messaging)
		send<ChannelID>(write_flag, current_peer, true);
	else if (broadcasting)
	{
		for (auto& p : peers)
		{
			send<ChannelID>(write_flag, p.first, true);
		}
	}

	broadcasting = false;
	messaging = false;
	
	if (net::verbose_net)
		std::cout << "Write end of stream " << write_flag << std::endl;
}

/*
* Opens a networking session
*/

void Server::host()
{
	address.host = ENET_HOST_ANY;
	address.port = 7777;

	client = enet_host_create(&address, 32, 1, 0, 0);

	if (client == NULL)
		printf("An error occurred while trying to create an ENet server host.");

	std::cout << "Server successfully created" << std::endl;

	setup_listener();

	connected = true;
}
