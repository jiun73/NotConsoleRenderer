#pragma once
#include "RAS.h"

namespace FF {
	enum Fields
	{
		POSX,
		POSY,
		VELX,
		VELY,
		ACCX,
		ACCY,
	};

	void add_field(RAS::Manager& manager)
	{
		manager.register_field<double>(POSX);
		manager.register_field<double>(POSY);
	}
}
