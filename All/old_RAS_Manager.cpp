#include "pch.h"
//#include "RAS_Manager.h"
//
//void TimeManager::update_systems()
//{
//	vector<vector<Actor*>> list;
//
//	for (auto& s : systems)
//	{
//		vector<Actor*> act_list;
//		for (auto& a : actors)
//		{
//			Bitmask64 key = s->get_key();
//			if ((key & a.key) == key)
//			{
//				act_list.push_back(&a);
//			}
//		}
//		s->update(act_list);
//	}
//}
//
//inline pair<RawData, size_t> TimeManager::allocate_actor_data(Bitmask64 key)
//{
//	vector<DatatypeID> ids = get_list_from_bytes(key);
//	size_t full_size = 0;
//	for (auto i : ids)
//	{
//		full_size += factories.at(field_types.at(i))->size();
//	}
//	RawData data = new char[full_size];
//	return { data,full_size };
//}
//
//inline RawData TimeManager::make_actor_data(Bitmask64 key)
//{
//	vector<DatatypeID> ids = get_list_from_bytes(key);
//	RawData data = allocate_actor_data(key).first;
//
//	size_t index = 0;
//	for (auto i : ids)
//	{
//		factories.at(field_types.at(i))->construct(data + index);
//		index += factories.at(field_types.at(i))->size();
//	}
//
//	return data;
//}
//
////Makes a new Actor and returns it's index
////Once you make an Actor, it can never be removed (at least during the manager's life cycle)
//
//inline ActorID TimeManager::make_actor(Bitmask64 key)
//{
//	Actor actor;
//	actor.key = key;
//	actor.data = make_actor_data(key);
//	actors.push_back(actor);
//	return actors.size() - 1;
//}
//
//inline void TimeManager::add_event_to_actor(FieldID fieldid, ActorID actorid, EventSequence e)
//{
//	e.field_index = get_index_for_field(actors.at(actorid).key, fieldid);
//	e.set_actor(actorid);
//	actors.at(actorid).timeline.add_event(e);
//}
//
//inline void TimeManager::snapshot_now(TimeMs time)
//{
//	for (auto& a : actors)
//	{
//		a.apply_snapshot(time - start_time);
//	}
//}
//
//inline FieldByteIndex TimeManager::get_index_for_field(Bitmask64 key, FieldID fieldid)
//{
//	size_t cntr = 0;
//	bool f = false;
//	for (auto& i : get_list_from_bytes(key))
//	{
//		if (i == fieldid)
//		{
//			f = true;
//			break;
//		}
//		cntr += factories.at(field_types.at(i))->size();
//	}
//	assert(f);
//	return cntr;
//}
//
//inline EventSequence& TimeManager::get_actor_data_sequence(TimeMs time, ActorID actorid, FieldID fieldid)
//{
//	return actors.at(actorid).timeline.event_at(time, fieldid);
//}
//
//inline Modifier& TimeManager::get_actor_data_event(TimeMs time, ActorID actorid, FieldID fieldid)
//{
//	return get_actor_data_sequence(time, actorid, fieldid).event_at(time);
//}
//
////Set the time mesured to start from now
//
//inline void TimeManager::start(TimeMs time)
//{
//	start_time = time;
//}
//
//inline TimeMs TimeManager::now(TimeMs time)
//{
//	return time - start_time;
//}
