#pragma once
#include <map>
#include <unordered_map>
#include <array>
#include <vector>
#include <string>
#include <new>
#include <typeindex>
#include <cassert>
#include <tuple>

struct Event;


typedef char HandlerID; //that would leave us with only 256 possible event types, but if we make them generic enough, it can work
typedef size_t TimeMs;
typedef size_t Bitmask64;
typedef size_t DatatypeID;
typedef char* RawData;
constexpr size_t MAX_DATA_TYPES() { return (sizeof(Bitmask64) * 8); }

using std::map;
using std::unordered_map;
using std::string;
using std::vector;
using std::launder;
using std::type_index;

struct Event;
struct EventHandler;

inline vector<size_t> get_list_from_bytes(Bitmask64 bytes)
{
	vector<size_t> ret;
	for (size_t i = 0; i < 32; i++)
	{
		if (bytes & (1ull << i))
		{
			ret.push_back(i);
		}
	}
	return ret;
}

struct EventParameter
{
	template<typename... Ts>
	RawData set(const Ts&... data)
	{
		if constexpr (sizeof...(Ts) > 0)
		{
			vector<type_index> types;
			size_t size = 0;
			get_types<Ts...>(types, size);
			RawData raw = new char[size];
			size_t index = 0;
			set_types<Ts...>(raw, index, data...);
			assert((internal_check(types)));
			return raw;
		}
		else
			return nullptr;


	}

protected:
	//void get_types(vector<type_index>& list, size_t& size) {}

	template<typename T>
	void get_types_single(vector<type_index>& list, size_t& size)
	{
		list.push_back(typeid(T));
		size += sizeof(T);
	}

	template<typename T, typename... Rs>
	void get_types(vector<type_index>& list, size_t& size)
	{
		get_types_single<T>(list, size);
		if constexpr (sizeof...(Rs) == 0) return;
		else {
			get_types<Rs...>(list, size);
		}
	}

	//void set_types(RawData data, size_t& index) {}

	template<typename T>
	void set_types_single(RawData data, size_t& index, const T& a)
	{

		*(T*)(data + index) = a;
		index += sizeof(T);
	}

	template<typename T, typename... Rs>
	void set_types(RawData data, size_t& index, const T& a, const Rs&... as)
	{

		set_types_single<T>(data, index, a);
		if constexpr (sizeof...(Rs) == 0) return;
		else
		{
			set_types<Rs...>(data, index, as...);
		}
	}

	virtual bool internal_check(vector<type_index> types) = 0;
};

template<typename... Ts>
struct EventParameterType : public EventParameter
{
	bool internal_check(vector<type_index> types) override
	{
		vector<type_index> self_types;
		size_t size = 0;
		if constexpr (sizeof...(Ts) > 0)
			EventParameter::get_types<Ts...>(self_types, size);
		return self_types == types;
	}
};

struct EventHandler
{
	virtual EventParameter* param() = 0;
	virtual void apply(TimeMs time, const Event& event, RawData data, RawData args) = 0;
};

/*
* Describes what to do with a certain actor given a certain time
* (e.x position, size, etc)
*/
struct Event
{
	size_t time = 0;
	HandlerID id = 0;
	RawData params = nullptr;
	EventHandler* behaviour = nullptr;

	TimeMs time_from(TimeMs global_time) const
	{
		return global_time - time;
	}

	void apply(TimeMs time, RawData data, size_t field_index)
	{
		behaviour->apply(time, *this, (data + field_index), params);
	}

	Event() {}
	~Event() {}
};

class EventSequence
{
	map<TimeMs, Event> subevents;

public:
	EventSequence() {}
	EventSequence(TimeMs time, const Event& e) { add_event(time, e); }
	~EventSequence() {}

	size_t field_index = 0;

	Event& event_at(TimeMs time)
	{
		auto it = subevents.lower_bound(time);

		if (it != subevents.begin())
		{
			return (--it)->second;
		}
	}

	void apply(TimeMs time, RawData data)
	{
		event_at(time).apply(time, data, field_index);
	}

	TimeMs start_time() const
	{
		return subevents.begin()->first;
	}

	void add_event(TimeMs time, const Event& e)
	{
		subevents.emplace(time, e).first->second.time = time;
	}
};

/*
* Stores all events of an actor
* by event type and time
*/
class Timeline
{
	map<size_t, map<TimeMs, EventSequence>> events;

public:
	EventSequence& event_at(TimeMs time, size_t field)
	{
		auto it = events.at(field).lower_bound(time);

		if (it != events.at(field).begin())
		{
			return (--it)->second;
		}
	}

	//Applies all the events at the given time
	void snapshot(TimeMs time, RawData data)
	{
		for (auto& event : events)
		{
			event_at(time, event.first).apply(time,data);
		}
	}

	void add_event(const EventSequence& event)
	{
		events[event.field_index].emplace(event.start_time(), event);
	}

	map<size_t, map<TimeMs, EventSequence>>& get_events() 
	{
		return events;
	}
};

/*
* Each actor has it's own timeline
* 'key' is a bitmask describing the data types this actor has (e.g pos, size, etc)
* 'data' is a pointer to this data, in the order it appears in the bitmask
*/
struct Actor
{
	Timeline timeline;
	Bitmask64 key;
	RawData data;

	void apply_snapshot(TimeMs time)
	{
		timeline.snapshot(time, data);
	}
};

struct DataTypeFactory
{
	virtual void construct(RawData data) = 0;
	virtual void destruct(RawData data) = 0;
	virtual void move(RawData source, RawData destination) = 0;
	virtual size_t size() const = 0;
	virtual const type_info& type() = 0;
};

template<typename T>
class DataType : public DataTypeFactory
{
	const type_info& type() override
	{
		return typeid(T);
	}

	void construct(RawData data) override
	{
		new (&data[0]) T();
	}

	void destruct(RawData data) override
	{
		T* location = launder(reinterpret_cast<T*>(data));

		location->~T();
	}

	void move(RawData source, RawData destination) override
	{
		new (&destination[0]) T(std::move(*reinterpret_cast<T*>(source)));
	}

	size_t size() const override { return sizeof(T); }
};


template<typename T, typename D, typename... Args>
class EventHandlerType : public EventHandler
{
	T system;
	EventParameterType<Args...> params;

	EventParameter* param() override
	{
		return &params;
	};

	template<size_t I, typename... Ts>
	void apply_unfold(TimeMs time, const Event& event, RawData data, RawData raw_args, size_t& index, const Ts&... args)
	{
		if constexpr (I < sizeof...(Args))
		{
			using type = std::tuple_element_t<I, tuple<Args...>>;
			size_t index_before = index;
			index += sizeof(type);
			apply_unfold<I + 1>(time, event, data, raw_args, index, args..., *(type*)(raw_args + index_before));

		}
		else
		{
			system.apply(time, event, *(D*)(data), args...);
		}
		//TODO pass real args;
	}


	void apply(TimeMs time, const Event& event, RawData data, RawData args) override
	{
		if constexpr (sizeof...(Args) == 0)
		{
			system.apply(time, event, *(D*)(data)); //TODO pass real args;
		}
		else
		{
			size_t index = 0;
			apply_unfold<0>(time, event, data, args, index);
		}

	}
};

/*
* The time managers handles all actors and all events
*
* I'm optimising for events rather than for actors (there won't be that many anyway compared to the event count)
*/
class TimeManager
{
	TimeMs start_time = 0;
	vector<EventHandler*> handlers;
	vector<Actor> actors;

	unordered_map<type_index, DataTypeFactory*> factories; //used to allocate registered types
	unordered_map<DatatypeID, type_index> field_types; //get the types of fields (index of factory)
	map<string, DatatypeID> field_names; //field names for debugging

	DatatypeID field_counter = 0;

public:
	vector<Actor>& get_actors() { return actors; }

	template<typename T>
	void register_type()
	{
		factories.emplace(typeid(T), new DataType<T>());
	}

	template<typename T>
	size_t register_field(const string& name)
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

	template<typename T, typename D, typename... Args>
	size_t register_handler()
	{
		handlers.push_back(new EventHandlerType<T, D, Args...>());
		return handlers.size() - 1;
	}

	//Makes a new Actor and returns it's index
	//Once you make an Actor, it can never be removed (at least during the manager's life cycle)
	size_t make_actor(Bitmask64 key)
	{
		Actor actor;
		actor.key = key;
		vector<DatatypeID> ids = get_list_from_bytes(key);
		size_t full_size = 0;
		for (auto i : ids)
		{
			full_size += factories.at(field_types.at(i))->size();
		}
		RawData data = new char[full_size];

		size_t index = 0;
		for (auto i : ids)
		{
			factories.at(field_types.at(i))->construct(data + index);
			index += factories.at(field_types.at(i))->size();
		}

		actor.data = data;
		actors.push_back(actor);
		return actors.size() - 1;
	}

	template<typename... Ts>
	Event make_event(HandlerID id, const Ts&... args)
	{
		Event e;
		e.id = id;
		e.behaviour = handlers.at(id);
		e.params = handlers.at(id)->param()->set(args...);
		return e;
	}

	void add_event_to_actor(size_t fieldid, size_t actorid, EventSequence e)
	{
		e.field_index = get_index_for_field(actors.at(actorid).key, fieldid);
		actors.at(actorid).timeline.add_event(e);
	}

	void snapshot_now(size_t time)
	{
		for (auto& a : actors)
		{
			a.apply_snapshot(time - start_time);
		}
	}

	size_t get_index_for_field(size_t key, size_t fieldid)
	{
		size_t cntr = 0;
		bool f = false;
		for (auto& i : get_list_from_bytes(key))
		{
			if (i == fieldid)
			{
				f = true;
				break;
			}
			cntr += factories.at(field_types.at(i))->size();
		}
		assert(f);
		return cntr;
	}

	template<typename T>
	T& get_actor_field(size_t actorid, size_t fieldid)
	{
		assert(factories.at(field_types.at(fieldid))->type() == typeid(T));

		return *(T*)(actors.at(actorid).data + get_index_for_field(actors.at(actorid).key, fieldid));
	}

	EventSequence& get_actor_data_sequence(TimeMs time, size_t actorid, size_t fieldid)
	{
		return actors.at(actorid).timeline.event_at(time, fieldid);
	}

	Event& get_actor_data_event(TimeMs time, size_t actorid, size_t fieldid)
	{
		return get_actor_data_sequence(time, actorid, fieldid).event_at(time);
	}

	//Set the time mesured to start from now
	void start(TimeMs time)
	{
		start_time = time;
	}

	TimeMs now(TimeMs time)
	{
		return time - start_time;
	}
};