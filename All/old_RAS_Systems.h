#pragma once
#include "RAS_Typedef.h"
#include "RAS_Actors.h"

struct SystemFactory
{
	virtual Bitmask64 get_key() = 0;
	virtual void update(vector<Actor*>& vec) = 0;
};

template<typename T>
struct SystemType : SystemFactory
{
	SystemType(Bitmask64 key) : key(key) {}
	~SystemType() {}

	T system;
	Bitmask64 key;

	Bitmask64 get_key() override
	{
		return key;
	}

	void update(vector<Actor*>& vec) override
	{

	}
};
