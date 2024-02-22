#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <enet/enet.h>
#include <bitset>

#include "ThreadPool.h"

using std::deque;
using std::queue;
using std::bitset;
using std::map;

enum NetFlags 
{
	NET_DATA,
	NET_CHANNEL,
	NET_REFLECT
};

struct DataStream
{
	size_t flag = -1;
	deque<enet_uint8*> data;
};

struct StreamEnd {};
struct StreamWait {};

namespace net
{
	extern bool verbose_net;
	inline StreamEnd send;
	inline StreamWait wait;
}

inline bool read_packet(ENetPacket* packet, enet_uint8*& data)
{
	bool* is_flag = new bool;
	data = new enet_uint8[packet->dataLength - 1];
	memcpy(is_flag, packet->data, 1);
	memcpy(data, packet->data + 1, packet->dataLength - 1);

	bool ret = *is_flag;
	delete is_flag;
	return ret;
}

struct ReadBuffer 
{
	deque<enet_uint8*> data_buffer;
	map<size_t, queue<DataStream*>> read_buffer;

	bool has(size_t channel);
	void end(size_t channel);
	void add(enet_uint8* data);

	template<typename T>
	T read(size_t channel) 
	{
		DataStream* stream = read_buffer[channel].front();
			T ret = *(T*)stream->data.front();
		stream->data.pop_front();

		if (stream->data.empty()) read_buffer[channel].pop();

		return ret;
	}

	template<size_t I>
	void read_unfold(const deque<enet_uint8*>& data) {}

	template<size_t I, typename T>
	void read_unfold(const deque<enet_uint8*>& data, T& get) { get = *(T*)data.at(I); }

	template<size_t I, typename R, typename... Ts>
	void read_unfold(const deque<enet_uint8*>& data, R& get, Ts&... rest)
	{
		read_unfold<I, R>(data, get);
		read_unfold<I + 1, Ts...>(data, rest...);
	}

	template<typename... Ts>
	void read_stream(size_t channel, Ts&... get)
	{
		DataStream* stream = read_buffer[channel].front();
		read_unfold<0, Ts...>(stream->data, get...);
		read_buffer[channel].pop();
	}
};

/*
* Classe de networking qui supporte seulement un hôte et un client
*/
class Peer2Peer
{
private:
	ENetAddress address = {0,0};
	ENetHost* client = nullptr;
	ENetPeer* peer = nullptr;

	//deque<enet_uint8*> data_buffer;
	//map<size_t, queue<DataStream*>> read_buffer;

	ReadBuffer buffer;

	size_t write_flag;
	size_t current_peer;
	bool connected = false;

	void handle_events(ENetEvent& event);
	void listen();
	void setup_listener();

public:
	Peer2Peer();
	~Peer2Peer() { }

	void start_stream(size_t channel);
	void end_stream();

	template<typename T>
	void send(T data, bool signal = false)
	{
		if (net::verbose_net)
			std::cout << "Sending... " << bitset<sizeof(data) * 8>(*(size_t*)(&data)) << std::endl;
		enet_uint8* bytes = new enet_uint8[sizeof(data) + 1];
		memcpy(bytes + 1, &data, sizeof(data));
		memcpy(bytes, &signal, sizeof(bool));

		ENetPacket* packet = enet_packet_create((void*)(bytes), sizeof(data) + 1, ENET_PACKET_FLAG_RELIABLE);

		delete[] bytes;

		if (enet_peer_send(peer, 0, packet) != 0)
			puts("Failed to send packet");
	}

	/*
	* Read a stream of data that was sent in a given channel
	*/
	template<typename... Ts>
	void read_stream(size_t channel, Ts&... get) { buffer.read_stream(channel, get...); }

	template<typename T>
	T read(size_t channel) { return buffer.read<T>(channel); }

	bool has_stream(size_t flag);

	void wait_for_stream(size_t channel);

	/*
	* Attends qu'un utilisateur se connecte
	*/
	void wait_for_peer();

	bool is_connected() { return connected; }

	/*
	* Opens a networking session
	*/
	void host();

	/*
	* Tries to join the given ip
	*
	* return TRUE if successfull
	* Note that you need to port forward and open firewall and stuff to make this work
	*/
	bool join(std::string ip = "127.0.0.1");

	Peer2Peer& operator[](size_t i)
	{
		write_flag = i;
		return *this;
	}

	template<typename T>
	Peer2Peer& operator<<(const T& object)
	{
		send(object);
		return *this;
	}

	void operator<<(const StreamEnd& end)
	{
		end_stream();
	}

	template<typename T>
	Peer2Peer& operator>>(T& ref)
	{
		if constexpr (std::is_same_v<T, StreamWait>)
		{
			wait_for_stream(write_flag);
		}
		else
		{
			ref = read<T>(write_flag);
		}
		return *this;
	}
};

class Server
{
private:
	ENetAddress address = { 0,0 };
	ENetHost* client = nullptr;
	map<size_t, ENetPeer*> peers;

	map<size_t, ReadBuffer> buffers;

	size_t counter = 0;
	size_t write_flag = 0;
	size_t current_peer = 0;

	bool open = false;
	bool connected = false;
	bool broadcasting = false;
	bool messaging = false;

	function<void(Server&)> server_func;

	void handle_events(ENetEvent& event);
	void listen();
	void setup_listener();

public:
	map<size_t, ReadBuffer>& get_buffers() { return buffers; }

	void open_session(function<void(Server&)> callback);
	void start_stream(size_t channel);
	void end_stream();

	template<typename T>
	void send(T data, size_t peerid, bool signal = false)
	{
		
		enet_uint8* bytes = new enet_uint8[sizeof(data) + 1];
		memcpy(bytes + 1, &data, sizeof(data));
		memcpy(bytes, &signal, sizeof(bool));

		ENetPacket* packet = enet_packet_create((void*)(bytes), sizeof(data) + 1, ENET_PACKET_FLAG_RELIABLE);

		delete[] bytes;

		if (enet_peer_send(peers.at(peerid), 0, packet) != 0)
			puts("Failed to send packet");

		if (net::verbose_net)
			std::cout << "Sending... " << bitset<sizeof(data) * 8>(*(size_t*)(&data)) << std::endl;
	}

	/*
	* Read a stream of data that was sent in a given channel
	*/
	template<typename... Ts>
	void read_stream(size_t channel, Ts&... get)
	{
		buffers[channel].read_stream(channel, get...);
	}

	template<typename T>
	T read(size_t channel, size_t peerID)
	{
		return buffers[channel].read(channel);
	}

	/*
	* Attends qu'un utilisateur se connecte
	*/
	void wait_for_peer()
	{
		while (peers.empty()) { if (net::verbose_net) std::cout << "Waiting for peer \r"; };
	}

	int peer_count() { return peers.size(); }

	bool is_connected() { return connected; }

	/*
	* Opens a networking session
	*/
	void host();

	Server& broadcast(size_t channel)
	{
		write_flag = channel;
		broadcasting = true;
		messaging = false;

		return *this;
	}

	Server& message(size_t channel, size_t peerid)
	{
		write_flag = channel;
		current_peer = peerid;
		messaging = true;
		broadcasting = false;

		return *this;
	}

	void reflect(size_t channel)
	{

	}

	size_t get_id_from_address(ENetAddress address)
	{
		for (auto& p : peers)
		{
			if (address.host == p.second->address.host && address.port == p.second->address.port)
				return p.first;
		}
	}

	template<typename T>
	Server& operator<<(const T& object)
	{
		if (!broadcasting) 
		{
			std::cout << "not broadcasting!" << std::endl;
			return *this;
		}

		for (auto& p : peers)
		{
			send(object, p.first);
		}
		
		return *this;
	}

	template<typename T>
	Server& operator<(const T& object)
	{
		if (!messaging)
		{
			std::cout << "not messaging!" << std::endl;
			return *this;
		}

		send(object, current_peer);

		return *this;
	}

	void operator<<(const StreamEnd& end)
	{
		end_stream();
	}

	void operator<(const StreamEnd& end)
	{
		end_stream();
	}
};