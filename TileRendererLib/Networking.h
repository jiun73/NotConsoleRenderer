#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <enet/enet.h>
#include <bitset>

#include "ThreadPool.h"

using std::vector;
using std::queue;
using std::bitset;
using std::map;

struct DataStream
{
	size_t flag;
	vector<enet_uint8*> data;
};

class Networking
{
private:
	ENetAddress address;
	ENetHost* client;
	ENetPeer* peer;

	ThreadPool threads;

	vector<enet_uint8*> data_buffer;
	map<size_t, queue<DataStream*>> read_buffer;

	size_t write_flag;

	bool read_packet(ENetPacket* packet, enet_uint8** data)
	{
		bool* is_flag = new bool;
		*data = new enet_uint8[packet->dataLength - 1];
		memcpy(is_flag, packet->data, 1);
		memcpy(data, packet->data + 1, packet->dataLength - 1);

		delete is_flag;
	}

	template<typename T>
	size_t get_bytes(T& data) { return *(size_t*)(&data); }

	void handle_events(ENetEvent& event);

	void listen()
	{
		ENetEvent event;
		while (enet_host_service(client, &event, 1000) > 0)
			handle_events(event);
	}

	void setup_listener()
	{
		Threads::get()->start(0);
		Threads::get()->queueJob([&](int i)
			{
				while (true)
					listen();
			});
	}

public:
	Networking()
	{
		if (enet_initialize() != 0)
			std::cerr << "An error occurred while initializing ENet.\n" << std::endl;
		atexit(enet_deinitialize);
	}
	~Networking() { }

	void start_stream(size_t flag)
	{
		std::cout << "Start of stream " << flag << std::endl;
		write_flag = flag;
	}

	void end_stream()
	{
		std::cout << "End of stream " << write_flag << std::endl;
		send<size_t>(write_flag, true);
	}

	template<typename T>
	void send(T data, bool signal = false)
	{
		std::cout << "Sending... " << bitset<sizeof(data) * 8>(*(size_t*)(&data)) << std::endl;
		enet_uint8* bytes = new enet_uint8[sizeof(data) + 1];
		memcpy(bytes + 1, &data, sizeof(data));
		memcpy(bytes, &signal, sizeof(bool));

		ENetPacket* packet = enet_packet_create((void*)(bytes), sizeof(data) + 1, ENET_PACKET_FLAG_RELIABLE);

		delete[] bytes;

		if (enet_peer_send(peer, 0, packet) != 0)
			puts("Failed to send packet");
	}

	template<size_t I, typename T>
	void read_unfold(const vector<enet_uint8*>& data) {}

	template<size_t I , typename T>
	void read_unfold(const vector<enet_uint8*>& data, T& get)
	{
		get = *(T*)data.at(I);
	}

	template<size_t I, typename R, typename... Ts>
	void read_unfold(const vector<enet_uint8*>& data, R& get, Ts&... rest)
	{
		read_stream<I, R>(data, get);
		read_stream<I + 1, Ts...>(rest...);
	}

	template<typename... Ts>
	void read_stream(size_t flag, Ts&... get)
	{
		DataStream* stream = read_buffer[flag].front();
		read_unfold<0, Ts...>(stream->data);
		read_buffer[flag].pop();
	}

	/*
	* Opens a networking session
	*/
	void host()
	{
		address.host = ENET_HOST_ANY;
		address.port = 7777;

		client = enet_host_create(&address, 32, 1, 0, 0);

		if (client == NULL)
			printf("An error occurred while trying to create an ENet server host.");

		std::cout << "Server successfully created" << std::endl;

		setup_listener();
	}

	/*
	* Tries to join the given ip
	*
	* return TRUE if successfull
	* Note that you need to port forward and open firewall and stuff to make this work
	*/
	bool join(std::string ip = "127.0.0.1")
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

			return true;
		}
		else
		{
			enet_peer_reset(peer);
			std::cout << "Connection to " << ip << ":7777 failed" << std::endl;
			return false;
		}
	}
};