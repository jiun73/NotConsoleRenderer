#pragma once
#include "EntityX.h"
#include "BasicComponents.h"
#include "CollisionComponent.h"

template<typename... Ts>
using Object = EntityX<Position_x, Shape_x, GFX_x, Angle_x, Collider_x, Ts...>;

enum GameTags
{
	ENTITY_PLAYER = 2,
	ENTITY_ENEMY
};
