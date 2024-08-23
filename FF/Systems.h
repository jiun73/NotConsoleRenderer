#pragma once
#include <algorithm>
#include "RAS.h"
#include "Fields.h"
#include "Rect.h"
#include "FMods.h"

namespace FF
{
	using std::vector;
	using std::string;

	/*Simple rectangle describing a collider*/
	struct Collider
	{
		Rect bounds;
		size_t tag;
	};

	/*Set of rectangles describing a frames' bounds*/
	struct ColliderSet 
	{
		size_t unique_id;
		vector<Collider> colliders;
		Rect bounding_box;
	};

	/*Manages all the colliders associated with animations and gives them IDs*/
	class ColliderManager
	{
	private:
		const string file_path = "";
		vector<ColliderSet> sets;

	public:
		void load_from_file() 
		{

		}

		ColliderSet& get_set(size_t id)
		{

		}
	};

	/*Describes a pair of sets to resolve*/
	struct SetPair
	{
		RAS::Time start_time;
		RAS::Time end_time;

		RAS::Modifier* mod1;
		RAS::Time mod_offset1;
		double axis_size1;
		ColliderSet* collider1;
		size_t group1;

		RAS::Modifier* mod2;
		RAS::Time mod_offset2;
		double axis_size2;
		ColliderSet* collider2;
		size_t group2;
	};

	struct CollisionInfo
	{
		bool collided = false;
		RAS::Time collision_time;
	};

	class CollisionDetector 
	{
	private:
		RAS::Manager* ras;
		ColliderManager collider_manager;

	public:
		const RAS::FieldAlias set_id_field = COLLIDER_SETID;

		void find_collision(const SetPair& pair)
		{
			auto p1 = mvt_mod_to_quad(pair.mod1);

			auto p2 = mvt_mod_to_quad(pair.mod1);

			find_overlap_range(p1_1, p1_2, p2_1, p2_2);
		}

		CollisionInfo find_pair_collision_info(const SetPair& pair) 
		{
			
		}

		vector<RAS::Time> get_all_unique_modifiers(RAS::Time start_time, RAS::Timeline& timeline) 
		{
			vector<RAS::Time> ret_val;
			for (auto it = timeline.begin_from_time(start_time); it != timeline.events.end(); it++) //get all of the events after start_time
			{
				std::pair<const RAS::Time, RAS::Event>& event_pair = *it;
				for (auto yt = event_pair.second.begin_from_time(start_time); yt != event_pair.second.modifiers.begin(); yt++)
				{
					RAS::Time real_time = yt->first + event_pair.first;
					RAS::Time cut_time = std::max(start_time, real_time);
					ret_val.push_back(cut_time);
				}
			}

			return ret_val;
		}

		vector<RAS::Time>  merge_times(vector<RAS::Time>& t1, vector<RAS::Time>& t2)
		{
			vector<RAS::Time> ret_val;
			while (!(t1.empty() && t2.empty()))
			{
				if (t1.front() == t2.front())
				{
					ret_val.push_back(t1.front());
					t1.erase(t1.begin());
					t2.erase(t2.begin());
				}
				else if (t1.front() < t2.front())
				{
					ret_val.push_back(t1.front());
					t1.erase(t1.begin());
				}
				else
				{
					ret_val.push_back(t2.front());
					t2.erase(t2.begin());
				}
			}
		}

		/*get a list of all the changes of states in relevent fields in the timeline*/
		vector<RAS::Time> merge_timeline_events(RAS::Time start_time, RAS::ActorAlias& actor, RAS::FieldAlias axis_field) 
		{			
			vector<RAS::Time> set_events = get_all_unique_modifiers(start_time, ras->get_field_timeline(actor, set_id_field));
			vector<RAS::Time> axis_events= get_all_unique_modifiers(start_time, ras->get_field_timeline(actor, axis_field));

			return merge_times(set_events, axis_events);
		}

		bool axis_intersect(RAS::Time start_time, RAS::ActorAlias actor1, RAS::ActorAlias actor2, RAS::FieldAlias axis_field) 
		{
			vector<RAS::Time> actor1_events = merge_timeline_events(start_time, actor1, axis_field);
			vector<RAS::Time> actor2_events = merge_timeline_events(start_time, actor2, axis_field);
			
			vector<RAS::Time> all_events = merge_times(actor1_events, actor2_events);

			for (auto it = all_events.begin(); it != all_events.end(); it++)
			{
				SetPair pair;

				RAS::Time start_time = *it;
				RAS::Time end_time;
				auto it_next = it + 1;
				if (it_next != all_events.end())
				{
					end_time = *it_next;
				}
				else
				{
					end_time = std::numeric_limits<size_t>::max();
				}

				pair.start_time = start_time;
				pair.end_time = end_time;

				pair.collider1 = &collider_manager.get_set(ras->actor_field_at<size_t>(start_time, actor1, COLLIDER_SETID));
				pair.collider1 = &collider_manager.get_set(ras->actor_field_at<size_t>(start_time, actor2, COLLIDER_SETID));

				pair.group1 = ras->actor_field_at<size_t>(start_time, actor1, COLLIDER_GROUP);
				pair.group2 = ras->actor_field_at<size_t>(start_time, actor2, COLLIDER_GROUP);

				const RAS::Event& ev1 = ras->get_event_at(start_time, actor1, axis_field);
				const RAS::Event& ev2 = ras->get_event_at(start_time, actor2, axis_field);

				auto p1 = ev1.modifier_at_absolute_pair(start_time);
				auto p2 = ev2.modifier_at_absolute_pair(start_time);

				pair.mod1 = p1.second;
				pair.mod2 = p2.second;

				pair.mod_offset1 = p1.first + ev1.start_time;
				pair.mod_offset2 = p2.first + ev2.start_time;


			}
		}

		bool intersect_2d(RAS::Time start_time) 
		{

		}
	};

	class CollisionSystem
	{
	private:
		RAS::Time valid_time = 0;


	public:
	};
}