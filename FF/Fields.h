#pragma once
#include "RAS.h"

namespace FF {
	struct Mvt
	{
		double pos = 0;
		double vel = 0;
		double acc = 0;
	};

	enum Fields
	{
		MVTX,
		MVTY,
		COLLIDER_SETID,
		COLLIDER_GROUP,
		FRAME_ID,
	};

	void add_field(RAS::Manager& manager)
	{
		manager.register_field<Mvt>(MVTX);
		manager.register_field<Mvt>(MVTY);
	}
}
