#include "pch.h"
#include "RAS_Actor.h"
#include "AnimationX.h"

namespace FIGHT
{
	RAS::Time operator ""ms(RAS::Time ms)
	{
		return ms;
	}

	RAS::Time operator ""s(RAS::Time s)
	{
		return s * 1000;
	}

	RAS::Time operator ""s(long double s)
	{
		return s * 1000;
	}

	//struct PointHandler
	//{
	//	void apply(TimeMs time, const Modifier& event, int& i, const int c)
	//	{
	//		i = c;
	//	}
	//};

	//struct LinearHandler
	//{
	//	//speed is movement per 1000 ms
	//	void apply(TimeMs time, const Modifier& event, int& i, const int start, const int speed)
	//	{
	//		i = ((event.time_from(time) / 1000.0) * (double)speed) + start;
	//	}
	//};

	////Like linear handler, but start from the value right before the event
	//struct MoveHandler
	//{
	//	//speed is movement per 1000 ms
	//	void apply(TimeMs time, const Modifier& event, int& i, const int speed)
	//	{
	//		int start = event.manager->get_actor_field_at<int>(event.time - 1, event.actorid, event.fieldbyteid); //get the value right before the event happened
	//		i = ((event.time_from(time) / 1000.0) * (double)speed) + start;
	//	}
	//};

	//struct QuadHandler
	//{
	//	//speed is movement per 1000 ms
	//	void apply(TimeMs time, const Modifier& event, int& i, int p1, int p2, int slope, int start)
	//	{
	//		double x = (event.time_from(time) / 1000.0);
	//		double x1 = (event.time_from(p1) / 1000);
	//		double x2 = (event.time_from(p2) / 1000);

	//		i = (double)slope * (x - x1) * (x - x2);
	//		i += start;
	//	}
	//};

	/*struct CollisionSystem 
	{
		void update(TimeMs time, int fielddata1, int fielddata2)
		{

		}
	};*/

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

	/*struct AnimationHandler 
	{
		AnimationManager manager;

		void apply(TimeMs time, const Modifier& event, size_t& frame, size_t animation, size_t setid)
		{
			frame = manager.get_frame_for_animation(event.time_from(time), animation, setid);
		}
	};*/

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
		//bool x_collision_found = false;
		//RAS::Time collision_time = 0;

		//bool find_collision(RAS::Manager* manager, RAS::Time time, vector<RAS::Actor>& actors, RAS::ActorID ball, RAS::ActorID player, RAS::GeneratorID genif)
		//{
		//	auto& events_ball = actors.at(ball).timelines.at(0).events;
		//	auto& events_p1 = actors.at(player).timelines.at(0).events;
		//	auto ball_it = events_ball.lower_bound(time);
		//	auto player1_it = events_p1.lower_bound(time);

		//	if (ball_it != events_ball.begin()) ball_it--;
		//	if (player1_it != events_p1.begin()) player1_it--;

		//	while (ball_it != events_ball.end() && player1_it != events_p1.end())
		//	{
		//		RAS::Event& ball_event = ball_it->second;
		//		RAS::Event& p1_event = player1_it->second;

		//		RAS::Time next_event_time = 0;

		//		ball_it++;

		//		if (ball_it == events_ball.end())
		//		{
		//			next_event_time = -1;
		//		}
		//		else
		//		{
		//			next_event_time = ball_it->first;
		//		}

		//		ball_it--;

		//		/*check for collision*/
		//		assert(p1_event.modifiers.size() == 1); //we assume the x position of the paddle doesn't change
		//		RAS::Modifier* mod_p1 = p1_event.modifiers.at(0);
		//		assert(mod_p1->is_reversible() && mod_p1->reverse_type() == POINT);
		//		int p1_x = mod_p1->reverse_params().at(0);
		//		
		//		for (auto it_mod = ball_event.modifiers.begin(); it_mod != ball_event.modifiers.end(); it_mod++)
		//		{
		//			assert(it_mod->second->is_reversible());
		//			auto params = it_mod->second->reverse_params();

		//			it_mod++;

		//			RAS::Time next_mod_time = 0;

		//			if (it_mod == ball_event.modifiers.end())
		//			{
		//				next_mod_time = next_event_time;
		//			}
		//			else
		//			{
		//				next_mod_time = ball_event.start_time + it_mod->first;
		//			}

		//			it_mod--;

		//			if (it_mod->first > next_event_time)
		//				break;

		//			switch (it_mod->second->reverse_type())
		//			{
		//			case POINT:
		//				x_collision_found = (p1_x == params.at(0));
		//				collision_time = ball_event.start_time + it_mod->first;
		//				break;
		//			case LINEAR:
		//			{
		//				RAS::Time potential_time = ball_event.start_time + it_mod->first + find_linear(p1_x, { params.at(0), params.at(1) });

		//				x_collision_found = (potential_time >= ball_event.start_time + it_mod->first && potential_time <= next_mod_time);

		//				collision_time = potential_time;
		//			}
		//			break;
		//			default:
		//				assert(false);
		//				break;
		//			}

		//			if (x_collision_found) break;
		//		}

		//		if (x_collision_found)
		//		{
		//			int p1_y = manager->actor_field_at<int>(collision_time, player, 1); // get the y coord of the player
		//			int ball_y = manager->actor_field_at<int>(collision_time, ball, 1);

		//			if ((ball_y >= p1_y) && (ball_y <= (p1_y + 90)))
		//			{
		//				actual collision found
		//				std::cout << "FUCKING COLLISION!" << player << std::endl;
		//				
		//				return true;
		//			}
		//			else
		//			{
		//				std::cout << "nope" << std::endl;
		//			}
		//		}

		//		ball_it++;
		//		player1_it++;

		//		if (ball_it == events_ball.end() && player1_it != events_p1.end())
		//		{
		//			ball_it--;
		//			time = player1_it->first;
		//		}
		//		else if (ball_it != events_ball.end() && player1_it == events_p1.end())
		//		{
		//			player1_it--;
		//			time = ball_it->first;
		//		}
		//		else if (ball_it != events_ball.end() && player1_it != events_p1.end())
		//		{
		//			RAS::Time time_ball = ball_it->first;
		//			RAS::Time time_player1 = player1_it->first;
		//			if (time_ball > time_player1) { ball_it--; time = time_player1; }
		//			else if (time_ball < time_player1) { player1_it--; time = time_ball; }
		//		}
		//	}
		//	return false;
		//}

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

				return (potential_time >= start_time && potential_time >= mod1_time && potential_time <= next_time);
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

			map<RAS::Time, RAS::Modifier*>::iterator event1_mod = event1.modifiers.lower_bound(start_time);
			map<RAS::Time, RAS::Modifier*>::iterator event2_mod = event2.modifiers.lower_bound(start_time);

			if (event1_mod != event1.modifiers.begin()) { event1_mod--; }
			if (event2_mod != event2.modifiers.begin()) { event2_mod--; }

			while (true)
			{
				RAS::Time next_time = get_next_event_time<map<RAS::Time, RAS::Modifier*>>(event1_mod, event2_mod, event1.modifiers.end(), event2.modifiers.end(), event1.start_time, event2.start_time);

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
				check_resume = next_event_time;

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

		void update(RAS::Manager* arg_manager, RAS::Time arg_time, vector<RAS::Actor>& arg_actors)
		{
			actors = &arg_actors;
			manager = arg_manager;
			time = arg_time;

			RAS::Time checking_time = time;

			RAS::ActorID ball = 2;
			RAS::ActorID p1 = 0;
			RAS::ActorID p2 = 1;

			while (true)
			{
				bool col_p1 = find_collision(checking_time, ball, p1);
				RAS::Time collision_time_p1 = collision_time;
				bool continue_1 = continue_detection;
				RAS::Time resume_check1 = check_resume;

				bool col_p2 = find_collision(checking_time, ball, p2);
				RAS::Time collision_time_p2 = collision_time;
				bool continue_2 = continue_detection;
				RAS::Time resume_check2 = check_resume;

				if (col_p1 && col_p2)
				{
					if (collision_time_p1 > collision_time_p2)
					{
						col_p1 = false;
					}
					else
					{
						col_p2 = false;
					}
				}


				if (col_p1)
				{
					int ballY = manager->actor_field_at<int>(collision_time_p1, ball, POSY);
					int playerY = manager->actor_field_at<int>(collision_time_p1, p1, POSY);

					if (ballY >= playerY && ballY <= playerY + 90)
					{
						manager->add_event(collision_time, ball, 6, POSX, true);
					}

					checking_time = std::max(checking_time, std::max(resume_check1, collision_time_p1));
				}

				if (col_p2)
				{
					int ballY = manager->actor_field_at<int>(collision_time_p2, ball, POSY);
					int playerY = manager->actor_field_at<int>(collision_time_p2, p2, POSY);

					if (ballY >= playerY && ballY <= playerY + 90)
					{
						manager->add_event(collision_time, ball, 5, POSX, true);
					}

					checking_time = std::max(checking_time, std::max(resume_check2, collision_time_p2));
				}

				if (col_p1 || col_p2)
				{
					continue;
				}

				break;
			}
			
			//resolve_collision(2, 1, 6, POSX);
			//resolve_collision(2, 0, 5, POSX);

			/*RAS::Timeline& ball_X_timeline = actors->at(2).timelines.at(POSX);
			RAS::Timeline& player_X_timeline = actors->at(0).timelines.at(POSX);

			if (ball_X_timeline.events.empty() || player_X_timeline.events.empty()) return;

			map<RAS::Time, RAS::Event>::iterator ball_it = get_event_from_timeline(arg_time, ball_X_timeline);
			map<RAS::Time, RAS::Event>::iterator player_it = get_event_from_timeline(arg_time, player_X_timeline);

			RAS::Time regen_time = std::min(ball_it->first, player_it->first);*/

			//manager->regenerate_from(regen_time);

			
		}
	};

	

	void main_fight() 
	{
		set_window_size({200,200});
		set_window_resizable();

		RAS::Manager man;

		man.set_time_fetcher(SDL_GetTicks);

		RAS::FieldID posX = man.register_field<int>();
		RAS::FieldID posY = man.register_field<int>();

		man.register_system<CollisionSystem>();

		RAS::GeneratorID stay = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func, POINT, { old_x });
			}));

		RAS::GeneratorID player_start_posY = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			return RAS::Event().add_modifier<int, 1>(0ms, point_func, POINT, { 100 });
			}));

		RAS::GeneratorID player1_start_posX = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func, POINT, { 100 });
			}));

		RAS::GeneratorID player2_start_posX = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event{
				return RAS::Event().add_modifier<int, 1>(0ms, point_func, POINT, { 900 });
			}));

		RAS::GeneratorID ball_start_pos = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			return RAS::Event().add_modifier<int, 1>(0ms, point_func, POINT, { 500 });
			}));

		RAS::GeneratorID move_speed_100 = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event
			{
				double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
				RAS::Time bounce_time = find_linear(1000, { old_x, 300 });

				return RAS::Event().add_modifier<int, 2>(0ms, linear_func, LINEAR, { old_x, 300 }).add_modifier<int, 2>(bounce_time, linear_func, LINEAR, { 1000, -300 });
			}));

		RAS::GeneratorID move_speed_n100 = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event
			{
				double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
				RAS::Time bounce_time = find_linear(0, { old_x, -300 });
				return RAS::Event().add_modifier<int, 2>(0ms, linear_func, LINEAR, { old_x, -300 }).add_modifier<int, 2>(bounce_time, linear_func, LINEAR, { 0, 300 });
			}));

		man.set_start();

		RAS::ActorID player1 = man.register_actor(0b11);
		RAS::ActorID player2 = man.register_actor(0b11);
		RAS::ActorID ball = man.register_actor(0b11);


		man.add_event(0ms, player1, player1_start_posX, posX);
		man.add_event(0ms, player1, player_start_posY, posY);
		man.add_event(0ms, player2, player2_start_posX, posX);
		man.add_event(0ms, player2, player_start_posY, posY);
		man.add_event(0ms, ball, ball_start_pos, posX);
		man.add_event(0ms, ball, ball_start_pos, posY);
		man.add_event(1000ms, ball, move_speed_n100, posX);
		man.add_event(1000ms, ball, move_speed_n100, posY);

		while (run())
		{
			pencil(COLOR_BLACK);
			draw_clear();
			

			if (key_pressed(SDL_SCANCODE_W) || (key_released(SDL_SCANCODE_S) && key_held(SDL_SCANCODE_W)))
			{
				man.add_event(man.now(), player1, move_speed_n100, posY);
			}
			else if (key_pressed(SDL_SCANCODE_S) || (key_released(SDL_SCANCODE_W) && key_held(SDL_SCANCODE_S)))
			{
				man.add_event(man.now(), player1, move_speed_100, posY);
			}
			else if ((key_released(SDL_SCANCODE_S) && !key_held(SDL_SCANCODE_W)) || (key_released(SDL_SCANCODE_W) && !key_held(SDL_SCANCODE_S)))
			{
				man.add_event(man.now(), player1, stay, posY);
			}

			if (key_pressed(SDL_SCANCODE_UP) || (key_released(SDL_SCANCODE_DOWN) && key_held(SDL_SCANCODE_UP)))
			{
				man.add_event(man.now(), player2, move_speed_n100, posY);
			}
			else if (key_pressed(SDL_SCANCODE_DOWN) || (key_released(SDL_SCANCODE_UP) && key_held(SDL_SCANCODE_DOWN)))
			{
				man.add_event(man.now(), player2, move_speed_100, posY);
			}
			else if ((key_released(SDL_SCANCODE_DOWN) && !key_held(SDL_SCANCODE_UP)) || (key_released(SDL_SCANCODE_UP) && !key_held(SDL_SCANCODE_DOWN)))
			{
				man.add_event(man.now(), player2, stay, posY);
			}

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
			draw_circle({ x3,y3 }, 10);


			if (key_pressed(SDL_SCANCODE_SPACE))
			{
				man.set_start();
			}
		}
	}
}

namespace FIGHT
{
	GLUU_IMPORT_MAIN(main_fight);
}
