#pragma once
#include "RAS_Typedef.h"
#include "RAS_Systems.h"
#include "RAS_DataFactory.h"
#include "RAS_EventSequence.h"

/*
* The time managers handles all actors and all events
*
* I'm optimising for events rather than for actors (there won't be that many anyway compared to the event count)
*/
class TimeManager
{
	TimeMs start_time = 0;
	vector<ModifierHandler*> handlers;
	vector<Actor> actors;

	unordered_map<type_index, DataTypeFactory*> factories; //used to allocate registered types
	unordered_map<DatatypeID, type_index> field_types; //get the types of fields (index of factory)
	map<string, DatatypeID> field_names; //field names for debugging

	vector<SystemFactory*> systems;

	DatatypeID field_counter = 0;

public:
	vector<Actor>& get_actors() { return actors; }

	void	start(TimeMs time);
	TimeMs	now(TimeMs time);
	void	snapshot_now(TimeMs time);
	void	update_systems();

	template<typename T> size_t		register_system(Bitmask64 key);
	template<typename T> void		register_type();
	template<typename T> FieldID	register_field(const string& name);
	template<typename T, typename D, typename... Args> HandlerID register_handler();

	pair<RawData, size_t> allocate_actor_data(Bitmask64 key);
	RawData make_actor_data(Bitmask64 key);

	//Makes a new Actor and returns it's index
	//Once you make an Actor, it can never be removed (at least during the manager's life cycle)
	ActorID							make_actor(Bitmask64 key);
	template<typename... Ts> Modifier	make_event(HandlerID id, const Ts&... args);

	void add_event_to_actor(FieldID fieldid, ActorID actorid, EventSequence e);

	FieldByteIndex			get_index_for_field		(Bitmask64 key, FieldID fieldid);
	template<typename T> T& get_actor_field			(ActorID actorid, FieldID fieldid);
	template<typename T> T& get_actor_field_at		(TimeMs time, ActorID actorid, FieldByteIndex fieldbyteid);
	EventSequence&			get_actor_data_sequence	(TimeMs time, ActorID actorid, FieldID fieldid);
	Modifier&					get_actor_data_event	(TimeMs time, ActorID actorid, FieldID fieldid);

	
};

template<typename T>
inline size_t TimeManager::register_system(Bitmask64 key)
{
	systems.push_back(new SystemType<T>(key));
}

template<typename T>
inline void TimeManager::register_type()
{
	factories.emplace(typeid(T), new DataType<T>());
}

template<typename T>
FieldID TimeManager::register_field(const string& name)
{
	register_type<T>();
	field_types.emplace(field_counter, typeid(T));
	field_names.emplace(name, field_counter);
	field_counter++;

	if (field_counter >= MAX_DATA_TYPES())
	{
		assert(false);
	}

	return field_counter - 1;
}

template<typename T, typename D, typename ...Args>
inline HandlerID TimeManager::register_handler()
{
	handlers.push_back(new EventHandlerType<T, D, Args...>());
	return handlers.size() - 1;
}

template<typename ...Ts>
inline Modifier TimeManager::make_event(HandlerID id, const Ts & ...args)
{
	Modifier e;
	e.id = id;
	e.manager = this;
	e.behaviour = handlers.at(id);
	e.params = handlers.at(id)->param()->set(args...);
	return e;
}

template<typename T>
inline T& TimeManager::get_actor_field(ActorID actorid, FieldID fieldid)
{
	assert(factories.at(field_types.at(fieldid))->type() == typeid(T));

	return *(T*)(actors.at(actorid).data + get_index_for_field(actors.at(actorid).key, fieldid));
}

template<typename T>
inline T& TimeManager::get_actor_field_at(TimeMs time, ActorID actorid, FieldByteIndex fieldbyteid)
{
	assert(factories.at(field_types.at(fieldbyteid))->type() == typeid(T));
	RawData data = make_actor_data(actors.at(actorid).key); //allocate a temporary dummy of the actor data

	actors.at(actorid).timeline.event_at(time, fieldbyteid).apply(time, data); //get a snapshot of this event
	return *(T*)(data + fieldbyteid);
}
