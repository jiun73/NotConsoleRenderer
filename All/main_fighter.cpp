#include "pch.h"
#include "RAS.h"
#include "Defines.h"
#include "CustomNet.h"
#include "CollisionSystem.h"
#include "SoundSystem.h"
#include "AnimationX.h"

/*
* I don't really know what to add to make this more interesting, i mean, at the end of the day it's still pong
* I want to make a 2d racing game with this engine, it's really pushing it but i feel it could work 
* and it'd we a good step towards a fighting game (that's really more complex)
* 
* I should finish this first though. DONE> Maybe adding sound effects will make the game more interesting (DONE> and somewhat better graphics)
* I feel like if there's was more cooldown in between attacks, and more mechanics it could actually work
* 
* like maybe the other guns have more interesting mechanics
* - out of bounds does no damage
* - maybe you can only bounce bullets by taking damage, but if you are protecting (cooldown) it bounces
* - normal gun can be aimed at more orientations and has normal cooldown
* - ak shoots burts of bullets
* - bazooka shoot a single, slow bullet that can make others explode
* - talkie walkie removes all of you bullets allowing you to shoot more
* - that's pretty much the minimum i can do to make the game actually fun somewhat
*/

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

			if (input_pressed("up") || (input_released("down") && input_held("up")))
			{
				process_event_pack(net, man, man.now(), '\x01', is_p1,  true);
			}
			else if (input_pressed("down") || (input_released("up") && input_held("down")))
			{
				process_event_pack(net, man, man.now(), '\x02', is_p1, true);
			}
			else if ((input_released("down") && !input_held("up")) || (input_released("up") && !input_held("up")))
			{
				process_event_pack(net, man, man.now(), '\x03', is_p1, true);
			}

			if(input_pressed("gun_1"))
				process_event_pack(net, man, man.now(), '\x1F', is_p1, true);

			if (input_pressed("gun_2"))
				process_event_pack(net, man, man.now(), '\x2F', is_p1, true);

			if (input_pressed("shield"))
				process_event_pack(net, man, man.now(), '\x3F', is_p1, true);

			if (input_pressed("recall"))
				process_event_pack(net, man, man.now(), '\x4F', is_p1, true);

			if (input_pressed("shoot"))
			{
				bool is_active;
				int type;

				if (is_p1)
					type = man.current_actor_field<int>(PLAYER1, TYPE);
				else
					type = man.current_actor_field<int>(PLAYER2, TYPE);

				char weapon_code;
				bool shoot = true;

				switch (type)
				{
				case GLOCK:
					if(input_held("up"))
						weapon_code = '\x06';
					else if (input_held("down"))
						weapon_code = '\x04';
					else
						weapon_code = '\x07';
					break;
				case AK:
					weapon_code = '\x05';
					break;
				case RECALL:
					process_event_pack(net, man, man.now(), '\x4E', is_p1, true);
					shoot = false;
					break;
				default:
					shoot = false;
					break;
				}

				if (shoot)
				{
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

							char code = (i << 4) | weapon_code;
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
							char code = (i << 4) | weapon_code;
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
		}

		void add_fields() 
		{
			man.register_field<int>(POSX);
			man.register_field<int>(POSY);
			man.register_field<bool>(ACTIVE);
			man.register_field<int>(TYPE);
			man.register_field<int>(HEALTH);
			man.register_field<string>(SOUND);
			man.register_field<int>(STRENGTH);
		}

		void add_systems() 
		{
			man.register_system<CollisionSystem>();
			man.register_system<SoundSystem>();
		}

		/*you can't add generators that take a snapshot into the future because of regenerate from!!!*/

		void add_generators() 
		{
			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {

				return RAS::Event().add_modifier<string>(0ms, [extra](RAS::Time time, string& x) 
					{
						switch (extra)
						{
						case RIZZ:
							x = "Sounds/rizz.wav";
							break;

						case GUN1:
							x = "Sounds/gun1.ogg";
							break;
						case GUN2:
							x = "Sounds/gun2.ogg";
							break;
						case HURT:
							x = "Sounds/hurt.ogg";
							break;
						default:
							break;
						}
					});
				}), PLAY_SOUND);

			RAS::GeneratorID stay = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x });
				}), STAY);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { 100 });
				}), PLAYER_START_HEALTH);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x - 4 });
				}), HEALTH_SUB_PONG);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x - 1 });
				}), HEALTH_SUB_BULLET);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, PLAYER1, POSX);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x + 150 });
				}), POS_TO_PLAYER1X);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, PLAYER1, POSY);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x  + PADDLE_SIZE_Y / 2 + 16});
				}), POS_TO_PLAYER1Y);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, PLAYER2, POSX);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x - 150 });
				}), POS_TO_PLAYER2X);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				double old_x = manager->actor_field_at<int>(time - 1, PLAYER2, POSY);
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { old_x + PADDLE_SIZE_Y / 2 + 16 });
				}), POS_TO_PLAYER2Y);

			RAS::GeneratorID player1_start_posY = man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { 100 });
				}), PLAYER1_START_POSY);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { GLOCK });
				}), SET_TYPE_GLOCK);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { AK });
				}), SET_TYPE_AK);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { SHIELD });
				}), SET_TYPE_SHIELD);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
				return RAS::Event().add_modifier<int, 1>(0ms, point_func<int>, POINT, { RECALL });
				}), SET_TYPE_RECALL);


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
					 return move_bounce(old_x, 1500, 1920, 0, true);
				 }), MOVE_BULLET_X);

			 man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event
				 {
					 double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					 return move_bounce(old_x, 1500, 1920, 0, false);
				 }), MOVE_BULLET_XN);

			 man.register_generator(RAS::Generator([&](GENERATOR_ARGS) -> RAS::Event
				 {
					 double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					 return RAS::Event().add_modifier<int>(0ms, [old_x](RAS::Time time, int& x) 
						 {
							 x = (sin(time / 300.0) * 100.0) + old_x;
						 });
				 }), MOVE_BULLET_Y);

			man.register_generator(RAS::Generator([&](GENERATOR_ARGS) -> RAS::Event
				{
					double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					return move_bounce(old_x, 1200, 1920, 0, true);
				}), MOVE_PONG_X);

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event
				{
					double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);
					return move_bounce(old_x, 1200, 1920, 0, false);
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

			man.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event
				{
					double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);

					RAS::Time time_at_0 = find_linear(0, { (double)extra, -30.0 });

					return RAS::Event().add_modifier<int, 2>(0, linear_func, LINEAR, { (double)extra, -30.0 }).add_modifier<int, 1>(time_at_0, point_func<int>, POINT, { 0 });
				}), SET_STRENGTH);
		}

		void add_actors() 
		{
			man.register_actor(0b0011011, PLAYER1);
			man.register_actor(0b0011011, PLAYER2);
			for (int i = 0; i < MAX_BALLS; i++)
			{
				man.register_actor(0b0001111, BALL_PLAYER1 + i);
			}
			for (int i = 0; i < MAX_BALLS; i++)
			{
				man.register_actor(0b0001111, BALL_PLAYER2 + i);
			}
			man.register_actor(0b0100000, SOUND_MASTER);
			man.register_actor(0b1000000, SCREENSHAKER);
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
			man.add_event_extra(0ms, SCREENSHAKER, SET_STRENGTH, STRENGTH, 1);

			man.add_event(0ms, PLAYER1, PLAYER1_START_POSX, POSX);
			man.add_event(0ms, PLAYER1, PLAYER1_START_POSY, POSY);
			man.add_event(0ms, PLAYER1, SET_TYPE_AK, TYPE);
			man.add_event(0ms, PLAYER1, PLAYER_START_HEALTH, HEALTH);

			man.add_event(0ms, PLAYER2, PLAYER2_START_POSX, POSX);
			man.add_event(0ms, PLAYER2, PLAYER2_START_POSY, POSY);
			man.add_event(0ms, PLAYER2, SET_TYPE_GLOCK, TYPE);
			man.add_event(0ms, PLAYER2, PLAYER_START_HEALTH, HEALTH);

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

		V2d_i shake(V2d_i vec, double force)
		{
			vec.x += (int)random().frange(-force, force);
			vec.y += (int)random().frange(-force, force);
			return vec;
		}

		int draw() 
		{

			
			V2d_i old_cam_offset = camera().offset;

			int shake_s = man.current_actor_field<int>(SCREENSHAKER, STRENGTH);
			camera().offset = shake(camera().offset, shake_s);

			int type1 = man.current_actor_field<int>(PLAYER1, TYPE);
			int type2 = man.current_actor_field<int>(PLAYER2, TYPE);

			pencil(COLOR_PINK);
			int x1 = man.current_actor_field<int>(PLAYER1, POSX);
			int y1 = man.current_actor_field<int>(PLAYER1, POSY);
			if (type1 == SHIELD)
				draw_image("Images/padsr.png", to_game(Rect({ x1 - (int)PADDLE_SIZE_X * 2 + 60,y1 }, { (int)PADDLE_SIZE_X,(int)PADDLE_SIZE_Y })));
			else
				draw_image("Images/padr.png", to_game(Rect({ x1 - (int)PADDLE_SIZE_X * 2 + 60,y1 },{(int)PADDLE_SIZE_X,(int)PADDLE_SIZE_Y} )));
			pencil(COLOR_GREEN);
			int x2 = man.current_actor_field<int>(PLAYER2, POSX);
			int y2 = man.current_actor_field<int>(PLAYER2, POSY);
			if (type2 == SHIELD)
				draw_image("Images/padsl.png", to_game(Rect( { x2 + (int)PADDLE_SIZE_X - 60,y2 },{(int)PADDLE_SIZE_X,(int)PADDLE_SIZE_Y} )));
			else
				draw_image("Images/padl.png", to_game(Rect({ x2 + (int)PADDLE_SIZE_X - 60,y2 }, { (int)PADDLE_SIZE_X,(int)PADDLE_SIZE_Y })));

			int p1h = man.current_actor_field<int>(PLAYER1, HEALTH);
			int p2h = man.current_actor_field<int>(PLAYER2, HEALTH);

			

			string path1 = "Images/" + strings::stringify(type1) + "r.png";
			string path2 = "Images/" + strings::stringify(type2) + "l.png";

			V2d_i size1 = get_image_size(path1);
			V2d_i size2 = get_image_size(path2);

			draw_image(path1, to_game(Rect( shake({ x1 - 10,y1 + PADDLE_SIZE_Y / 2 }, 5.0), size1)));
			draw_image(path2, to_game(Rect( shake({x2 + 10 - size2.x,y2 + PADDLE_SIZE_Y / 2 }, 5.0), size2)));


			pencil(rainbow(100));

			for (int i = 0; i < MAX_BALLS; i++)
			{
				int xball1 = man.current_actor_field<int>(BALL_PLAYER1 + i, POSX);
				int yball1 = man.current_actor_field<int>(BALL_PLAYER1 + i, POSY);

				draw_image("Images/bullet1l.png", to_game(Rect({ xball1 , yball1 }, {(int)(154 * 0.25), (int)(71 * 0.25)})));
				//draw_circle(to_game(V2d_i(xball1, yball1)), 10);
			}
			for (int i = 0; i < MAX_BALLS; i++)
			{
				int xball2 = man.current_actor_field<int>(BALL_PLAYER2 + i, POSX);
				int yball2 = man.current_actor_field<int>(BALL_PLAYER2 + i, POSY);

				draw_image("Images/bullet1l.png", to_game(Rect({ xball2 , yball2 }, { 360 / 3, 62 / 3 })));
				//draw_circle(to_game(V2d_i(xball2,yball2 )), 10);
			}

			 camera().offset = old_cam_offset;
			
			if (p1h < 0) return 1;
			if (p2h < 0) return 2;
			

			draw_text(strings::stringify(man.now()), 1000, to_game(V2d_i(0)), get_font(0));
			draw_text(strings::stringify(p1h), 1000, to_game(V2d_i( 200, 900 )), get_font(0));
			draw_text(strings::stringify(p2h), 1000, to_game(V2d_i(800, 900)), get_font(0));

			return 0;
		}
	};

	

	

	void main_fight() 
	{
		NetPong pong;

		pong.init();
		pong.connection_menu();

		AnimationX bg;
		bg.scale = 4;
		bg.position = -6;
		NCR::File file("Images/fire.anim", NCR::Files::FILE_READING);
		bg.readwrite(file, "fire");
		
		inputs().map("up", { { KEYBOARD_INPUT, SDL_SCANCODE_W }, { KEYBOARD_INPUT, SDL_SCANCODE_UP }, { CONTROLLER_AXISNEGATIVE_INPUT, 1 } });
		inputs().map("down", { { KEYBOARD_INPUT, SDL_SCANCODE_S }, { KEYBOARD_INPUT, SDL_SCANCODE_DOWN }, { CONTROLLER_AXISPOSITIVE_INPUT, 1 } });
		inputs().map("left", { { KEYBOARD_INPUT, SDL_SCANCODE_A }, { KEYBOARD_INPUT, SDL_SCANCODE_LEFT }, { CONTROLLER_AXISNEGATIVE_INPUT, 0 } });
		inputs().map("right", { { KEYBOARD_INPUT, SDL_SCANCODE_D }, {KEYBOARD_INPUT, SDL_SCANCODE_RIGHT}, { CONTROLLER_AXISPOSITIVE_INPUT, 0 } });

		inputs().map("shoot", { { KEYBOARD_INPUT, SDL_SCANCODE_SPACE }, { CONTROLLER_INPUT, SDL_CONTROLLER_BUTTON_A } });
		inputs().map("gun_1", { KEYBOARD_INPUT, SDL_SCANCODE_1 });
		inputs().map("gun_2", { KEYBOARD_INPUT, SDL_SCANCODE_2 });
		inputs().map("shield", { KEYBOARD_INPUT, SDL_SCANCODE_3 });
		inputs().map("recall", { KEYBOARD_INPUT, SDL_SCANCODE_4 });

		set_window_logical_rescaling(false);
		set_logical_size({ 1920,1080 });

		pong.setup_game();

		int result = 0;

		//sound().playMusic("Sounds/ff7boss.ogg");

		

		while (run())
		{


			pong.snapshot();
			pong.handle_inputs();
			pong.snapshot();
			pencil(COLOR_BLACK);
			draw_clear();
			bg.render(get_sdl_ren());
			result = pong.draw();

			if (result != 0) break;
			

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

		while (run())
		{
			pencil(COLOR_BLACK);
			draw_clear();

			if (key_pressed(SDL_SCANCODE_ESCAPE))
				break;

			if (result == 1)
			{
				draw_text("player 1 wins (right)", 1000, {0, 0}, get_font(0));
			}
			else if (result == 2)
			{
				draw_text("player 2 wins (left)", 1000, { 0, 0 }, get_font(0));
			}
		}

		set_window_logical_rescaling(true);
	}
}

namespace FIGHT
{
	GLUU_IMPORT_MAIN(main_fight);
}
