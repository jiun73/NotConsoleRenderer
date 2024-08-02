#pragma once
#include "RAS.h"

namespace FF
{
	enum Actors
	{
		PLAYER1,
		PLAYER2
	};

	void add_actors(RAS::Manager& man)
	{
		man.register_actor(0b11, PLAYER1);
		man.register_actor(0b11, PLAYER2);
	}
}