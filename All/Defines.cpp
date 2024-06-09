#include "pch.h"
#include "Defines.h"
#include "CustomNet.h"

void FIGHT::process_event_pack(CustomNet& net, RAS::Manager& manager, RAS::Time time, char code, bool from_p1, bool send)
{
	switch (code)
	{
	case '\x01':
		if(from_p1)
			manager.add_event(time, PLAYER1, MOVE_PLAYER_XN, POSY);
		else
			manager.add_event(time, PLAYER2, MOVE_PLAYER_XN, POSY);
		break;

	case '\x02':
		if (from_p1)
			manager.add_event(time, PLAYER1,MOVE_PLAYER_X, POSY);
		else
			manager.add_event(time, PLAYER2, MOVE_PLAYER_X, POSY);
		break;

	case '\x03':
		if (from_p1)
			manager.add_event(time, PLAYER1, STAY, POSY);
		else
			manager.add_event(time, PLAYER2, STAY, POSY);
		break;

	case '\x04':
		if (from_p1)
		{
			
		}
		else
		{
			manager.add_event(time, BALL_PLAYER2, POS_TO_PLAYER2Y, POSY);
			manager.add_event(time, BALL_PLAYER2, POS_TO_PLAYER2X, POSX);
			manager.add_event(time + 2, BALL_PLAYER2, MOVE_PONG_XN, POSX);
			manager.add_event(time + 2, BALL_PLAYER2, MOVE_PONG_Y, POSY);
			manager.add_event(time, BALL_PLAYER2, SET_ACTIVE, ACTIVE);
		}
		break;

	default:
		break;
	}

	if ((code & '\x0F') == '\x04' && from_p1)
	{
		char ballid = code >> 4;

		manager.add_event(time, BALL_PLAYER1 + ballid, POS_TO_PLAYER1Y, POSY);
		manager.add_event(time, BALL_PLAYER1 + ballid, POS_TO_PLAYER1X, POSX);
		manager.add_event(time + 2, BALL_PLAYER1 + ballid, MOVE_PONG_X, POSX);
		manager.add_event(time + 2, BALL_PLAYER1 + ballid, MOVE_PONG_Y, POSY);
		manager.add_event(time, BALL_PLAYER1 + ballid, SET_ACTIVE, ACTIVE);
	}

	if (send && net.connected())
	{
		net.send(time, code);
	}
}