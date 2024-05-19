#pragma once
#include "RAS_Typedef.h"
#include "RAS_Timeline.h"

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
