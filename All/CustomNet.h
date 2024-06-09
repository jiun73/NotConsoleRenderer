#pragma once
#include "RAS.h"
#include "Defines.h"
#include "enet/enet.h"

namespace FIGHT
{
	class CustomNet
	{
	private:
		RAS::Manager* manager;
		ENetAddress address = { 0,0 };
		ENetHost* client = nullptr;
		ENetPeer* peer = nullptr;
		std::vector<std::pair<char, RAS::Time>> buffer;

		bool determine_round_trip = true;
		bool waiting_for_answer = false;
		int current_trip_index = 0;
		const int max_round_trips = 20;
		RAS::Time current_trip_time = 0;
		std::vector<RAS::Time> round_trips;
		bool is_host = true;

		std::atomic_bool lock_buffer = false;
		std::list<std::thread> threads;

	public:
		std::mutex lock;

		bool hosting() { return is_host; }

		void sync_clocks();

		bool connected() { return (peer != nullptr); }

		void send(RAS::Time time, char c)
		{
			enet_uint8* bytes = new enet_uint8[sizeof(size_t) + sizeof(char)];
			memcpy(bytes, &c, sizeof(char));
			memcpy(bytes + 1, &time, sizeof(3));

			ENetPacket* packet = enet_packet_create((void*)(bytes), sizeof(3) + sizeof(char), ENET_PACKET_FLAG_RELIABLE);

			

			if (enet_peer_send(peer, 0, packet) != 0)
				puts("Failed to send packet");

			delete[] bytes;
		}

		void handle_buffer()
		{
			if (buffer.size() > 0)
			{
				lock.lock();
				for (auto& b : buffer)
					process_event_pack(*this, *manager, b.second, b.first, !hosting());

				buffer.clear();
				lock.unlock();
			}
		}

		void handle_events(ENetEvent& event) {
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				std::cout << "A new client connected from " << event.peer->address.host << ":" << event.peer->address.port << "\n";
				event.peer->data = (void*)("Peer");
				peer = event.peer;
				break;

			case ENET_EVENT_TYPE_RECEIVE:
			{
				enet_uint8* data = event.packet->data;
				char c = *data;
				RAS::Time time = *(size_t*)(data + 1);

				if (c == '\xFF')
				{
					manager->set_start(time);
				}
				else
				{
					lock.lock();
					buffer.push_back({ c,time });
					lock.unlock();
				}

				RAS::Time now = manager->now();
				if (time > now)
				{
					std::cout << "Received packet " << time - now << " ms early" << std::endl;
				}
				else
				{
					std::cout << "Received packet " << now - time << " ms late" << std::endl;
				}

				//if (!determine_round_trip)
				//{
				//	
				//}
				//else if (!is_host && waiting_for_answer)
				//{
				//	RAS::Time now_time = manager->now();
				//	RAS::Time trip_time = now_time - current_trip_time;

				//	std::cout << "Got reply from peer from round trip #" << (int)c << " in " << trip_time << "ms" << std::endl;

				//	round_trips.push_back(trip_time);
				//	waiting_for_answer = false;
				//	current_trip_index++;

				//	if (current_trip_index > max_round_trips)
				//	{
				//		determine_round_trip = false;

				//		std::cout << "Ending round trip analysis..." << std::endl;

				//		RAS::Time sum = 0;

				//		for (auto& s : round_trips)
				//			sum += s;

				//		RAS::Time avg = (double)sum / round_trips.size();

				//		std::cout << "Calculated an average of "  << avg << "ms in a total of " << sum << "ms" << std::endl;

				//		send(avg, '\xFF');
				//	}
				//}
				//else
				//{
				//	if (c == '\xFF')
				//	{
				//		std::cout << "End of round trip analysis. average of " << time << " ms per round trip" << std::endl;
				//		determine_round_trip = false;
				//	}
				//	else
				//	{
				//		send(time, c); //send back to determine round-trip
				//		std::cout << "Got round-trip message from host (" << time << "," << (int)c << "). sending back..." << std::endl;
				//	}
				//}

				enet_packet_destroy(event.packet);
			}
			break;

			case ENET_EVENT_TYPE_DISCONNECT:
				std::cout << "Peer disconnected" << std::endl;
				event.peer->data = NULL;
				break;
			}


			
		}

		void listen()
		{
			

			ENetEvent event;
			while (enet_host_service(client, &event, 0) > 0)
			{
				handle_events(event);
			}

			//if (determine_round_trip && !is_host && !waiting_for_answer)
			//{
			//	current_trip_time = manager->now();
			//	std::cout << "Sending round trip request #" << current_trip_index + 1 << " at " << current_trip_time << " ms. waiting for reply" << std::endl;
			//	send(current_trip_time, (int)current_trip_index);
			//	waiting_for_answer = true;
			//}

			
		}

		void setup_listener()
		{
			Threads::get()->queueJob([&](int i)
				{
					while (true)
						listen();
				});
		}

		void wait_for_peer()
		{
			while (peer == nullptr) { std::cout << "Waiting for peer \r"; };
		}

		void setup(RAS::Manager* manager)
		{
			this->manager = manager;
		}

		void host()
		{
			address.host = ENET_HOST_ANY;
			address.port = 55566;

			client = enet_host_create(&address, 32, 1, 0, 0);

			if (client == nullptr)
				printf("An error occurred while trying to create an ENet server host.");

			std::cout << "Server successfully created on " << client->address.host << ":" << client->address.port << std::endl;
			

			is_host = true;

			setup_listener();
			wait_for_peer();

			std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;

			sync_clocks();
		}

		bool join(string ip)
		{

			client = enet_host_create(NULL, 1, 1, 0, 0);

			if (client == NULL)
				fprintf(stderr, "An error occurred while trying to create an ENet client host!\n");

			ENetEvent event;

			enet_address_set_host(&address, ip.c_str());
			address.port = 55566;

			peer = enet_host_connect(client, &address, 1, 0);
			if (peer == NULL)
				fprintf(stderr, "No available peers for initiating an ENet connection!\n");


			if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
			{
				std::cout << "Connection to " << ip << ":7777 succeeded" << std::endl;

				event.peer->data = (void*)"Host";
				enet_host_flush(client);

				is_host = false;

				setup_listener();

				sync_clocks();
				return true;
			}
			else
			{
				enet_peer_reset(peer);
				std::cout << "Connection to " << ip << ":7777 failed" << std::endl;
				return false;
			}
		}

		void start(RAS::Manager* manager)
		{
			this->manager = manager;

			std::cout << "Host?? ";
			char c;
			std::cin >> c;



			RAS::Time peer_delay;

			if (c == 'Y')
			{
				address.host = ENET_HOST_ANY;
				address.port = 55566;

				client = enet_host_create(&address, 32, 1, 0, 0);

				if (client == NULL)
					printf("An error occurred while trying to create an ENet server host.");

				std::cout << "Server successfully created" << std::endl;

				setup_listener();
				wait_for_peer();
				std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;

				manager->set_start();
			}
			else
			{
				while (true)
				{
					std::string ip;

					std::cout << "enter ip" << std::endl;

					std::cin >> ip;

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

						manager->set_start();
						break;
					}
					else
					{
						enet_peer_reset(peer);
						std::cout << "Connection to " << ip << ":7777 failed" << std::endl;
					}
				}
			}
		}
	};
}