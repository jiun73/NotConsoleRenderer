#pragma once
#include "EntityX.h"
#include "BasicComponents.h"
#include "CollisionComponent.h"

template<typename... Ts>
using Object = EntityX<Position_x, Shape_x, GFX_x, Angle_x, Collider_x, Ts...>;

extern EntityX< Position_x, Angle_x, GFX_x, Controller_x, Shape_x, Shooter_x, ParticleEmitter_x, Collider_x, Health_x> player;