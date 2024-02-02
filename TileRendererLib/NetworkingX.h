#pragma once
#include "ThreadPool.h"
#include <iostream>
#include <string>
#include <list>
#include <map>
#include <enet/enet.h>
//#include "DynamicCounter.h"
//#include "Functions.h"

enum
{
	SIGNAL_DATA_SENT,
	SIGNAL_SYNC,
	SIGNAL_MESSAGE,
};

class NetworkingX
{
private:
	ENetAddress address;
	ENetHost* client;

	ThreadPool threads;
	std::list<enet_uint8*> buffer;
	std::map<size_t, size_t> signals;
	ENetPeer* peer;

	void handleEvents(ENetEvent& event)
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

			bool* b = new bool; //extract signal from data
			memcpy(b, event.packet->data, 1);
			enet_uint8* data = new enet_uint8[event.packet->dataLength - 1]; //copy the data
			memcpy(data, event.packet->data + 1, event.packet->dataLength - 1);

			if (*b) //packet is signal
			{
				//std::cout << "was signaled" << std::endl;
				size_t channel = *(size_t*)(data);
				signals[channel]++;
			}
			else {
				buffer.push_back(data);
			}

			delete b;
			enet_packet_destroy(event.packet);
		}
		break;

		case ENET_EVENT_TYPE_DISCONNECT:
			std::cout << "Peer disconnected" << std::endl;
			event.peer->data = NULL;
		}
	}

	void manualListen()
	{
		ENetEvent event;
		while (enet_host_service(client, &event, 1000) > 0)
			handleEvents(event);
	}

	void setupListener()
	{
		threads.start(1);
		threads.queueJob([&](int i)
			{
				while (true)
					manualListen();
			});
	}

	template<typename T>
	size_t getBytes(T& data) { return *(size_t*)(&data); }

public:
	NetworkingX()
	{
		if (enet_initialize() != 0)
			fprintf(stderr, "An error occurred while initializing ENet.\n");
		atexit(enet_deinitialize);
	}
	~NetworkingX() {  }

	template<typename T>
	void recvec(std::vector<T>& vec)
	{
		vec.clear();

		while (!hasSignal(SIGNAL_SYNC)) {}
		//std::cout << "reced..." << std::endl;

		size_t size = read<size_t>();

		for (size_t i = 0; i < size; i++)
			vec.push_back(read<T>());
	}

	template<typename T>
	void sendvec(std::vector<T>& vec)
	{
		send(vec.size());

		for (auto c : vec)
			send(c);

		signal(SIGNAL_SYNC);
		//	std::cout << "sent..." << std::endl;
	}

	template<typename T>
	void sendrecvec(std::vector<T>& vec, bool send)
	{
		std::cout << "____sednrecvec____" << std::endl;
		if (send)
			sendvec(vec);
		else
			recvec(vec);
	}

	template<typename T>
	void sendrec(T& obj, bool send)
	{
		std::cout << "____sednrec____" << std::endl;
		if (send)
		{
			this->send(obj);
			signal(SIGNAL_SYNC);
			//std::cout << "sent..." << std::endl;
		}
		else
		{
			while (!hasSignal(SIGNAL_SYNC)) {}
			//std::cout << "reced..." << std::endl;
			obj = read<T>();
		}
	}

	/*
	* Retourne si un signal a été recu
	*/
	bool hasSignal(size_t channel = 0)
	{
		if (signals[channel] == 0)
		{
			//std::cout << "waiting for signal..." << channel <<"\r";
			return false;
		}

		std::cout << "found signal!" << channel << std::endl;
		signals[channel]--;
		return true;
	}

	/*
	* Si il y a un message dans le buffer
	*/
	bool hasMessages() { return (buffer.size() > 0); }

	/*
	* Lit les données qui on été recues dans le buffer
	*/
	template<typename T>
	T read()
	{
		T data = *(T*)buffer.front();
		std::cout << "reading..." << std::bitset<sizeof(data) * 8>(*(size_t*)(&data)) << std::endl;
		delete buffer.front();
		buffer.pop_front();
		return data;
	}

	/*
	* Envoie un 'signal' à l'autre utilisateur
	* C'est utile pour synchroniser les deux pour s'assurer qu'ils sont les deux en train de lire/ecrire les bonnes choses
	*/
	void signal(size_t channel = 0)
	{
		std::cout << "signaling..." << channel;
		send<size_t>(channel, true);
	}

	/*
	* Tente d'envoyer les données à l'autre utilisateur
	*/
	template<typename T>
	void send(T data, bool signal = false)
	{
		std::cout << "sending..." << std::bitset<sizeof(data) * 8>(*(size_t*)(&data)) << std::endl;
		enet_uint8* bytes = new enet_uint8[sizeof(data) + 1];
		memcpy(bytes + 1, &data, sizeof(data));
		memcpy(bytes, &signal, sizeof(bool));

		ENetPacket* packet = enet_packet_create((void*)(bytes), sizeof(data) + 1, ENET_PACKET_FLAG_RELIABLE);

		delete bytes;

		if (enet_peer_send(peer, 0, packet) != 0)
			puts("Failed to send packet");
	}

	/*
	* Attends qu'un utilisateur se connecte
	*/
	void wait_for_peer()
	{
		while (peer == NULL) {};
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

		setupListener();
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

			setupListener();

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