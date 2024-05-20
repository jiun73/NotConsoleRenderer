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

	struct CollisionSystem 
	{
		void update(RAS::Manager* manager, RAS::Time time, vector<RAS::Actor>& actors)
		{

		}
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

	void main_fight() 
	{
		set_window_size({200,200});
		set_window_resizable();

		RAS::Manager man;

		man.set_time_fetcher(SDL_GetTicks);

		RAS::FieldID posX = man.register_field<int>();
		RAS::FieldID posY = man.register_field<int>();

		man.register_system<CollisionSystem>();

		RAS::ActorID player1 = man.register_actor(0b11);

		RAS::Generator gen;

		gen.generate = [](RAS::Time time, RAS::Manager* manager, RAS::FieldID generator_field, RAS::ActorID actor) -> RAS::Event
			{
				RAS::Event event;

				RAS::ModifierPureType<int, 1>* mod = new RAS::ModifierPureType<int, 1>();

				mod->params = { 110 };
				mod->mod_func = point_func;

				event.modifiers.emplace(0ms, mod);
				return event;
			};

		RAS::GeneratorID point_100 = man.register_generator(gen);

		gen.generate = [](RAS::Time time, RAS::Manager* manager, RAS::FieldID generator_field, RAS::ActorID actor) -> RAS::Event
			{
				RAS::Event event;

				RAS::ModifierPureType<int, 2>* mod = new RAS::ModifierPureType<int, 2>();

				double old_x = manager->actor_field_at<int>(time - 1, actor, generator_field);

				mod->params = { old_x, 100 };
				mod->mod_func = linear_func;

				event.modifiers.emplace(0ms, mod);
				return event;
			};

		RAS::GeneratorID move_speed_100 = man.register_generator(gen);

		man.set_start();

		man.add_event(0ms, player1, point_100, posX);
		man.add_event(0ms, player1, point_100, posY);

		while (run())
		{
			pencil(COLOR_BLACK);
			draw_clear();
			man.snapshot_now();

			int x = man.current_actor_field<int>(player1, posX);
			int y = man.current_actor_field<int>(player1, posY);

			std::cout << x << " " << y << std::endl;
		}
	}
}

namespace FIGHT
{
	GLUU_IMPORT_MAIN(main_fight);
}
