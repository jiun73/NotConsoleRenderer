#include "pch.h"
#include "RAS_Actor.h"
#include "AnimationX.h"

namespace FIGHT
{
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

	struct AnimationHandler 
	{
		AnimationManager manager;

		void apply(TimeMs time, const Modifier& event, size_t& frame, size_t animation, size_t setid)
		{
			frame = manager.get_frame_for_animation(event.time_from(time), animation, setid);
		}
	};

	void main_fight() 
	{
		set_window_size({200,200});
		set_window_resizable();

		TimeManager man;

		size_t posXField = man.register_field<int>("posX");
		size_t posYField = man.register_field<int>("posY");
		size_t sizeXField = man.register_field<int>("sizeX");
		size_t sizeYField = man.register_field<int>("sizeY");
		size_t animationField = man.register_field<int>("anim");

		//std::cout << "int handler id: " << man.register_handler<IntHandler, V2d_i, V2d_i, size_t>() << std::endl;
		size_t linear = man.register_handler<LinearHandler, int, int, int>();
		size_t point = man.register_handler<PointHandler, int, int>();
		size_t quad = man.register_handler<QuadHandler, int, int, int, int, int>();
		size_t move = man.register_handler<MoveHandler, int, int>();
		std::cout << "int actor id: " << man.make_actor(0b1111) << std::endl;
		std::cout << "int actor id: " << man.make_actor(0b1111) << std::endl;

		man.add_event_to_actor(posXField, 0, EventSequence(0, man.make_event<int>(point, 100)));
		man.add_event_to_actor(posYField, 0, EventSequence(0, man.make_event<int>(point, 100)));
		man.add_event_to_actor(sizeXField, 0, EventSequence(0, man.make_event<int>(point, 10)));
		man.add_event_to_actor(sizeYField, 0, EventSequence(0, man.make_event<int>(point, 90)));

		man.add_event_to_actor(posXField, 1, EventSequence(0, man.make_event<int>(point, 1000)));
		man.add_event_to_actor(posYField, 1, EventSequence(0, man.make_event<int>(point, 100)));
		man.add_event_to_actor(sizeXField, 1, EventSequence(0, man.make_event<int>(point, 10)));
		man.add_event_to_actor(sizeYField, 1, EventSequence(0, man.make_event<int>(point, 90)));

		man.start(SDL_GetTicks());

		while (run())
		{
			pencil(COLOR_BLACK);
			draw_clear();

			size_t global_now = SDL_GetTicks();
			size_t now = man.now(global_now);

			man.snapshot_now(global_now);

			int x = man.get_actor_field<int>(0, posXField);
			int y = man.get_actor_field<int>(0, posYField);
			int szx = man.get_actor_field<int>(0, sizeXField);
			int szy = man.get_actor_field<int>(0, sizeYField);

			int x2 = man.get_actor_field<int>(1, posXField);
			int y2 = man.get_actor_field<int>(1, posYField);
			int szx2 = man.get_actor_field<int>(1, sizeXField);
			int szy2 = man.get_actor_field<int>(1, sizeYField);

			if (key_pressed(SDL_SCANCODE_W) || (key_released(SDL_SCANCODE_S) && key_held(SDL_SCANCODE_W)))
			{
				man.add_event_to_actor(posYField, 0, EventSequence(now, man.make_event<int>(move, -100)));
			}
			else if(key_pressed(SDL_SCANCODE_S) || (key_released(SDL_SCANCODE_W) && key_held(SDL_SCANCODE_S)))
			{
				man.add_event_to_actor(posYField, 0, EventSequence(now, man.make_event<int>(move, 100)));
			}
			else if ((key_released(SDL_SCANCODE_S) && !key_held(SDL_SCANCODE_W)) || (key_released(SDL_SCANCODE_W) && !key_held(SDL_SCANCODE_S)))
			{
				man.add_event_to_actor(posYField, 0, EventSequence(now, man.make_event<int>(point, y)));
			}

			/*if (key_pressed(SDL_SCANCODE_SPACE))
			{
				EventSequence seq;

				seq.add_event(now, man.make_event<int, int, int>(quad, now, now + 1000, 100, y));
				seq.add_event(now + 1000, man.make_event<int>(point, 100));
				man.add_event_to_actor(posYField, 0, seq);
			}*/

			if (key_pressed(SDL_SCANCODE_0))
			{
				man.start(global_now);
			}

			pencil(COLOR_WHITE);
			draw_full_rect({ { x,y }, {szx,szy} });
			draw_full_rect({ { x2,y2 }, {szx2,szy2} });

			size_t i = 0;
			for (auto a : man.get_actors())
			{
				for (auto e : a.timeline.get_events())
				{
					for (auto ee : e.second)
					{
						draw_line({ (int)now - (int)ee.first, (int)i * 10 }, { (int)now - (int)ee.first, ((int)i * 10) + 10 });
					}
					i++;
				}
				
			}

			std::cout << x << "," << y << std::endl;
		}
	}
}

namespace FIGHT
{
	GLUU_IMPORT_MAIN(main_fight);
}
