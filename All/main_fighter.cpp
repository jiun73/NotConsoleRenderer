#include "pch.h"
#include "Timeline.h"

namespace FIGHT
{
	struct PointHandler
	{
		void apply(TimeMs time, const Event& event, int& i, const int c)
		{
			i = c;
		}
	};

	struct LinearHandler
	{
		//speed is movement per 1000 ms
		void apply(TimeMs time, const Event& event, int& i, const int start, const int speed)
		{
			i = ((event.time_from(time) / 1000.0) * (double)speed) + start;
		}
	};

	void main_fight() 
	{
		set_window_size({200,200});
		set_window_resizable();

		TimeManager man;

		size_t posXField = man.register_field<int>("posX");
		size_t posYField = man.register_field<int>("posY");
		size_t sizeField = man.register_field<V2d_i>("size");
		size_t animationField = man.register_field<int>("anim");

		//std::cout << "int handler id: " << man.register_handler<IntHandler, V2d_i, V2d_i, size_t>() << std::endl;
		size_t linear = man.register_handler<LinearHandler, int, int, int>();
		size_t point = man.register_handler<PointHandler, int, int>();
		std::cout << "int actor id: " << man.make_actor(0b1111) << std::endl;

		man.add_event_to_actor(posXField, 0, EventSequence(0, man.make_event<int>(point, 100)));
		man.add_event_to_actor(posYField, 0, EventSequence(0, man.make_event<int>(point, 100)));

		man.start(SDL_GetTicks());

		while (run())
		{
			pencil(COLOR_BLACK);
			draw_clear();

			man.snapshot_now(SDL_GetTicks());

			int x = man.get_actor_field<int>(0, posXField);
			int y = man.get_actor_field<int>(0, posYField);

			if (key_pressed(SDL_SCANCODE_D))
			{
				std::cout << "right" << std::endl;
				man.add_event_to_actor(posXField, 0, EventSequence(man.now(SDL_GetTicks()), man.make_event<int, int>(linear, x, 50)));
			}
			else if(key_pressed(SDL_SCANCODE_A))
			{
				std::cout << "left" << std::endl;
				man.add_event_to_actor(posXField, 0, EventSequence(man.now(SDL_GetTicks()), man.make_event<int, int>(linear, x, -50)));
			}
			

			
			if (key_released(SDL_SCANCODE_A) || key_released(SDL_SCANCODE_D))
			{
				std::cout << "stop e" << std::endl;
				man.add_event_to_actor(posXField, 0, EventSequence(man.now(SDL_GetTicks()), man.make_event<int>(point, x)));
			}

			pencil(COLOR_WHITE);
			draw_circle({ x,y }, 5);

			size_t i = 0;
			for (auto a : man.get_actors())
			{
				for (auto e : a.timeline.get_events())
				{
					for (auto ee : e.second)
					{

					}
				}
				i++;
			}

			std::cout << x << "," << y << std::endl;
		}
	}
}

namespace FIGHT
{
	GLUU_IMPORT_MAIN(main_fight);
}
