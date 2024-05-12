#pragma once
#include <map>
#include <array>
#include <vector>

struct Event;


typedef char EventID; //that would leave us with only 256 possible event types, but if we make them generic enough, it can work
typedef size_t TimeMs;
typedef size_t Bitmask64;
typedef char* RawData;
typedef void(*EventFunc)(const Event&, RawData, TimeMs);

using std::map;
using std::vector;

/*
* Describes what to do with a certain actor given a certain time
* (e.x position, size, etc)
*/
struct Event
{
	EventID id; 
	bool end = false;
	RawData params = nullptr;
	EventFunc fn = nullptr;

	void apply(TimeMs time, RawData data)
	{
		fn(*this, data, time);
	}
};

/*
* Stores all events of an actor
* by event type and time
*/
class Timeline 
{
	map<TimeMs, Event> events;

public:
	//Applies all the events at the given time
	void snapshot(const TimeMs& time)
	{
		events.lower_bound(time)->second.apply(time);
	}

	void add_event(const TimeMs& time, const Event& event)
	{
		events.emplace(time, event);
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
};

/*
* The time managers handles all actors and all events
* 
* I'm optimising for events rather than for actors (there won't be that many anyway compared to the event count)
*/
class TimeManager 
{
	vector<EventFunc> functions;
	vector<Actor> actors;

public:
	Event make_event(EventID id)
	{

	}

	void snapshot_now()
	{

	}
};