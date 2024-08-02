#include "pch.h"
#include "RAS.h"
#include "Defines.h"
#include "CustomNet.h"
#include "CollisionSystem.h"
#include "SoundSystem.h"
#include "AnimationX.h"
#include "Fields.h"
#include "Actors.h"
#include "Generators.h"

namespace FF
{
	class Game 
	{
	private:
		RAS::Manager man;

		void setup() 
		{
			add_field(man);
			add_actors(man);
			add_generators(man);
		}

	public:
		Game() {}
		~Game() {}

		void run() 
		{
			man.set_time_fetcher(SDL_GetTicks);
			man.set_start();

			while (::run())
			{
				pencil(COLOR_BLACK);
				draw_clear();

				man.snapshot_now();
			}
		}
	};


	void main_ff() 
	{
		Game game;
		game.run();
	}	
}

namespace FF
{
	GLUU_IMPORT_MAIN(main_ff);
}