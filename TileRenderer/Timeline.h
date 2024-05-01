#pragma once
#include <map>
#include <array>
#include <vector>

struct Event;

typedef void(*fn_type)(const Event&, char*, size_t);

/*
* Describes what to do with a certain actor given a certain time
* (e.x position, size, etc)
*/
struct Event
{
	char event_code; //that would leave us with only 256 possible event types, but if we make them generic enough, it can work
	bool end = false;
	char* params = nullptr;
	fn_type fn = nullptr;
};

/*
* Stores all events of an actor
* by event type and time
*/
class Timeline 
{
	std::array<std::map<size_t, Event>, 256> events;

public:

	//Applies all the events at the given time
	void snapshot(size_t time) 
	{

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
	size_t key;
	char* data;
};

/*
* The time managers handles all actors and all events
* 
* I'm optimising for events rather than for actors (there won't be that many anyway compared to the event count)
*/
class TimeManager 
{
	std::vector<fn_type> functions;
	std::vector<Actor> actors;

public:
	void snapshot_now()
	{

	}
};