#pragma once
#include "RAS_Typedef.h"

/*
* Describes what to do with a certain actor given a certain time
* (e.x position, size, etc)
*/
struct Modifier
{
public:
	TimeMs time = 0;
	HandlerID id = 0;
	ActorID actorid = 0;

	FieldByteIndex fieldbyteid = 0;
	RawData params = nullptr;
	ModifierHandler* behaviour = nullptr;
	TimeManager* manager = nullptr;

	TimeMs time_from(TimeMs global_time) const
	{
		return global_time - time;
	}

	void apply(TimeMs time, RawData data, FieldByteIndex field_index);

	Modifier() {}
	~Modifier() {}
};
