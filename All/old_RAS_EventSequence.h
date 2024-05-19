#pragma once
#include "RAS_Event.h"
#include "RAS_Typedef.h"

class EventSequence
{
	map<TimeMs, Modifier> subevents;

public:
	EventSequence() {}
	EventSequence(TimeMs time, const Modifier& e) { add_event(time, e); }
	~EventSequence() {}

	size_t field_index = 0;

	Modifier& event_at(TimeMs time)
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

	void add_event(TimeMs time, const Modifier& e)
	{
		subevents.emplace(time, e).first->second.time = time;
	}

	void set_actor(size_t actorid)
	{
		for (auto& e : subevents)
		{
			e.second.actorid = actorid;
		}
	}
};