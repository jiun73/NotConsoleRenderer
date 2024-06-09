#pragma once
#include "RAS.h"
#include "Defines.h"
#include <cassert>
#include <iostream>
#include <algorithm>

using std::vector;
using std::map;

#undef min
#undef max

namespace FIGHT
{
	struct CollisionSystem
	{

		RAS::Time collision_time = 0;
		RAS::Time check_resume = 0;
		bool continue_detection = false;
		RAS::Time time;
		RAS::Manager* manager;
		vector<RAS::Actor>* actors;

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

			map<RAS::Time, RAS::Modifier*>::iterator event1_mod = event1.modifiers.lower_bound(event1.start_time - (std::min)(event1.start_time, start_time));
			map<RAS::Time, RAS::Modifier*>::iterator event2_mod = event2.modifiers.lower_bound(event2.start_time - (std::min)(event2.start_time, start_time));

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
			RAS::Timeline& ball_X_timeline = actors->at(ball_actor).timelines.at(manager->get_field(POSX));
			RAS::Timeline& player_X_timeline = actors->at(player_actor).timelines.at(manager->get_field(POSX));

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
			RAS::ActorID ball = 0;
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
				int ballY = manager->actorid_field_at<int>(collision_time, ball, POSY);
				int playerY = manager->actorid_field_at<int>(collision_time, player, POSY);

				if (ballY >= playerY && ballY <= playerY + 90)
				{
					manager->add_event(collision_time, ball, gen, field, true);
				}

				p1_coll_check = std::max(check_resume, collision_time + 1);

			}
		}

		//RAS::ActorID ball = 2;
		std::array<RAS::ActorID, 2> paddles = { PLAYER1 , PLAYER2 };
		std::array<RAS::ActorID, 6> balls = { BALL_PLAYER1, BALL_PLAYER1 + 1, BALL_PLAYER1 + 2, BALL_PLAYER1 + 3, BALL_PLAYER1 + 4 , BALL_PLAYER2 };
		std::array<CollisionInfo, 6 * 2> collisions;

		RAS::Time valid_time = 0;
		bool disable_snap = false;

		void on_snap(RAS::Manager* arg_manager, RAS::Time arg_time, vector<RAS::Actor>& arg_actors)
		{
			if (disable_snap) return;

			manager = arg_manager;
			actors = &arg_actors;
			//valid_time = 0;

			//std::cout << arg_time << std::endl;
			if (arg_time <= valid_time) return;

			std::cout << "new snap from " << valid_time << " to " << arg_time << std::endl;

			for (auto& c : collisions) c = CollisionInfo();



			while (true)
			{
				//if the snapshot is being taken from a time where the timeline was not invalidated, then we can return early
				//also, this way, we avoid edge cases where collision happen continously (like a bounce) and we avoid calculating too far into the future (which will likely be invalidated anyway)

				if (arg_time < valid_time) return;

				bool no_collision = true;
				size_t i = 0;
				for (auto& p : paddles)
				{
					for (auto& b : balls)
					{
						CollisionInfo& info = collisions.at(i);
						info = find_collision_info(info.found ? info.resume_check : valid_time, manager->get_actor(b), p);
						info.actor = p;
						info.ball = b;

						if (info.collision_time < valid_time) info.found = false;

						if (info.found) no_collision = false;
						i++;
					}
				}

				if (no_collision) //no collision found from there, so we break
				{
					valid_time = arg_time;
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

				int ballY = manager->actor_field_at<int>(early_collision->collision_time, early_collision->ball, POSY);
				int playerY = manager->actorid_field_at<int>(early_collision->collision_time, early_collision->actor, POSY);

				if (ballY >= playerY && ballY <= playerY + 90)
				{
					switch (early_collision->actor)
					{
					case 0:
						std::cout << "collision 1 found " << valid_time << std::endl;
						manager->add_event(early_collision->collision_time, early_collision->ball, MOVE_PONG_X, POSX, true);


						break;
					case 1:
						std::cout << "collision 2 found " << valid_time << std::endl;
						manager->add_event(early_collision->collision_time, early_collision->ball, MOVE_PONG_XN, POSX, true);

						break;
					default:
						break;
					}

					const RAS::Event& balle = actors->at(manager->get_actor(early_collision->ball)).timelines.at(manager->get_field(POSY)).event_at(early_collision->collision_time);
					RAS::Modifier* mod = nullptr;
					mod = balle.modifier_at(early_collision->collision_time - balle.start_time);

					if (mod->is_reversible() && mod->reverse_type() == LINEAR)
					{
						if (mod->reverse_params()[1] > 0)
						{
							manager->add_event(early_collision->collision_time, early_collision->ball, MOVE_PONG_Y, POSY, true);
						}
						else
						{
							manager->add_event(early_collision->collision_time, early_collision->ball, MOVE_PONG_YN, POSY, true);
						}
					}
					else
					{
						std::cout << "found a collision on a point???" << std::endl;
					}
				}
				else
				{
					std::cout << "collision oob found " << valid_time << std::endl;
					disable_snap = true;
					manager->add_event(early_collision->collision_time, early_collision->ball, BALL_POS_OOB, POSX, true);
					manager->add_event(early_collision->collision_time, early_collision->ball, BALL_POS_OOB, POSY, true);
					manager->add_event(early_collision->collision_time, early_collision->ball, SET_INACTIVE, ACTIVE, true);
					disable_snap = false;
				}
			}
		}

		void update(RAS::Manager* arg_manager, RAS::Time arg_time, vector<RAS::Actor>& arg_actors)
		{
			actors = &arg_actors;
			manager = arg_manager;
			time = arg_time;


			//valid_time = std::min(valid_time, arg_time); //take the earliest time where the timeline is still valid and rebuild it from there on the next snapshot (this saves a lot of calculation per event added)
			valid_time = arg_time;
			std::cout << "new event: " << valid_time << std::endl;
		}
	};

}