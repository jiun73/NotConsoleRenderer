#pragma once
#include "RAS.h"
#include "Defines.h"
#include "NotConsoleRenderer.h"
#include <vector>

namespace FIGHT
{
	using std::vector;

	struct SoundSystem
	{
		RAS::Time done_time = 0;
		bool recursion_protection = false;

		void on_snap(RAS::Manager* arg_manager, RAS::Time arg_time, vector<RAS::Actor>& arg_actors)
		{
			if (recursion_protection) return;
			RAS::Timeline& timeline = arg_actors.at(arg_manager->get_actor(SOUND_MASTER)).timelines.at(arg_manager->get_field(SOUND));

			auto event_it = timeline.events.lower_bound(arg_time);

			for (; event_it != timeline.events.end(); event_it++)
			{
				assert(event_it->second.modifiers.size() == 1);

				recursion_protection = true;
				string& sound_path = arg_manager->actor_field_at<string>(event_it->first + 1, SOUND_MASTER, SOUND); //let the manager call snapshot and set the right value
				recursion_protection = false;

				std::cout << "playing " << sound_path << std::endl;
				sound().playSound(sound_path, 0, event_it->second.extra.at(0) + 4);
			}

			done_time = (std::max)(arg_time, done_time); //ensure we never play the same sound twice no matter what
		}

		void update(RAS::Manager* arg_manager, RAS::Time arg_time, vector<RAS::Actor>& arg_actors)
		{

		}
	};
}