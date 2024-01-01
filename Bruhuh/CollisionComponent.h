#pragma once
#include <type_traits>

enum CollisionTypes
{
	CTYPE_CUSTOM,
	CTYPE_PUSH,
	CTYPE_DESTROY,
	CTYPE_HURT
};

enum ColliderTags
{
	TAG_PLAYER_ = 0b00001,
	TAG_TESTOBJ = 0b00010,
	TAG_PBULLET = 0b00100,
	TAG_ENEMY__ = 0b01000,
	TAG_EBULLET = 0b10000
};

typedef uint32_t ColliderTag;
typedef uint32_t ColliderKey;
typedef uint64_t ColliderPair;

struct Collider_x
{
	ColliderTag tag;
};
