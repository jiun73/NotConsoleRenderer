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

	case '\x1F':
		if (from_p1)
			manager.add_event(time, PLAYER1, SET_TYPE_GLOCK, TYPE);
		else
			manager.add_event(time, PLAYER2, SET_TYPE_GLOCK, TYPE);
		break;
	case '\x2F':

		if (from_p1)
			manager.add_event(time, PLAYER1, SET_TYPE_AK, TYPE);
		else
			manager.add_event(time, PLAYER2, SET_TYPE_AK, TYPE);
		break;

	case '\x3F':

		if (from_p1)
			manager.add_event(time, PLAYER1, SET_TYPE_SHIELD, TYPE);
		else
			manager.add_event(time, PLAYER2, SET_TYPE_SHIELD, TYPE);
		break;

	case '\x4F':
		if (from_p1)
			manager.add_event(time, PLAYER1, SET_TYPE_RECALL, TYPE);
		else
			manager.add_event(time, PLAYER2, SET_TYPE_RECALL, TYPE);
		break;

	case '\x4E':
		if (from_p1)
			for (int i = 0; i < MAX_BALLS; i++)
			{
				manager.add_event(time, BALL_PLAYER1 + i, BALL_POS_OOB, POSX);
				manager.add_event(time, BALL_PLAYER1 + i, BALL_POS_OOB, POSY);
				manager.add_event(time, BALL_PLAYER1 + i, SET_INACTIVE, ACTIVE);
			}
		else
			for (int i = 0; i < MAX_BALLS; i++)
			{
				manager.add_event(time, BALL_PLAYER2 + i, BALL_POS_OOB, POSX);
				manager.add_event(time, BALL_PLAYER2 + i, BALL_POS_OOB, POSY);
				manager.add_event(time, BALL_PLAYER2 + i, SET_INACTIVE, ACTIVE);
			}
		break;
	default:
		break;
	}

	if ((code & '\x0F') == '\x04')
	{
		unsigned char ballid = (code >> 4) & '\x0F';
		if (from_p1)
		{
			manager.add_event(time, BALL_PLAYER1 + ballid, POS_TO_PLAYER1Y, POSY);
			manager.add_event(time, BALL_PLAYER1 + ballid, POS_TO_PLAYER1X, POSX);
			manager.add_event(time + 2, BALL_PLAYER1 + ballid, MOVE_PONG_X, POSX);
			manager.add_event(time + 2, BALL_PLAYER1 + ballid, MOVE_PONG_Y, POSY);
			manager.add_event(time, BALL_PLAYER1 + ballid, SET_TYPE_GLOCK, TYPE);
			manager.add_event(time, BALL_PLAYER1 + ballid, SET_ACTIVE, ACTIVE);
			manager.add_event_extra(time, SOUND_MASTER, PLAY_SOUND, SOUND, { GUN1 });
			manager.add_event_extra(time, SCREENSHAKER, SET_STRENGTH, STRENGTH, { 10 });
		}
		else
		{
			manager.add_event(time, BALL_PLAYER2 + ballid, POS_TO_PLAYER2Y, POSY);
			manager.add_event(time, BALL_PLAYER2 + ballid, POS_TO_PLAYER2X, POSX);
			manager.add_event(time + 2, BALL_PLAYER2 + ballid, MOVE_PONG_XN, POSX);
			manager.add_event(time + 2, BALL_PLAYER2 + ballid, MOVE_PONG_Y, POSY);
			manager.add_event(time, BALL_PLAYER2 + ballid, SET_TYPE_GLOCK, TYPE);
			manager.add_event(time, BALL_PLAYER2 + ballid, SET_ACTIVE, ACTIVE);
			manager.add_event_extra(time, SOUND_MASTER, PLAY_SOUND, SOUND, { GUN1 });
			manager.add_event_extra(time, SCREENSHAKER, SET_STRENGTH, STRENGTH, { 10 });
		}
	}

	if ((code & '\x0F') == '\x06')
	{
		unsigned char ballid = (code >> 4) & '\x0F';
		if (from_p1)
		{
			manager.add_event(time, BALL_PLAYER1 + ballid, POS_TO_PLAYER1Y, POSY);
			manager.add_event(time, BALL_PLAYER1 + ballid, POS_TO_PLAYER1X, POSX);
			manager.add_event(time + 2, BALL_PLAYER1 + ballid, MOVE_PONG_X, POSX);
			manager.add_event(time + 2, BALL_PLAYER1 + ballid, MOVE_PONG_YN, POSY);
			manager.add_event(time, BALL_PLAYER1 + ballid, SET_TYPE_GLOCK, TYPE);
			manager.add_event(time, BALL_PLAYER1 + ballid, SET_ACTIVE, ACTIVE);
			manager.add_event_extra(time, SOUND_MASTER, PLAY_SOUND, SOUND, { GUN1 });
			manager.add_event_extra(time, SCREENSHAKER, SET_STRENGTH, STRENGTH, { 10 });
		}
		else
		{
			manager.add_event(time, BALL_PLAYER2 + ballid, POS_TO_PLAYER2Y, POSY);
			manager.add_event(time, BALL_PLAYER2 + ballid, POS_TO_PLAYER2X, POSX);
			manager.add_event(time + 2, BALL_PLAYER2 + ballid, MOVE_PONG_XN, POSX);
			manager.add_event(time + 2, BALL_PLAYER2 + ballid, MOVE_PONG_YN, POSY);
			manager.add_event(time, BALL_PLAYER2 + ballid, SET_TYPE_GLOCK, TYPE);
			manager.add_event(time, BALL_PLAYER2 + ballid, SET_ACTIVE, ACTIVE);
			manager.add_event_extra(time, SOUND_MASTER, PLAY_SOUND, SOUND, { GUN1 });
			manager.add_event_extra(time, SCREENSHAKER, SET_STRENGTH, STRENGTH, {10});
		}
	}

	if ((code & '\x0F') == '\x07')
	{
		unsigned char ballid = (code >> 4) & '\x0F';
		if (from_p1)
		{
			manager.add_event(time, BALL_PLAYER1 + ballid, POS_TO_PLAYER1Y, POSY);
			manager.add_event(time, BALL_PLAYER1 + ballid, POS_TO_PLAYER1X, POSX);
			manager.add_event(time + 2, BALL_PLAYER1 + ballid, MOVE_PONG_X, POSX);
			manager.add_event(time + 2, BALL_PLAYER1 + ballid, STAY, POSY);
			manager.add_event(time, BALL_PLAYER1 + ballid, SET_TYPE_GLOCK, TYPE);
			manager.add_event(time, BALL_PLAYER1 + ballid, SET_ACTIVE, ACTIVE);
			manager.add_event_extra(time, SOUND_MASTER, PLAY_SOUND, SOUND, { GUN1 });
			manager.add_event_extra(time, SCREENSHAKER, SET_STRENGTH, STRENGTH, { 10 });
		}
		else
		{
			manager.add_event(time, BALL_PLAYER2 + ballid, POS_TO_PLAYER2Y, POSY);
			manager.add_event(time, BALL_PLAYER2 + ballid, POS_TO_PLAYER2X, POSX);
			manager.add_event(time + 2, BALL_PLAYER2 + ballid, MOVE_PONG_XN, POSX);
			manager.add_event(time + 2, BALL_PLAYER2 + ballid, STAY, POSY);
			manager.add_event(time, BALL_PLAYER2 + ballid, SET_TYPE_GLOCK, TYPE);
			manager.add_event(time, BALL_PLAYER2 + ballid, SET_ACTIVE, ACTIVE);
			manager.add_event_extra(time, SOUND_MASTER, PLAY_SOUND, SOUND, { GUN1 });
			manager.add_event_extra(time, SCREENSHAKER, SET_STRENGTH, STRENGTH, {10});
		}
	}

	if ((code & '\x0F') == '\x05')
	{
		unsigned char ballid = (code >> 4) & '\x0F';
		if (from_p1)
		{
			manager.add_event(time, BALL_PLAYER1 + ballid, POS_TO_PLAYER1Y, POSY);
			manager.add_event(time, BALL_PLAYER1 + ballid, POS_TO_PLAYER1X, POSX);
			manager.add_event(time + 2, BALL_PLAYER1 + ballid, MOVE_BULLET_X, POSX);
			manager.add_event(time + 2, BALL_PLAYER1 + ballid, MOVE_BULLET_Y, POSY);
			manager.add_event(time, BALL_PLAYER1 + ballid, SET_TYPE_AK, TYPE);
			manager.add_event(time, BALL_PLAYER1 + ballid, SET_ACTIVE, ACTIVE);
			manager.add_event_extra(time, SOUND_MASTER, PLAY_SOUND, SOUND, { GUN2 });
			manager.add_event_extra(time, SCREENSHAKER, SET_STRENGTH, STRENGTH, {10});
		}
		else
		{
			manager.add_event(time, BALL_PLAYER2 + ballid, POS_TO_PLAYER2Y, POSY);
			manager.add_event(time, BALL_PLAYER2 + ballid, POS_TO_PLAYER2X, POSX);
			manager.add_event(time + 2, BALL_PLAYER2 + ballid, MOVE_BULLET_XN, POSX);
			manager.add_event(time + 2, BALL_PLAYER2 + ballid, MOVE_BULLET_Y, POSY);
			manager.add_event(time, BALL_PLAYER2 + ballid, SET_TYPE_AK, TYPE);
			manager.add_event(time, BALL_PLAYER2 + ballid, SET_ACTIVE, ACTIVE);
			manager.add_event_extra(time, SOUND_MASTER, PLAY_SOUND, SOUND, {GUN2});
			manager.add_event_extra(time, SCREENSHAKER, SET_STRENGTH, STRENGTH, { 10 });
		}
	}

	if (send && net.connected())
	{
		net.send(time, code);
	}
}