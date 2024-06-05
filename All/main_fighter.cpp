#include "pch.h"
#include "RAS_Actor.h"
#include "AnimationX.h"

namespace FIGHT
{
	size_t operator ""ms(size_t ms)
	{
		return ms;
	}

	size_t operator ""s(size_t s)
	{
		return s * 1000;
	}

	size_t operator ""s(long double s)
	{
		return s * 1000;
	}

	namespace Gen {
		enum Actorenum
		{
			PLAYER1,
			PLAYER2,
			BALL
		};

		enum Fieldenum
		{
			POSX,
			POSY
		};

		enum GeneratorEnum
		{
			STAY,
			PLAYER1_START_POSY,
			PLAYER2_START_POSY,
			PLAYER1_START_POSX,
			PLAYER2_START_POSX,
			BALL_START_POS,
			MOVE_SPEED_300_BOUNCE,
			MOVE_SPEED_N300_BOUNCE,
		};
	}

	struct AnimationManager 
	{
	private:
		map<string, size_t> ids_by_path;
		vector<AnimationX> sets;

	public:
		void load_animation(const string& path)
		{
			AnimationX anim;
			NCR::File file(path, NCR::Files::FILE_READING);
			anim.readwrite(file, path);

			sets.push_back(anim);
			ids_by_path.emplace(path, sets.size() - 1);
		}

		size_t get_frame_for_animation(size_t relative_time, size_t animation, size_t setid)
		{
			auto& map = sets.at(setid).get_frame_time_list();
			auto it = map.lower_bound(relative_time);

			if (it != map.begin())
			{
				return (--it)->second;
			}
		}
	};

	enum 
	{
		POINT,
		LINEAR
	};

	void point_func(RAS::Time time, int& i, array<double, 1> arr)
	{
		i = arr.at(0);
	}

	void linear_func(RAS::Time time, int& i, array<double, 2> arr)
	{
		double start = arr.at(0);
		double speed = arr.at(1);
		i = ((time / 1000.0) * speed) + start;
	}

	RAS::Time find_linear(int y, array<double, 2> arr)
	{
		double start = arr.at(0);
		double speed = arr.at(1);
		return ((y - start) / speed) * 1000.0;
	}

	struct CollisionSystem 
	{

		RAS::Time collision_time = 0;
		RAS::Time check_resume = 0;
		bool continue_detection = false;
		RAS::Time time;
		RAS::Manager* manager;
		vector<RAS::Actor>* actors;

		enum 
		{
			POSX,
			POSY,
		};

		map<RAS::Time, RAS::Event>::iterator get_event_from_timeline(RAS::Time time, RAS::Timeline& timeline)
		{
			map<RAS::Time, RAS::Event>::iterator event = timeline.events.lower_bound(time);
			if (event != timeline.events.begin()) { event--; }
			return event;
		}

		template<typename T>
		RAS::Time get_next_event_time(typename T::iterator it1, typename T::iterator it2, const typename T::iterator& end1, const typename T::iterator& end2, RAS::Time adder1 = 0, RAS::Time adder2 = 0)
		{
			RAS::Time next_event_time = 0;

			it1++;
			it2++;

			if (it1 == end1 && it2 != end2)
			{
				return it2->first + adder2;
			}
			else if (it1 != end1 && it2 == end2)
			{
				return it1->first + adder1;
			}
			else if (it1 == end1 && it2 == end2)
			{
				return -1;
			} //there's no next event, so there are endless, so we pass the maximum value

			if (it1->first > it2->first)
			{
				return it2->first + adder2;
			}
			else if (it1->first < it2->first)
			{
				return it1->first + adder1;
			}

			it1--;
			it2--;

			return next_event_time;
		}

		template<typename T>
		bool get_next_pair(typename T::iterator& it1, typename T::iterator& it2, const typename T::iterator& end1, const typename T::iterator& end2)
		{
			it1++;
			it2++;

			if (it1 == end1 && it2 != end2)
			{
				it1--;
				return true;
			}
			else if (it1 != end1 && it2 == end2)
			{
				it2--;
				return true;
			}
			else
			{
				return false;
			}

			if (it1->first > it2->first)
			{
				it1--;
			}
			else if (it1->first < it2->first)
			{
				it2--;
			}

			return true;
		}

		bool calculate_collision_time(RAS::Modifier* mod1, RAS::Modifier* mod2, RAS::Time mod1_time, RAS::Time mod2_time, RAS::Time start_time, RAS::Time next_time, RAS::Time& collision_time)
		{
			assert(mod1->is_reversible());
			assert(mod2->is_reversible() && mod2->reverse_type() == POINT); //we assume the x position of the paddle doesn't change
			int p1_x = mod2->reverse_params().at(0);


			auto params = mod1->reverse_params();

			

			switch (mod1->reverse_type())
			{
			case POINT:
				collision_time = mod2_time;
				return (p1_x == params.at(0));
			case LINEAR:
			{
				RAS::Time potential_time = mod1_time + find_linear(p1_x, { params.at(0), params.at(1) });

				collision_time = potential_time;

				return (potential_time >= start_time && potential_time >= mod1_time && potential_time < next_time);
			}
			break;
			default:
				assert(false);
				return false;
			}
		}

		bool find_collision_pair(RAS::Event& event1, RAS::Event event2, RAS::Time start_time, RAS::Time stop_time)
		{
			if (event1.modifiers.empty() || event2.modifiers.empty()) return false;

			map<RAS::Time, RAS::Modifier*>::iterator event1_mod = event1.modifiers.lower_bound(event1.start_time - std::min(event1.start_time, start_time));
			map<RAS::Time, RAS::Modifier*>::iterator event2_mod = event2.modifiers.lower_bound(event2.start_time - std::min(event2.start_time, start_time));

			if (event1_mod != event1.modifiers.begin()) { event1_mod--; }
			if (event2_mod != event2.modifiers.begin()) { event2_mod--; }

			while (true)
			{
				RAS::Time next_time = get_next_event_time<map<RAS::Time, RAS::Modifier*>>(event1_mod, event2_mod, event1.modifiers.end(), event2.modifiers.end(), event1.start_time, event2.start_time);

				next_time = std::min(stop_time, next_time);
				check_resume = next_time;

				if (calculate_collision_time(event1_mod->second, event2_mod->second, event1.start_time + event1_mod->first, event2.start_time + event2_mod->first, start_time, next_time, collision_time))
				{
					return true;
				}

				if (!get_next_pair<map<RAS::Time, RAS::Modifier*>>(event1_mod, event2_mod, event1.modifiers.end(), event2.modifiers.end()))
				{
					break;
				}

				if (event1_mod->first > stop_time && event2_mod->first > stop_time) break; //both events are past the stop time, stop
			}

			return false;
		}

		bool find_collision(RAS::Time start_time, RAS::ActorID ball_actor, RAS::ActorID player_actor)
		{
			RAS::Timeline& ball_X_timeline = actors->at(ball_actor).timelines.at(POSX);
			RAS::Timeline& player_X_timeline = actors->at(player_actor).timelines.at(POSX);

			if (ball_X_timeline.events.empty() || player_X_timeline.events.empty()) return false;

			map<RAS::Time, RAS::Event>::iterator ball_it = get_event_from_timeline(start_time, ball_X_timeline);
			map<RAS::Time, RAS::Event>::iterator player_it = get_event_from_timeline(start_time, player_X_timeline);

			while (true)
			{
				RAS::Time next_event_time = get_next_event_time<map<RAS::Time, RAS::Event>>(ball_it, player_it, ball_X_timeline.events.end(), player_X_timeline.events.end());

				if (find_collision_pair(ball_it->second, player_it->second, start_time, next_event_time))
				{
					return true;
				}

				if (!get_next_pair<map<RAS::Time, RAS::Event>>(ball_it, player_it, ball_X_timeline.events.end(), player_X_timeline.events.end()))
				{
					continue_detection = false;
					break;
				}
			}

			return false;
		}

		struct CollisionInfo
		{
			bool found = false;
			RAS::Time collision_time;
			bool resume = false;
			RAS::Time resume_check;
			RAS::ActorID actor = 0;
		};

		CollisionInfo find_collision_info(RAS::Time start_time, RAS::ActorID ball_actor, RAS::ActorID player_actor)
		{
			CollisionInfo ret;
			ret.found = find_collision(start_time, ball_actor, player_actor);
			ret.collision_time = collision_time;
			ret.resume = continue_detection;
			ret.resume_check = check_resume;
			return ret;
		}

		void resolve_collision(RAS::ActorID ball, RAS::ActorID player, RAS::GeneratorID gen, RAS::FieldID field) 
		{
			RAS::Time p1_coll_check = time;

			continue_detection = true;
			while (continue_detection && find_collision(p1_coll_check, ball, player))
			{
				int ballY = manager->actor_field_at<int>(collision_time, ball, POSY);
				int playerY = manager->actor_field_at<int>(collision_time, player, POSY);

				if (ballY >= playerY && ballY <= playerY + 90)
				{
					manager->add_event(collision_time, ball, gen, field, true);
				}

				p1_coll_check = std::max(check_resume, collision_time + 1);

			}
		}

		RAS::ActorID ball = 2;
		std::array<RAS::ActorID, 2> paddles = { 1 , 0 };
		std::array<CollisionInfo, 2> collisions;

		RAS::Time valid_time = 0;

		void on_snap(RAS::Manager* arg_manager, RAS::Time arg_time, vector<RAS::Actor>& arg_actors)
		{
			//valid_time = 0;

			//std::cout << arg_time << std::endl;
			if (arg_time <= valid_time) return;

			for (auto& c : collisions) c = CollisionInfo();



			while (true)
			{
				//if the snapshot is being taken from a time where the timeline was not invalidated, then we can return early
				//also, this way, we avoid edge cases where collision happen continously (like a bounce) and we avoid calculating too far into the future (which will likely be invalidated anyway)

				if (arg_time < valid_time ) return;

				bool no_collision = true;
				size_t i = 0;
				for (auto& p : paddles)
				{
					CollisionInfo& info = collisions.at(i);
					info = find_collision_info(info.found ? info.resume_check : valid_time, ball, p);
					info.actor = p;

					if (info.collision_time < valid_time) info.found = false;

					if (info.found) no_collision = false;
					i++;
				}

				if (no_collision) //no collision found from there, so we break
				{
					valid_time = arg_time;
					std::cout << "nothing found... " << valid_time << std::endl;
					break;
				}

				//otherwise we need to find the earliest collision found and restart the process from there
				
				CollisionInfo* early_collision = nullptr;
				for (auto& c : collisions)
				{
					if (c.found)
					{
						if (early_collision == nullptr) 
						{
							early_collision = &c;
						}
						else if (c.collision_time < early_collision->collision_time)
						{
							early_collision->found = false;
							early_collision = &c;
						}
						else
						{
							c.found = false; // we want to be able to find this collision again
						}
					}
				}

				valid_time = early_collision->collision_time + 1;

				int ballY = manager->actor_field_at<int>(early_collision->collision_time, ball, POSY);
				int playerY = manager->actor_field_at<int>(early_collision->collision_time, early_collision->actor, POSY);

				if (ballY >= playerY && ballY <= playerY + 90)
				{
					switch (early_collision->actor)
					{
					case 0:
						std::cout << "collision 1 found " << valid_time << std::endl;
						manager->add_event(early_collision->collision_time, ball, 6, POSX, true);
						
						
						break;
					case 1:
						std::cout << "collision 2 found " << valid_time << std::endl;
						manager->add_event(early_collision->collision_time, ball, 7, POSX, true);
						
						break;
					default:
						break;
					}

					const RAS::Event& balle = actors->at(ball).timelines.at(POSY).event_at(early_collision->collision_time);

					if (balle.modifier_at(early_collision->collision_time - balle.start_time)->reverse_params()[1] > 0)
					{
						manager->add_event(early_collision->collision_time, ball, 6, POSY, true);
					}
					else
					{
						manager->add_event(early_collision->collision_time, ball, 7, POSY, true);
					}
				}
			}
		}

		void update(RAS::Manager* arg_manager, RAS::Time arg_time, vector<RAS::Actor>& arg_actors)
		{
			actors = &arg_actors;
			manager = arg_manager;
			time = arg_time;

			
			//valid_time = std::min(valid_time, arg_time); //take the earliest time where the timeline is still valid and rebuild it from there on the next snapshot (this saves a lot of calculation per event added)
			valid_time = 0;
			std::cout << "new event: " << valid_time << std::endl;
		}
	};

	class CustomNet;

	void process_event_pack(CustomNet& net, RAS::Manager& manager, RAS::Time time, char code, bool send = false);

	class CustomNet 
	{
	private:
		RAS::Manager* manager;
		ENetAddress address = { 0,0 };
		ENetHost* client = nullptr;
		ENetPeer* peer = nullptr;
		std::vector<std::pair<char, RAS::Time>> buffer;

	public:
		void send(RAS::Time time, char c) 
		{
			enet_uint8* bytes = new enet_uint8[sizeof(size_t) + sizeof(char)];
			memcpy(bytes, &c, sizeof(char));
			memcpy(bytes + 1, &time, sizeof(3));

			ENetPacket* packet = enet_packet_create((void*)(bytes), sizeof(3) + sizeof(char), ENET_PACKET_FLAG_RELIABLE);

			delete[] bytes;

			if (enet_peer_send(peer, 0, packet) != 0)
				puts("Failed to send packet");
		}

		void handle_buffer()
		{
			for(auto& b : buffer)
				process_event_pack(*this, *manager, b.second, b.first);

			buffer.clear();
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
				buffer.push_back({ c,time });
				
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
				handle_events(event);
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
			while (peer == NULL) { "Waiting for peer \r"; };
		}

		void setup(RAS::Manager* manager)
		{
			this->manager = manager;
		}

		void host() 
		{
			address.host = ENET_HOST_ANY;
			address.port = 7777;

			client = enet_host_create(&address, 32, 1, 0, 0);

			if (client == NULL)
				printf("An error occurred while trying to create an ENet server host.");

			std::cout << "Server successfully created" << std::endl;

			setup_listener();
			wait_for_peer();
			std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;

			manager->set_start();
		}

		bool join(string ip)
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

				manager->set_start();
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
				address.port = 7777;

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

	void process_event_pack(CustomNet& net, RAS::Manager& manager, RAS::Time time, char code, bool send)
	{
		switch (code) 
		{
		case '\x01':
			manager.add_event(time, Gen::PLAYER1, Gen::MOVE_SPEED_N300_BOUNCE, Gen::POSY);
			break;

		case '\x02':
			manager.add_event(time, Gen::PLAYER1, Gen::MOVE_SPEED_300_BOUNCE, Gen::POSY);
			break;

		case '\x03':
			manager.add_event(time, Gen::PLAYER1, Gen::STAY, Gen::POSY);
			break;

		case '\x10':
			manager.add_event(time, Gen::PLAYER2, Gen::MOVE_SPEED_N300_BOUNCE, Gen::POSY);
			break;

		case '\x20':
			manager.add_event(time, Gen::PLAYER2, Gen::MOVE_SPEED_300_BOUNCE, Gen::POSY);
			break;

		case '\x30':
			manager.add_event(time, Gen::PLAYER2, Gen::STAY, Gen::POSY);
			break;

		default:
			break;
		}

		if (send)
		{
			net.send(time, code);
		}
	}


	void main_fight() 
	{
		set_window_size({200,200});
		set_window_resizable();

		RAS::Manager man;
		CustomNet net;

		man.set_time_fetcher(SDL_GetTicks);

		RAS::FieldID posX = man.register_field<int>();
		RAS::FieldID posY = man.register_field<int>();

		man.register_system<CollisionSystem>();

		RAS::GeneratorID stay = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func, POINT, { old_x });
			}), Gen::STAY);

		RAS::GeneratorID player1_start_posY = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			return RAS::Event().add_modifier<int, 1>(0ms, point_func, POINT, { 100 });
			}), Gen::PLAYER1_START_POSY);

		RAS::GeneratorID player2_start_posY = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			return RAS::Event().add_modifier<int, 1>(0ms, point_func, POINT, { 650 });
			}), Gen::PLAYER2_START_POSY);

		RAS::GeneratorID player1_start_posX = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func, POINT, { 100 });
			}), Gen::PLAYER1_START_POSX);

		RAS::GeneratorID player2_start_posX = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event{
				return RAS::Event().add_modifier<int, 1>(0ms, point_func, POINT, { 900 });
			}), Gen::PLAYER2_START_POSX);

		RAS::GeneratorID ball_start_pos = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			return RAS::Event().add_modifier<int, 1>(0ms, point_func, POINT, { 500 });
			}), Gen::BALL_START_POS);

		RAS::GeneratorID move_speed_100 = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event
			{
				double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
				RAS::Event e;
				RAS::Time bounce_time = 0;
				double speed = 300;
				double max = 1000;
				double min = 0;
				bool dirmax = true;
				e.add_modifier<int, 2>(0ms, linear_func, LINEAR, { old_x, speed });
				bounce_time += find_linear(dirmax ? max : min, { old_x, dirmax ? speed : -speed });
				for (int i = 0; i < 10; i++)
				{
					e.add_modifier<int, 2>(bounce_time, linear_func, LINEAR, { dirmax ? max : min, dirmax ? -speed : speed });
					dirmax = !dirmax;
					bounce_time += find_linear(dirmax ? max : min, { !dirmax ? max : min, dirmax ? speed : -speed });
				}
				
				return e;
			}), Gen::MOVE_SPEED_300_BOUNCE);

		RAS::GeneratorID move_speed_n100 = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event
			{
				double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
				RAS::Event e;
				RAS::Time bounce_time = 0;
				double speed = 300;
				double max = 1000;
				double min = 0;
				bool dirmax = false;
				e.add_modifier<int, 2>(0ms, linear_func, LINEAR, { old_x, -speed });
				bounce_time += find_linear(dirmax ? max : min, { old_x, dirmax ? speed : -speed });
				for (int i = 0; i < 10; i++)
				{
					e.add_modifier<int, 2>(bounce_time, linear_func, LINEAR, { dirmax ? max : min, dirmax ? -speed : speed });
					dirmax = !dirmax;
					bounce_time += find_linear(dirmax ? max : min, { !dirmax ? max : min, dirmax ? speed : -speed });
				}
				return e;
			}), Gen::MOVE_SPEED_N300_BOUNCE);

		RAS::ActorID player1 = man.register_actor(0b11);
		RAS::ActorID player2 = man.register_actor(0b11);
		RAS::ActorID ball = man.register_actor(0b11);

		net.setup(&man);

		bool b = true;

		GLUU::import_function<void()>(":exit", [&]()
			{
				b = false;
			});

		GLUU::import_function<void()>(":host_game", [&]()
			{
				net.host();
				b = false;
			});

		GLUU::import_function<bool(string)>(":join_game", [&](string s)
			{
				bool j = net.join(s);
				if (!j)
					return false;
				b = false;

				keyboard().closeTextInput();
				return true;
			});

		GLUU::Compiled_ptr menu = GLUU::parse_file("PongMenu.gluu");


		while (run())
		{
			pencil(COLOR_BLACK);
			draw_clear();

			menu->render({ 0,get_window_size() });

			if (!b)
			{
				break;
			}
		}

		man.add_event(0ms, player1, player1_start_posX, posX);
		man.add_event(0ms, player1, player1_start_posY, posY);
		man.add_event(0ms, player2, player2_start_posX, posX);
		man.add_event(0ms, player2, player2_start_posY, posY);
		man.add_event(0ms, ball, ball_start_pos, posX);
		man.add_event(0ms, ball, ball_start_pos, posY);
		man.add_event(1s, ball, move_speed_n100, posX);
		man.add_event(1s, ball, move_speed_n100, posY);

		std::vector<V2d_i>  last_ball_pos;

		while (run())
		{
			pencil(COLOR_BLACK);
			draw_clear();
			

			if (key_pressed(SDL_SCANCODE_W) || (key_released(SDL_SCANCODE_S) && key_held(SDL_SCANCODE_W)))
			{
				process_event_pack(net, man, man.now(), '\x01', true);
			}
			else if (key_pressed(SDL_SCANCODE_S) || (key_released(SDL_SCANCODE_W) && key_held(SDL_SCANCODE_S)))
			{
				process_event_pack(net, man, man.now(), '\x02', true);
			}
			else if ((key_released(SDL_SCANCODE_S) && !key_held(SDL_SCANCODE_W)) || (key_released(SDL_SCANCODE_W) && !key_held(SDL_SCANCODE_S)))
			{
				process_event_pack(net, man, man.now(), '\x03', true);
			}

			if (key_pressed(SDL_SCANCODE_UP) || (key_released(SDL_SCANCODE_DOWN) && key_held(SDL_SCANCODE_UP)))
			{
				process_event_pack(net, man, man.now(), '\x10', true);
			}
			else if (key_pressed(SDL_SCANCODE_DOWN) || (key_released(SDL_SCANCODE_UP) && key_held(SDL_SCANCODE_DOWN)))
			{
				process_event_pack(net, man, man.now(), '\x20', true);
			}
			else if ((key_released(SDL_SCANCODE_DOWN) && !key_held(SDL_SCANCODE_UP)) || (key_released(SDL_SCANCODE_UP) && !key_held(SDL_SCANCODE_DOWN)))
			{
				process_event_pack(net, man, man.now(), '\x30', true);
			}

			net.handle_buffer();

			man.snapshot_now();

			pencil(COLOR_WHITE);
			int x1 = man.current_actor_field<int>(player1, posX);
			int y1 = man.current_actor_field<int>(player1, posY);
			draw_rect({ { x1-20,y1 },{10,90} });
			int x2 = man.current_actor_field<int>(player2, posX);
			int y2 = man.current_actor_field<int>(player2, posY);
			draw_rect({ { x2+10,y2 },{10,90} });

			int x3 = man.current_actor_field<int>(ball, posX);
			int y3 = man.current_actor_field<int>(ball, posY);

			last_ball_pos.push_back({ x3,y3 });

			if (last_ball_pos.size() > 100)
				last_ball_pos.erase(last_ball_pos.begin());

			/*for (auto& lb : last_ball_pos)
			{
				pencil(rainbow(100));
				draw_circle(lb, 10);
			}*/

			pencil(rainbow(100));
			draw_circle({ x3,y3 }, 10);

			if (key_pressed(SDL_SCANCODE_SPACE))
			{
				man.set_start();
			}

			/*pencil(COLOR_WHITE);
			draw_line({ 400, 0 }, { 400, 200 });

			size_t i = 0;
			for (auto& a : man.actors)
			{
				for(auto& t : a.timelines)
				{
					for (auto& e : t.second.events)
					{
						int x = ((int)e.second.start_time - (int)man.now()) + 400;
						pencil(e.second.regenerate ? COLOR_GREEN : COLOR_CYAN);
						draw_line({ x, (int)i * 15 }, { x, (int)i * 15 + 15 });

						for (auto& m : e.second.modifiers)
						{
							pencil(COLOR_PINK);
							draw_line({ x + (int)m.first, (int)i * 15 }, { x + (int)m.first, (int)i * 15 + 10 });
						}
					}
				}
				i++;
			}*/
		}
	}
}

namespace FIGHT
{
	GLUU_IMPORT_MAIN(main_fight);
}
