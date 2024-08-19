#include "NotConsoleRenderer.h" 
#include "Triggers.h"

namespace FF
{
	class Game
	{
	private:
		RAS::Manager man;

	public:
		void setup()
		{
			add_field(man);
			add_generators(man);
			add_actors(man);
			add_triggers(man);

			man.set_net(HOST, [](RAS::Time, RAS::TriggerID) {});

			man.add_event_extra(0, PLAYER1, SET_STATIC, MVTX, {100});
			man.add_event_extra(0, PLAYER1, SET_STATIC, MVTY, {100});

			man.add_event_extra(1, PLAYER1, SET_VELOCITY, MVTX, { 10 });	

			man.set_time_fetcher(SDL_GetTicks);
			man.set_start();
		}

		void handle_inputs() 
		{
			if (input_pressed("up") || (input_released("down") && input_held("up")))
			{
				man.trigger_now(PLYR1_MOVE_UP);
			}
			else if (input_pressed("down") || (input_released("up") && input_held("down")))
			{
				man.trigger_now(PLYR1_MOVE_DOWN);
			}
			else if ((input_released("down") && !input_held("up")) || (input_released("up") && !input_held("up")))
			{
				man.trigger_now(PLYR1_STOPY);
			}

			if (input_pressed("left") || (input_released("right") && input_held("left")))
			{
				man.trigger_now(PLYR1_MOVE_LEFT);
			}
			else if (input_pressed("right") || (input_released("left") && input_held("right")))
			{
				man.trigger_now(PLYR1_MOVE_RIGHT);
			}
			else if ((input_released("right") && !input_held("left")) || (input_released("left") && !input_held("left")))
			{
				man.trigger_now(PLYR1_STOPX);
			}
		}

		void run()
		{
			handle_inputs();
			man.snapshot_now();	
			pencil(COLOR_WHITE);
			Mvt curx = man.current_actor_field<Mvt>(PLAYER1, MVTX);
			Mvt cury = man.current_actor_field<Mvt>(PLAYER1, MVTY);
			draw_rect({ {(int)(curx.pos) , (int)(cury.pos)}, {10,10}});
		}
	};
	
}

int main(int argc, char* args[])
{
	FF::Game game;

	inputs().map("up", { { KEYBOARD_INPUT, SDL_SCANCODE_W }, { KEYBOARD_INPUT, SDL_SCANCODE_UP }, { CONTROLLER_AXISNEGATIVE_INPUT, 1 } });
	inputs().map("down", { { KEYBOARD_INPUT, SDL_SCANCODE_S }, { KEYBOARD_INPUT, SDL_SCANCODE_DOWN }, { CONTROLLER_AXISPOSITIVE_INPUT, 1 } });
	inputs().map("left", { { KEYBOARD_INPUT, SDL_SCANCODE_A }, { KEYBOARD_INPUT, SDL_SCANCODE_LEFT }, { CONTROLLER_AXISNEGATIVE_INPUT, 0 } });
	inputs().map("right", { { KEYBOARD_INPUT, SDL_SCANCODE_D }, {KEYBOARD_INPUT, SDL_SCANCODE_RIGHT}, { CONTROLLER_AXISPOSITIVE_INPUT, 0 } });

	set_window_resizable();
	set_window_logical_rescaling(false);
	set_logical_size({ 1920,1080 });

	game.setup();

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();

		game.run();
	}

	return 0;
}