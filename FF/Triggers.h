#pragma once
#include "RAS.h"
#include "Fields.h"
#include "Actors.h"
#include "Generators.h"

namespace FF 
{
	enum Users 
	{
		HOST,
		PEER
	};

	enum Triggers 
	{
		START_POS,
		PLYR1_MOVE_LEFT,
		PLYR1_MOVE_RIGHT,
		PLYR1_STOP
	};

	void add_triggers(RAS::Manager& manager)
	{
		manager.register_trigger({ { RAS::TriggerArg(PLAYER1, SET_STATIC, MVTX, { 100 }) } }, START_POS, HOST);
		manager.register_trigger({ { RAS::TriggerArg(PLAYER1, SET_ACCELLERATION, MVTX, { 280 }) } }, PLYR1_MOVE_LEFT, HOST);
		manager.register_trigger({ { RAS::TriggerArg(PLAYER1, SET_ACCELLERATION, MVTX, { -280 }) } }, PLYR1_MOVE_RIGHT, HOST);
		manager.register_trigger({ { RAS::TriggerArg(PLAYER1, STAY, MVTX, { -1 }) } }, PLYR1_STOP, HOST);
	}
}