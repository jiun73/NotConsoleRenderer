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
		PLYR1_MOVE_UP,
		PLYR1_MOVE_DOWN,
		PLYR1_MOVE_LEFT,
		PLYR1_MOVE_RIGHT,
		PLYR1_STOPX,
		PLYR1_STOPY,
	};

	void add_triggers(RAS::Manager& manager)
	{
		manager.register_trigger({ { RAS::TriggerArg(PLAYER1, SET_STATIC, MVTX, { 100 }) } }, START_POS, HOST);
		manager.register_trigger({ { RAS::TriggerArg(PLAYER1, SET_ACCELLERATION, MVTX, { -2800 }) } }, PLYR1_MOVE_LEFT, HOST);
		manager.register_trigger({ { RAS::TriggerArg(PLAYER1, SET_ACCELLERATION, MVTX, { 2800 }) } }, PLYR1_MOVE_RIGHT, HOST);
		manager.register_trigger({ { RAS::TriggerArg(PLAYER1, SET_ACCELLERATION, MVTY, { -2800 }) } }, PLYR1_MOVE_UP, HOST);
		manager.register_trigger({ { RAS::TriggerArg(PLAYER1, SET_ACCELLERATION, MVTY, { 2800 }) } }, PLYR1_MOVE_DOWN, HOST);
		manager.register_trigger({ { RAS::TriggerArg(PLAYER1, DECCELLARATION, MVTX, { 10000 }) } }, PLYR1_STOPX, HOST);
		manager.register_trigger({ { RAS::TriggerArg(PLAYER1, DECCELLARATION, MVTY, { 10000 }) } }, PLYR1_STOPY, HOST);
	}
}