#pragma once
#include "RAS_Typedef.h"
#include "RAS_EventSequence.h"

/*
* Stores all events of an actor
* by event type and time
*/
class Timeline
{
	map<size_t, map<TimeMs, EventSequence>> events;

public:
	EventSequence& event_at(TimeMs time, FieldByteIndex field)
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
			event_at(time, event.first).apply(time, data);
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