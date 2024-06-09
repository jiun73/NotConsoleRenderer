#include "pch.h"
#include "RAS.h"
#include "Defines.h"
#include "CustomNet.h"
#include "CollisionSystem.h"
#include "AnimationX.h"



namespace FIGHT
{
	RAS::Event move_bounce(double start, double speed, double max, double min, bool dir, int depth = 10)
	{
		RAS::Event e;
		RAS::Time bounce_time = 0;
		bool dirmax = dir;
		e.add_modifier<int, 2>(0ms, linear_func, LINEAR, { start, dirmax ? speed : -speed });
		bounce_time += find_linear(dirmax ? max : min, { start, dirmax ? speed : -speed });
		for (int i = 0; i < depth; i++)
		{
			e.add_modifier<int, 2>(bounce_time, linear_func, LINEAR, { dirmax ? max : min, dirmax ? -speed : speed });
			dirmax = !dirmax;
			bounce_time += find_linear(dirmax ? max : min, { !dirmax ? max : min, dirmax ? speed : -speed });
		}
		return e;
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

	class NetPong 
	{
		CustomNet net;
		RAS::Manager man;

	public:
		void connection_menu()
		{
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
		}

		void handle_inputs()
		{
			bool is_p1 = net.hosting();

			if (key_pressed(SDL_SCANCODE_W) || (key_released(SDL_SCANCODE_S) && key_held(SDL_SCANCODE_W)))
			{
				process_event_pack(net, man, man.now(), '\x01', is_p1,  true);
			}
			else if (key_pressed(SDL_SCANCODE_S) || (key_released(SDL_SCANCODE_W) && key_held(SDL_SCANCODE_S)))
			{
				process_event_pack(net, man, man.now(), '\x02', is_p1, true);
			}
			else if ((key_released(SDL_SCANCODE_S) && !key_held(SDL_SCANCODE_W)) || (key_released(SDL_SCANCODE_W) && !key_held(SDL_SCANCODE_S)))
			{
				process_event_pack(net, man, man.now(), '\x03', is_p1, true);
			}

			if (key_pressed(SDL_SCANCODE_SPACE))
			{
				bool is_active;
				if (is_p1)
				{
					bool found = false;
					int i = 0;
					for (; i < MAX_BALLS; i++)
					{
						found = !man.current_actor_field<bool>(BALL_PLAYER1 + i, ACTIVE);
						if (found)
							break;
					}

					if (found)
					{
						char code = (i << 4) | '\x04';
						std::cout << std::hex << (int)code << std::dec;
						process_event_pack(net, man, man.now(), code, is_p1, true);
					}
					else
					{
						std::cout << "no bullet found" << std::endl;
					}
				}
				else
				{
					bool found = false;
					int i = 0;
					for (; i < MAX_BALLS; i++)
					{
						found = !man.current_actor_field<bool>(BALL_PLAYER2 + i, ACTIVE);
						if (found)
							break;
					}

					if (found)
					{
						char code = (i << 4) | '\x04';
						std::cout << std::hex << (int)code << std::dec;
						process_event_pack(net, man, man.now(), code, is_p1, true);
					}
					else
					{
						std::cout << "no bullet found" << std::endl;
					}

					/*is_active = man.current_actor_field<bool>(BALL_PLAYER2, ACTIVE);

					if (!is_active)
						process_event_pack(net, man, man.now(), '\x04', is_p1, true);*/
				}

				
			}
		}

		void add_fields() 
		{
			man.register_field<int>(POSX);
			man.register_field<int>(POSY);
			man.register_field<bool>(ACTIVE);
			man.register_field<int>(TYPE);
		}

		void add_systems() 
		{
			man.register_system<CollisionSystem>();
		}

		/*you can't add generators that take a snapshot into the future because of regenerate from!!!*/

		void add_generators() 
		{
			RAS::GeneratorID stay = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x });
				}), STAY);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, PLAYER1, POSX);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x });
				}), POS_TO_PLAYER1X);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, PLAYER1, POSY);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x });
				}), POS_TO_PLAYER1Y);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, PLAYER2, POSX);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x });
				}), POS_TO_PLAYER2X);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, PLAYER2, POSY);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x });
				}), POS_TO_PLAYER2Y);

			RAS::GeneratorID player1_start_posY = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { 100 });
				}), PLAYER1_START_POSY);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { 0 });
				}), SET_TYPE_GLOCK);

			RAS::GeneratorID player2_start_posY = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { 100 });
				}), PLAYER2_START_POSY);

			RAS::GeneratorID player1_start_posX = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { 100 });
				}), PLAYER1_START_POSX);

			RAS::GeneratorID player2_start_posX = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { 1820 });
				}), PLAYER2_START_POSX);

			RAS::GeneratorID ball_start_pos = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { 500 });
				}), BALL_START_POS);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { -999 });
				}), BALL_POS_OOB);

			RAS::GeneratorID ball2_start_pos = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { 600 });
				}), BALL2_START_POS);

			 man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<bool, 1>(0ms, point_func<bool>, POINT, { true });
				}), SET_ACTIVE);

			 man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<bool, 1>(0ms, point_func<bool>, POINT, { false });
				}), SET_INACTIVE);

			man.register_generator(RAS::Generator([&](GENERATOR_ARGS) -> RAS::Event
				{
					double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					return move_bounce(old_x, 300, 1920, 0, true);
				}), MOVE_PONG_X);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event
				{
					double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					return move_bounce(old_x, 300, 1920, 0, false);
				}), MOVE_PONG_XN);

			man.register_generator(RAS::Generator([&](GENERATOR_ARGS) -> RAS::Event
				{
					double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					return move_bounce(old_x, 300, 1080, 0, true);
				}), MOVE_PONG_Y);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event
				{
					double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					return move_bounce(old_x, 300, 1080, 0, false);
				}), MOVE_PONG_YN);

			man.register_generator(RAS::Generator([&](GENERATOR_ARGS) -> RAS::Event
				{
					double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					return move_bounce(old_x, 450, 1920, 0, true);
				}), MOVE_PLAYER_X);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event
				{
					double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					return move_bounce(old_x, 450, 1920, 0, false);
				}), MOVE_PLAYER_XN);

			man.register_generator(RAS::Generator([&](GENERATOR_ARGS) -> RAS::Event
				{
					double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					return move_bounce(old_x, 450, 1080, 0, true);
				}), MOVE_PLAYER_Y);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event
				{
					double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					return move_bounce(old_x, 450, 1080, 0, false);
				}), MOVE_PLAYER_YN);
		}

		void add_actors() 
		{
			man.register_actor(0b1011, PLAYER1);
			man.register_actor(0b1011, PLAYER2);
			for (int i = 0; i < MAX_BALLS; i++)
			{
				man.register_actor(0b1111, BALL_PLAYER1 + i);
			}
			for (int i = 0; i < MAX_BALLS; i++)
			{
				man.register_actor(0b1111, BALL_PLAYER2 + i);
			}
		}

		void sync_clock() 
		{

		}

		void init() 
		{
			man.set_time_fetcher(SDL_GetTicks);

			add_systems();
			add_fields();
			add_generators();
			add_actors();
		}

		void setup_game() 
		{
			man.add_event(0ms, PLAYER1, PLAYER1_START_POSX, POSX);
			man.add_event(0ms, PLAYER1, PLAYER1_START_POSY, POSY);
			man.add_event(0ms, PLAYER1, SET_TYPE_GLOCK, TYPE);

			man.add_event(0ms, PLAYER2, PLAYER2_START_POSX, POSX);
			man.add_event(0ms, PLAYER2, PLAYER2_START_POSY, POSY);
			man.add_event(0ms, PLAYER2, SET_TYPE_GLOCK, TYPE);

			for (int i = 0; i < MAX_BALLS; i++)
			{
				man.add_event(0ms, BALL_PLAYER1 + i, BALL_POS_OOB, POSX);
				man.add_event(0ms, BALL_PLAYER1 + i, BALL_POS_OOB, POSY);
				man.add_event(0ms, BALL_PLAYER1 + i, SET_TYPE_GLOCK, TYPE);
				man.add_event(0ms, BALL_PLAYER1 + i, SET_INACTIVE, ACTIVE);
			}

			for (int i = 0; i < MAX_BALLS; i++)
			{
				man.add_event(0ms, BALL_PLAYER2 + i, BALL_POS_OOB, POSX);
				man.add_event(0ms, BALL_PLAYER2 + i, BALL_POS_OOB, POSY);
				man.add_event(0ms, BALL_PLAYER2 + i, SET_TYPE_GLOCK, TYPE);
				man.add_event(0ms, BALL_PLAYER2 + i, SET_INACTIVE, ACTIVE);
			}
		}

		void snapshot() 
		{
			net.handle_buffer();
			man.snapshot_now();
		}

		void draw() 
		{
			pencil(COLOR_BLACK);
			draw_clear();

			pencil(COLOR_PINK);
			int x1 = man.current_actor_field<int>(PLAYER1, POSX);
			int y1 = man.current_actor_field<int>(PLAYER1, POSY);
			draw_full_rect({ { x1 - 20,y1 },{10,90} });
			pencil(COLOR_GREEN);
			int x2 = man.current_actor_field<int>(PLAYER2, POSX);
			int y2 = man.current_actor_field<int>(PLAYER2, POSY);
			draw_full_rect({ { x2 + 10,y2 },{10,90} });


			draw_image("Images/gunr.png", { { x1 - 10,y1 }, {64,41} });
			draw_image("Images/gunl.png", { { x2 + 10 - 64,y2 }, {64,41} });

			pencil(rainbow(100));

			for (int i = 0; i < MAX_BALLS; i++)
			{
				int xball1 = man.current_actor_field<int>(BALL_PLAYER1 + i, POSX);
				int yball1 = man.current_actor_field<int>(BALL_PLAYER1 + i, POSY);

				draw_circle({ xball1,yball1 }, 10);
			}
			for (int i = 0; i < MAX_BALLS; i++)
			{
				int xball2 = man.current_actor_field<int>(BALL_PLAYER2 + i, POSX);
				int yball2 = man.current_actor_field<int>(BALL_PLAYER2 + i, POSY);

				draw_circle({ xball2,yball2 }, 10);
			}

			
			
			

			draw_text(strings::stringify(man.now()), 1000, 0, get_font(0));
		}
	};

	

	

	void main_fight() 
	{
		NetPong pong;

		pong.init();
		pong.connection_menu();
		

		set_window_logical_rescaling(false);
		set_logical_size({ 1920,1080 });

		pong.setup_game();
		while (run())
		{


			pong.snapshot();
			pong.handle_inputs();
			pong.snapshot();
			pong.draw();

			

			/*if (key_pressed(SDL_SCANCODE_SPACE))
			{
				man.set_start();
			}*/

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

		set_window_logical_rescaling(true);
	}
}

namespace FIGHT
{
	GLUU_IMPORT_MAIN(main_fight);
}
