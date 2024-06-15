#pragma once
#include "RAS.h"

namespace FIGHT
{
	

	using std::array;

	inline size_t operator ""ms(size_t ms) { return ms; }
	inline size_t operator ""s(size_t s) { return s * 1000; }
	inline size_t operator ""s(long double s) { return s * 1000; }
	
	inline const size_t MAX_BALLS = 16; //technical max considering the code is 8 bits and only 4 is used for the ball id
	inline const int PADDLE_SIZE_Y = 302;
	inline const int PADDLE_SIZE_X = 52;

	enum SoundEnum 
	{
		RIZZ,
		GUN1,
		GUN2,
		HURT
	};

	enum ActorEnum
	{
		PLAYER1,
		PLAYER2,
		BALL_PLAYER1,
		BALL_PLAYER2 = MAX_BALLS + BALL_PLAYER1,
		SOUND_MASTER = MAX_BALLS + BALL_PLAYER2,
		SCREENSHAKER,
	};

	enum FieldEnum
	{
		POSX,
		POSY,
		ACTIVE,
		TYPE,
		HEALTH,
		SOUND,
		STRENGTH
	};

	enum GeneratorEnum
	{
		PLAY_SOUND,
		STAY,
		PLAYER_START_HEALTH,
		HEALTH_SUB_PONG,
		HEALTH_SUB_BULLET,
		PLAYER1_START_POSY,
		PLAYER2_START_POSY,
		PLAYER1_START_POSX,
		PLAYER2_START_POSX,
		POS_TO_PLAYER1X,
		POS_TO_PLAYER1Y,
		POS_TO_PLAYER2X,
		POS_TO_PLAYER2Y,
		BALL_START_POS,
		BALL_POS_OOB,
		MOVE_PONG_X,
		MOVE_PONG_XN,
		MOVE_PONG_Y,
		MOVE_PONG_YN,
		MOVE_BULLET_X,
		MOVE_BULLET_XN,
		MOVE_BULLET_Y,
		MOVE_PLAYER_X,
		MOVE_PLAYER_XN,
		MOVE_PLAYER_Y,
		MOVE_PLAYER_YN,
		BALL2_START_POS,
		SET_ACTIVE,
		SET_INACTIVE,
		SET_TYPE_GLOCK,
		SET_TYPE_AK,
		SET_TYPE_SHIELD,
		SET_TYPE_RECALL,
		SET_STRENGTH,
	};

	enum GunsEnum
	{
		GLOCK,
		AK,
		SHIELD,
		RECALL
	};

	enum ModifierEnum
	{
		POINT,
		LINEAR
	};


	template<typename T>
	inline void point_func(RAS::Time time, T& i, array<double, 1> arr)
	{
		i = arr.at(0);
	}

	template<typename T>
	inline void const_func(RAS::Time time, T& x, const T& value)
	{
		x = value;
	}

	inline void linear_func(RAS::Time time, int& i, array<double, 2> arr)
	{
		double start = arr.at(0);
		double speed = arr.at(1);
		i = ((time / 1000.0) * speed) + start;
	}

	inline RAS::Time find_linear(int y, array<double, 2> arr)
	{
		double start = arr.at(0);
		double speed = arr.at(1);
		return ((y - start) / speed) * 1000.0;
	}

	class CustomNet;
	void process_event_pack(CustomNet& net, RAS::Manager& manager, RAS::Time time, char code, bool from_p1, bool send = false);
}